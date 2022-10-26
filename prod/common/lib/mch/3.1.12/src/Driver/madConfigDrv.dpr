// ***************************************************************
//  madConfigDrv.dpr          version: 1.0.2  ·  date: 2016-03-09
//  -------------------------------------------------------------
//  stores configuration information into a driver file
//  -------------------------------------------------------------
//  Copyright (C) 1999 - 2016 www.madshi.net, All Rights Reserved
// ***************************************************************

// 2016-03-09 1.0.2 the checksum is now always set correctly
// 2011-03-27 1.0.1 (1) "-safeStopAllowed" wasn't stored correctly
//                  (2) "-fixChecksum" added
// 2010-01-10 1.0.0 initial release

program madConfigDrv;

uses Windows, SysUtils, madTools, madStrings, madRipeMD;

{$apptype console}
{$R madConfigVersion.res}

type
  THookDll = packed record
    Size : dword;
    Hash : TDigest;
  end;

  TDriverConfig = packed record
    Magic    : array [0..3] of dword;
    Flags    : dword;
    Reserved : dword;
    Name     : array [0..39] of char;
    DllCount : dword;
    DllData  : array [0..39] of THookDll;
  end;

function AddDll(dll: string; var drvCfg: TDriverConfig) : boolean;
var fh, map : dword;
    buf     : pointer;
    i1      : integer;
begin
  result := false;
  KillChar(dll, '"');
  if drvCfg.DllCount < 40 then begin
    fh := CreateFile(pchar(dll), GENERIC_READ, FILE_SHARE_READ, nil, OPEN_EXISTING, 0, 0);
    if fh <> dword(-1) then begin
      map := CreateFileMapping(fh, nil, PAGE_READONLY, 0, 0, nil);
      if map <> 0 then begin
        buf := MapViewOfFile(map, FILE_MAP_READ, 0, 0, 0);
        if buf <> nil then begin
          with drvCfg.DllData[drvCfg.DllCount] do begin
            Size := GetFileSize(fh, nil);
            Hash := madRipeMD.Hash(buf, size);
          end;
          UnmapViewOfFile(buf);
          result := true;
          inc(drvCfg.DllCount);
          for i1 := 0 to drvCfg.DllCount - 2 do
            if CompareMem(@drvCfg.DllData[i1], @drvCfg.DllData[drvCfg.DllCount - 1], sizeOf(THookDll)) then begin
              WriteLn('The hook dll "' + dll + '" was already in the list.');
              dec(drvCfg.DllCount);
              break;
            end;
        end else
          WriteLn('Mapping the file view for hook dll "' + dll + '" failed.');
        CloseHandle(map);
      end else
        WriteLn('Creating a file mapping for hook dll "' + dll + '" failed.');
      CloseHandle(fh);
    end else
      WriteLn('Opening the hook dll "' + dll + '" failed.');
  end else
    WriteLn('One driver can only hold 40 hook dlls.');
end;

function ReadConfig(driver: string; out drvCfg: TDriverConfig) : boolean;
var fh, map  : dword;
    buf      : pointer;
    c1       : dword;
begin
  result := false;
  ZeroMemory(@drvCfg, sizeOf(drvCfg));
  drvCfg.Magic[0] := $77777777;
  drvCfg.Magic[1] := $88888888;
  drvCfg.Magic[2] := $99999999;
  drvCfg.Magic[3] := $77777777;
  fh := CreateFile(pchar(driver), GENERIC_READ, FILE_SHARE_READ, nil, OPEN_EXISTING, 0, 0);
  if fh <> dword(-1) then begin
    map := CreateFileMapping(fh, nil, PAGE_READONLY, 0, 0, nil);
    if map <> 0 then begin
      buf := MapViewOfFile(map, FILE_MAP_READ, 0, 0, 0);
      if buf <> nil then begin
        c1 := PosPChar(PAnsiChar(@drvCfg), buf, sizeOf(drvCfg.Magic), GetFileSize(fh, nil));
        if c1 > 0 then begin
          Move(pointer(dword(buf) + c1)^, drvCfg, sizeOf(drvCfg));
          result := true;
        end else
          WriteLn('This doesn''t seem to be a madCodeHook injection driver.');
        UnmapViewOfFile(buf);
      end else
        WriteLn('Mapping the file view for driver "' + driver + '" failed.');
      CloseHandle(map);
    end else
      WriteLn('Creating a file mapping for driver "' + driver + '" failed.');
    CloseHandle(fh);
  end else
    WriteLn('Opening the driver "' + driver + '" failed.');
end;

procedure CalcCheckSum(baseAddress: pointer; size: dword);
var nh : PImageNtHeaders;
    i1 : dword;
    c1 : dword;
begin
  nh := GetImageNtHeaders(dword(baseAddress));
  nh^.OptionalHeader.CheckSum := 0;
  c1 := 0;
  for i1 := 0 to size div 2 - 1 do begin
  	c1 := c1 + word(baseAddress^);
    if c1 and $ffff0000 <> 0 then
      c1 := c1 and $ffff + c1 shr 16;
    inc(dword(baseAddress), 2);
  end;
  if odd(size) then begin
    c1 := c1 + byte(baseAddress^);
    c1 := c1 and $ffff + c1 shr 16;
  end;
  c1 := word(c1 and $ffff + c1 shr 16);
  nh^.OptionalHeader.CheckSum := c1 + size;
end;

function WriteConfig(driver: string; const drvCfg: TDriverConfig; fixChecksum: boolean) : boolean;
var fh, map  : dword;
    buf      : pointer;
    c1       : dword;
begin
  result := false;
  fh := CreateFile(pchar(driver), GENERIC_READ or GENERIC_WRITE, 0, nil, OPEN_EXISTING, 0, 0);
  if fh <> dword(-1) then begin
    map := CreateFileMapping(fh, nil, PAGE_READWRITE, 0, 0, nil);
    if map <> 0 then begin
      buf := MapViewOfFile(map, FILE_MAP_ALL_ACCESS, 0, 0, 0);
      if buf <> nil then begin
        c1 := PosPChar(PAnsiChar(@drvCfg), buf, sizeOf(drvCfg.Magic), GetFileSize(fh, nil));
        if c1 > 0 then begin
          Move(drvCfg, pointer(dword(buf) + c1)^, sizeOf(drvCfg));
          CalcCheckSum(buf, GetFileSize(fh, nil));
          result := true;
        end else
          WriteLn('This doesn''t seem to be a madCodeHook injection driver.');
        UnmapViewOfFile(buf);
      end else
        WriteLn('Mapping the file view for driver "' + driver + '" failed.');
      CloseHandle(map);
    end else
      WriteLn('Creating a file mapping for driver "' + driver + '" failed.');
    CloseHandle(fh);
  end else
    WriteLn('Opening the driver "' + driver + '" failed.');
end;

var i1 : integer;
    s1 : string;
    driver            : string  = '';
    name              : string  = '';
    dlls              : string  = '';
    unsafeStopAllowed : boolean = false;
    safeStopAllowed   : boolean = false;
    stopDisallowed    : boolean = false;
    fixChecksum       : boolean = false;
    drvCfg            : TDriverConfig;
begin
  ExitCode := 1;
  if ParamCount < 2 then begin
    WriteLn('madConfigDrv v' + FileVersionToStr(GetFileVersion(ParamStr(0))) + ', madshi.net');
    WriteLn;
    WriteLn('- add your hook dlls to the list of allowed dlls');
    WriteLn('- define whether the driver can be stopped or not');
    WriteLn;
    WriteLn('madConfigDrv driver.sys drivername [hook1.dll] [...] [hook40.dll] [-options]');
    WriteLn('-unsafeStopAllowed  driver can be stopped by anyone at any time');
    WriteLn('-safeStopAllowed    driver can be stopped only...');
    WriteLn('                    (1) if no dll injection is active and');
    WriteLn('                    (2) by using a special madCodeHook API');
    WriteLn('-stopDisallowed     driver can''t be stopped at all');
    WriteLn;
    WriteLn('The driver must not be signed yet.');
    WriteLn('Sign the driver after having allowed all your hook dlls.');
    exit;
  end;
  for i1 := 1 to ParamCount do begin
    s1 := ParamStr(i1);
    case i1 of
      1 :  driver := s1;
      2 :  name := s1;
      else if (s1[1] in ['-', '/']) and
              ( IsTextEqual(Copy(s1, 2, maxInt), 'unsafeStopAllowed') or
                IsTextEqual(Copy(s1, 2, maxInt), 'safeStopAllowed'  ) or
                IsTextEqual(Copy(s1, 2, maxInt), 'stopDisallowed'   )    ) then begin
             unsafeStopAllowed := IsTextEqual(Copy(s1, 2, maxInt), 'unsafeStopAllowed');
             safeStopAllowed   := IsTextEqual(Copy(s1, 2, maxInt), 'safeStopAllowed');
             stopDisallowed    := IsTextEqual(Copy(s1, 2, maxInt), 'stopDisallowed');
           end else
             if (s1[1] in ['-', '/']) and
                IsTextEqual(Copy(s1, 2, maxInt), 'fixChecksum') then begin
               fixChecksum := true;
             end else
               if s1[1] in ['-', '/'] then begin
                 WriteLn('The option "' + s1 + '" is unknown.');
                 exit;
               end else
                 if GetFileAttributes(pchar(s1)) = dword(-1) then begin
                   WriteLn('The file "' + s1 + '" could not be found.');
                   exit;
                 end else
                   dlls := dlls + '|' + s1;
    end;
  end;
  Delete(dlls, 1, 1);
  KillChar(driver, '"');
  KillChar(name, '"');
  if driver = '' then begin
    WriteLn('Please specify the driver file name/path.');
    exit;
  end;
  if Length(name) >= 40 then begin
    WriteLn('The driver name may only be 39 characters or less.');
    exit;
  end;
  if Length(name) < 3 then begin
    WriteLn('The driver name must be at least 3 characters long.');
    exit;
  end;
  if not ReadConfig(driver, drvCfg) then
    exit;
  ZeroMemory(@drvCfg.Name, sizeOf(drvCfg.Name));
  Move(name[1], drvCfg.Name, Length(name));
  if stopDisallowed then
    drvCfg.Flags := drvCfg.Flags and (not 3);
  if safeStopAllowed and (not unsafeStopAllowed) then
    drvCfg.Flags := (drvCfg.Flags and (not 2)) or 1;
  if unsafeStopAllowed then
    drvCfg.Flags := drvCfg.Flags or 3;
  for i1 := 1 to SubStrCount(dlls) do
    if not AddDll(SubStr(dlls, i1), drvCfg) then
      exit;
  if not WriteConfig(driver, drvCfg, fixChecksum) then
    exit;    
  ExitCode := 0;
end.
