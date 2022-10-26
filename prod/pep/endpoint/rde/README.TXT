################################################################################
#
# NextLabs Device Enforcement (NLDevEnf)
#
################################################################################

Change Log
----------
29_Jul_2008 Changes to support Win2K.  WdfDeviceQueryProperty for removal policy
            not supported.  Instead issue IRP_MN_QUERY_CAPABILITIES to determine
            if the device is removable when WdfDeviceQueryProperty fails.
05_Aug_2008 WinLogon service registration information.
09_Dec_2009 Update source file descriptions.

The NLDevEnf PnP device filter and user-mode library for it allow control of
attached devices to the system.  When a device is attached (arrives) its
characteristics such as Vendor ID, Product ID, Serial Number, and Setup Class
are evaluated.  If the device may not be used, all IO directed at that device
will be failed.

Use cases:
  (1) Restrict use of a device by its characteristics.

WinLogon Service Registration
-----------------------------
[HKLM]\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon\Notify\NLDevEnfPlugin

Asynchronous (REG_DWORD) = 1
DllName (REG_SZ)         = [path to nl_devenf_plugin.dll]
Impersonate (REG_DWORD)  = 0x0
Logoff (REG_SZ)          = LoggofEvent
Logon (REG_SZ)           = LogonEvent
StartShell (REG_SZ)      = StartShellEvent

Interface (API)
---------------
  Name                        Description
  -----------------------------------------------------------------------------
  Device_GetDevices           Retreive devices on the system.     
  Device_TestForArrival       Test for device arrival (non-blocking).
  Device_WaitForArrival       Wait for device arrival (blocking).
  Device_Enable               Enable device.
  Device_Disable              Disable device.
  Device_GetContext           Retreive user-mode device context.
  Device_GetContextSize       Retreive user-mode device context size.
  Device_SetContext           Set user-mode device context.

  See user/nl_devenf_lib.h for details.

Files
  driver/nl_devenf.c                NLDevEnf PnP class filter driver.
  inc/nl_devinf.h
  user/Makefile.inc                 NLDevEnf Plug-in makefile.
  user/include/nl_devenf_lib.h      NLDevEnf user-mode library interface file.
  user/include/nl_devenf_device.h   Device helper (i.e. determine attributes).
  user/src/nl_devenf_lib.cpp        NLDevEnf user-mode library to control PnP devices.
  user/src/nl_devenf_device.cpp     Device helper implementation.
  user/src/nl_devenf_sdk_wraper.cpp NLDevEnf SDK evaluation front-end.
  user/nl_devenf_plugin.cpp         NLDevEnf Policy Controller plug-in

  support/                    Support files.
  support/coinstaller_1.7     Co-installer for KMDF 1.7

Targets
  nl_devenf.sys               NLDevEnf PnP device filter.
  nl_devenf_plugin.dll        Device Enforcement Policy Controller plug-in.

Notes
-----
Windows XP may not boot while a FireWire disk is attached.  This is a known problem not related
to NLDevEnf class filter.

Device                Class
----------------------------------------
FireWire (storage)    firewire
Imaging               image
Network Adapter       net
Bluetooth             bluetooth
USB (unknown)         usb
USB Hub               hub
USB Storage           storage

Compatible ID         Meaning
-----------------------------------------------
Class_06              Imaging
Class_08              USB Storage (USBSTOR)
Class_09              USB Hub (Generic) (WinXP)
HubClass              USB Hub (Generic) (Win2K)

Format:
device://[class]/[manufacturer]/[product]/[serial_number]

Examples:

Device                           Meaning
------------------------------------------------------------------
device://usb/storage/**          All USB storage devices
device://usb/hub/**              All USB Hubs
device://usb/image/0def/**       USB Image device by manufacturer 0def
device://usb/net/**              All USB network devices
device://**                      All devices
device://usb/*/0def/**           All USB devices by manufacturer 0def
device://firewire/**             All FireWire storage devices
