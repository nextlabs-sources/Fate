#pragma once

bool HookDesktop(ICollaborateDesktop* pDesktop);
bool HookCollaborateFrames(ICollaborateFrames* pFrames);
bool HookSimple(ICollaborateSimple* pSimple);

bool HookMonitors(ICollaborateMonitors* pMonitors);
bool HookMonitor(ICollaborateMonitor *pMonitor);
bool HookCollaborateWindows(ICollaborateWindows* pWindows);
bool HookWindow(ICollaborateWindow* pWindow);

bool HookProcesses(ICollaborateProcesses* pProcesses);
bool HookFrame(ICollaborateFrame* pFrame);

bool HookProcess(ICollaborateProcess* pProcess);
bool HookStream(INpwRC_Stream* pStream);