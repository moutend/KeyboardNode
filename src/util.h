#pragma once

#include <UIAutomationClient.h>
#include <windows.h>

template <class T> void SafeRelease(T **ppT) {
  if (*ppT) {
    (*ppT)->Release();
    *ppT = nullptr;
  }
}

void SafeCloseHandle(HANDLE *pHandle);
bool isEmptyIUIAutomationElement(IUIAutomationElement *pElement);
HRESULT logIUIAutomationElement(IUIAutomationElement *pElement, const wchar_t *longfile);
HRESULT printElements(IUIAutomationElementArray *pElements);
