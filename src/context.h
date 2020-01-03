#pragma once

#include <windows.h>

struct LogLoopContext {
  HANDLE QuitEvent = nullptr;
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
