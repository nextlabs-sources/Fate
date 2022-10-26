#pragma once

extern std::list<std::wstring> gMsgBlocks;
extern std::list<std::wstring> gVoiceBlocks;
extern std::list<std::wstring> gConfBlocks;
extern std::list<std::wstring> gAppBlocks;

void LoadBlocks(HMODULE Module);

bool IsBlock(wchar_t const* Name,enum UCC_SESSION_TYPE SessionType);