#include <cpplogger/cpplogger.h>
#include <windows.h>

#include <strsafe.h>

#include "context.h"
#include "winhookloop.h"

extern Logger::Logger *Log;
extern HANDLE requestEvent;

LRESULT WINAPI keyboardCallback(int code, WPARAM wParam, LPARAM lParam) {
  KBDLLHOOKSTRUCT *kbx{nullptr};

  if (code == HC_ACTION) {
    kbx = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);
  }

  HRESULT hr{};
  wchar_t *msg = new wchar_t[256]{};

  if (kbx != nullptr) {
    hr = StringCbPrintfW(
        msg, 255,
        L"keyboardCallback(wParam=%d, vkCode=%d, scanCode=%d, flags=%d)",
        wParam, kbx->vkCode, kbx->scanCode, kbx->flags);
  }
  if (!FAILED(hr)) {
    Log->Info(msg, GetCurrentThreadId(), __LONGFILE__);

    delete[] msg;
    msg = nullptr;
  }
  if (!SetEvent(requestEvent)) {
    Log->Warn(L"Failed to send event", GetCurrentThreadId(), __LONGFILE__);
  }

  return CallNextHookEx(0, code, wParam, lParam);
}

DWORD WINAPI winhookLoop(LPVOID context) {
  Log->Info(L"Start winhook loop thread", GetCurrentThreadId(), __LONGFILE__);

  HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

  if (FAILED(hr)) {
    Log->Fail(L"Failed to call CoInitializeEx", GetCurrentThreadId(),
              __LONGFILE__);
    return hr;
  }

  WinhookLoopContext *ctx = static_cast<WinhookLoopContext *>(context);

  if (ctx == nullptr) {
    Log->Fail(L"Failed to obtain context", GetCurrentThreadId(), __LONGFILE__);
    return E_FAIL;
  }

  requestEvent = ctx->RequestEvent;

  HHOOK hHook = SetWindowsHookExW(WH_KEYBOARD_LL, keyboardCallback,
                                  GetModuleHandleW(nullptr), 0);

  if (hHook == nullptr) {
    Log->Fail(L"Failed to set hook", GetCurrentThreadId(), __LONGFILE__);
    return E_FAIL;
  }

  Log->Fail(L"Register", GetCurrentThreadId(), __LONGFILE__);

  MSG msg;
  bool isActive{false};

  while (GetMessageW(&msg, nullptr, 0, 0)) {
    HANDLE waitArray[1] = {ctx->QuitEvent};
    DWORD waitResult = WaitForMultipleObjects(1, waitArray, FALSE, 0);

    switch (waitResult) {
    case WAIT_OBJECT_0 + 0: // ctx->QuitEvent
      isActive = false;
      break;
    }
    if (isActive) {
      break;
    }
  }
  if (!UnhookWindowsHookEx(hHook)) {
    Log->Info(L"Failed to call UnhookWindowsHookEx()", GetCurrentThreadId(),
              __LONGFILE__);
  }

  CoUninitialize();

  Log->Info(L"End winhook loop thread", GetCurrentThreadId(), __LONGFILE__);

  return S_OK;
}
