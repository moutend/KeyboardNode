#pragma once

#include <windows.h>

struct LogLoopContext {
  bool IsActive = true;
  const wchar_t *FullLogPath = nullptr;
};

struct RequestLoopContext {
  HANDLE RequestEvent = nullptr;
  HANDLE QuitEvent = nullptr;
};

struct WinhookLoopContext {
  HANDLE RequestEvent = nullptr;
  HANDLE QuitEvent = nullptr;
  bool IsActive = true;
};
