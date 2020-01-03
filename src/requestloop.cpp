#include <cpplogger/cpplogger.h>
#include <cpprest/http_client.h>
#include <iostream>

#include "context.h"
#include "requestloop.h"

using namespace web;
using namespace web::http;
using namespace web::http::client;

extern Logger::Logger *Log;

pplx::task<http_response> notifyKeyDown() {
  json::value c0;

  c0[U("type")] = json::value(1);
  c0[U("value")] = json::value(6);

  json::value postData;

  postData[U("commands")][0] = c0;

  http_client client(U("http://localhost:7902/v1/audio/command?force=true"));

  return client.request(methods::POST, U(""), postData.serialize(),
                        U("application/json"));
}

DWORD WINAPI requestLoop(LPVOID context) {
  Log->Info(L"Start request loop thread", GetCurrentThreadId(), __LONGFILE__);

  HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

  if (FAILED(hr)) {
    Log->Fail(L"Failed to call CoInitializeEx", GetCurrentThreadId(),
              __LONGFILE__);
    return hr;
  }

  RequestLoopContext *ctx = static_cast<RequestLoopContext *>(context);

  if (ctx == nullptr) {
    Log->Fail(L"Failed to obtain context", GetCurrentThreadId(), __LONGFILE__);
    return E_FAIL;
  }

  bool isActive{true};

  while (isActive) {
    HANDLE waitArray[2] = {ctx->QuitEvent, ctx->RequestEvent};
    DWORD waitResult = WaitForMultipleObjects(2, waitArray, FALSE, INFINITE);

    switch (waitResult) {
    case WAIT_OBJECT_0 + 0: // ctx->QuitEvent
      isActive = false;
      continue;
    case WAIT_OBJECT_0 + 1: // ctx->RequestEvent
      break;
    }
    try {
      notifyKeyDown().wait();

      Log->Info(L"Send HTTP request", GetCurrentThreadId(), __LONGFILE__);
    } catch (...) {
      Log->Info(L"Failed to send HTTP request", GetCurrentThreadId(),
                __LONGFILE__);
    }
  }

  CoUninitialize();

  Log->Info(L"End request loop thread", GetCurrentThreadId(), __LONGFILE__);

  return S_OK;
}
