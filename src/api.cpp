#include <cpplogger/cpplogger.h>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <windows.h>

#include "api.h"
#include "context.h"
#include "logloop.h"
#include "requestloop.h"
#include "util.h"
#include "winhookloop.h"

extern Logger::Logger *Log;

bool isActive{false};
std::mutex apiMutex;

RequestLoopContext *requestLoopCtx{nullptr};
LogLoopContext *logLoopCtx{nullptr};
WinhookLoopContext *winhookLoopCtx{nullptr};

HANDLE logLoopThread{nullptr};
HANDLE requestLoopThread{nullptr};
HANDLE winhookLoopThread{nullptr};

extern HANDLE requestEvent;

void __stdcall Setup(int32_t *code, int32_t logLevel) {
  std::lock_guard<std::mutex> lock(apiMutex);

  if (code == nullptr) {
    return;
  }
  if (isActive) {
    Log->Warn(L"Already initialized", GetCurrentThreadId(), __LONGFILE__);
    *code = -1;
    return;
  }

  Log = new Logger::Logger(L"KeyboardNode", L"v0.1.0-develop", 4096);

  Log->Info(L"Setup CoreNode", GetCurrentThreadId(), __LONGFILE__);

  logLoopCtx = new LogLoopContext();

  logLoopCtx->QuitEvent =
      CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);

  if (logLoopCtx->QuitEvent == nullptr) {
    Log->Fail(L"Failed to create event", GetCurrentThreadId(), __LONGFILE__);
    *code = -1;
    return;
  }

  Log->Info(L"Create log loop thread", GetCurrentThreadId(), __LONGFILE__);

  logLoopThread = CreateThread(nullptr, 0, logLoop,
                               static_cast<void *>(logLoopCtx), 0, nullptr);

  if (logLoopThread == nullptr) {
    Log->Fail(L"Failed to create thread", GetCurrentThreadId(), __LONGFILE__);
    *code = -1;
    return;
  }

  requestEvent =
      CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);

  if (requestEvent == nullptr) {
    Log->Fail(L"Failed to create event", GetCurrentThreadId(), __LONGFILE__);
    *code = -1;
    return;
  }

  requestLoopCtx = new RequestLoopContext();
  requestLoopCtx->RequestEvent = requestEvent;
  requestLoopCtx->QuitEvent =
      CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);

  if (requestLoopCtx->QuitEvent == nullptr) {
    Log->Fail(L"Failed to create event", GetCurrentThreadId(), __LONGFILE__);
    *code = -1;
    return;
  }

  Log->Info(L"Create request loop thread", GetCurrentThreadId(), __LONGFILE__);

  requestLoopThread = CreateThread(
      nullptr, 0, requestLoop, static_cast<void *>(requestLoopCtx), 0, nullptr);

  if (requestLoopThread == nullptr) {
    Log->Fail(L"Failed to create thread", GetCurrentThreadId(), __LONGFILE__);
    *code = -1;
    return;
  }

  winhookLoopCtx = new WinhookLoopContext();
  winhookLoopCtx->RequestEvent = requestEvent;
  winhookLoopCtx->QuitEvent =
      CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);

  if (winhookLoopCtx->QuitEvent == nullptr) {
    Log->Fail(L"Failed to create event", GetCurrentThreadId(), __LONGFILE__);
    *code = -1;
    return;
  }

  Log->Info(L"Create winhook loop thread", GetCurrentThreadId(), __LONGFILE__);

  winhookLoopThread = CreateThread(
      nullptr, 0, winhookLoop, static_cast<void *>(winhookLoopCtx), 0, nullptr);

  if (winhookLoopThread == nullptr) {
    Log->Fail(L"Failed to create thread", GetCurrentThreadId(), __LONGFILE__);
    *code = -1;
    return;
  }

  Log->Info(L"Complete setup KeyboardNode", GetCurrentThreadId(), __LONGFILE__);

  isActive = true;
}

void __stdcall Teardown(int32_t *code) {
  std::lock_guard<std::mutex> lock(apiMutex);

  if (code == nullptr) {
    return;
  }
  if (!isActive) {
    *code = -1;
    return;
  }

  Log->Info(L"Teardown KeyboardNode", GetCurrentThreadId(), __LONGFILE__);

  if (!SetEvent(winhookLoopCtx->QuitEvent)) {
    Log->Fail(L"Failed to send event", GetCurrentThreadId(), __LONGFILE__);
    *code = -1;
    return;
  }

  WaitForSingleObject(winhookLoopThread, INFINITE);
  SafeCloseHandle(&winhookLoopThread);

  Log->Info(L"Delete winhook loop thread", GetCurrentThreadId(), __LONGFILE__);

  if (!SetEvent(requestLoopCtx->QuitEvent)) {
    Log->Fail(L"Failed to send event", GetCurrentThreadId(), __LONGFILE__);
    *code = -1;
    return;
  }

  WaitForSingleObject(requestLoopThread, INFINITE);
  SafeCloseHandle(&requestLoopThread);

  Log->Info(L"Delete request loop thread", GetCurrentThreadId(), __LONGFILE__);

  SafeCloseHandle(&requestEvent);

  delete winhookLoopCtx;
  winhookLoopCtx = nullptr;

  delete requestLoopCtx;
  requestLoopCtx = nullptr;

  Log->Info(L"Complete teardown CoreNode", GetCurrentThreadId(), __LONGFILE__);

  if (!SetEvent(logLoopCtx->QuitEvent)) {
    Log->Fail(L"Failed to send event", GetCurrentThreadId(), __LONGFILE__);
    *code = -1;
    return;
  }

  WaitForSingleObject(logLoopThread, INFINITE);
  SafeCloseHandle(&logLoopThread);

  isActive = false;
}
