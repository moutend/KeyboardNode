#include <UIAutomationClient.h>
#include <cpplogger/cpplogger.h>
#include <cstring>
#include <windows.h>

extern Logger::Logger *Log;

void SafeCloseHandle(HANDLE *pHandle) {
  if (pHandle) {
    CloseHandle(*pHandle);
    *pHandle = nullptr;
  }
}

bool isEmptyIUIAutomationElement(IUIAutomationElement *pElement) {
  if (pElement == nullptr) {
    return true;
  }

  RECT boundingRectangle{0, 0, 0, 0};

  if (FAILED(pElement->get_CurrentBoundingRectangle(&boundingRectangle))) {
    return true;
  }
  if (boundingRectangle.left == 0 && boundingRectangle.top == 0 &&
      boundingRectangle.right == 0 && boundingRectangle.bottom == 0) {
    return true;
  }

  wchar_t *currentName{nullptr};
  wchar_t *cachedName{nullptr};

  if (FAILED(pElement->get_CurrentName(&currentName))) {
    currentName = nullptr;
  }
  if (FAILED(pElement->get_CachedName(&cachedName))) {
    cachedName = nullptr;
  }
  if (currentName == nullptr && cachedName == nullptr) {
    return true;
  }
  if (currentName != nullptr && std::wcslen(currentName) == 0) {
    return true;
  }
  if (cachedName != nullptr && std::wcslen(cachedName) == 0) {
    return true;
  }
  if (cachedName != nullptr) {
    SysFreeString(cachedName);
    cachedName = nullptr;
  }
  if (currentName != nullptr) {
    SysFreeString(currentName);
    currentName = nullptr;
  }

  return false;
}

HRESULT logIUIAutomationElement(IUIAutomationElement *pElement, const wchar_t *longfile) {
  if (pElement == nullptr) {
    Log->Info(L"<nullptr>", GetCurrentThreadId(), longfile);
    return S_OK;
  }

  CONTROLTYPEID controlTypeId{};

  if (FAILED(pElement->get_CachedControlType(&controlTypeId))) {
    controlTypeId = 0;
  }

  wchar_t *controlName{nullptr};

  if (FAILED(pElement->get_CurrentLocalizedControlType(&controlName))) {
    controlName = nullptr;
  }

  wchar_t *name{nullptr};

  if (FAILED(pElement->get_CurrentName(&name))) {
    name = nullptr;
  }

  UIA_HWND nativeWindow{};

  if (FAILED(pElement->get_CachedNativeWindowHandle(&nativeWindow))) {
    nativeWindow = 0;
  }

  wchar_t *frameworkId{nullptr};

  if (FAILED(pElement->get_CachedFrameworkId(&frameworkId))) {
    frameworkId = nullptr;
  }

  wchar_t *className{nullptr};

  if (FAILED(pElement->get_CachedClassName(&className))) {
    className = nullptr;
  }

  std::wstringstream wss;

  wss << L"ControlTypeId=";
  wss << controlTypeId;
  wss << L",ControlName=";

  if (controlName == nullptr) {
    wss << "<nullptr>";
  } else {
    wss << controlName;
  }

  wss << L",Name=";

  if (name == nullptr) {
    wss << L"<nullptr>";
  } else {
    wss << name;
  }

  wss << L",FrameworkId=";

  if (frameworkId == nullptr) {
    wss << L"<nullptr>";
  } else {
    wss << frameworkId;
  }

  wss << L",ClassName=";

  if (className == nullptr) {
    wss << L"<nullptr>";
  } else {
    wss << className;
  }

  wss << L",NativeWindow=";
  wss << nativeWindow;

  Log->Info(wss.str(), GetCurrentThreadId(), __LONGFILE__);

  if (controlName != nullptr) {
    SysFreeString(controlName);
    controlName = nullptr;
  }
  if (name != nullptr) {
    SysFreeString(name);
    name = nullptr;
  }
  if (frameworkId != nullptr) {
    SysFreeString(frameworkId);
    frameworkId = nullptr;
  }
  if (className != nullptr) {
    SysFreeString(className);
    className = nullptr;
  }

  return S_OK;
}

HRESULT printElements(IUIAutomationElementArray *pElements) {
  if (pElements == nullptr) {
    return S_OK;
  }

  int length{};

  if (FAILED(pElements->get_Length(&length))) {
    return E_FAIL;
  }

  IUIAutomationElement *pElement{nullptr};

  for (int i = 0; i < length; i++) {
    if (FAILED(pElements->GetElement(i, &pElement))) {
      continue;
    }

    CONTROLTYPEID controlTypeId{};

    if (FAILED(pElement->get_CurrentControlType(&controlTypeId))) {
      // todo
    }

    wchar_t *name{nullptr};

    if (FAILED(pElement->get_CurrentName(&name))) {
      // todo;
    }

    std::wstringstream wss;
    wss << (i + 1);
    wss << L" ";
    wss << L"ControlTypeId=";
    wss << controlTypeId;
    wss << L",";
    wss << L"Name=";
    if (name == nullptr) {
      wss << L"<nullptr>";
    } else {
      wss << name;
    }
    Log->Info(wss.str(), GetCurrentThreadId(), __LONGFILE__);

    if (name != nullptr) {
      SysFreeString(name);
      name = nullptr;
    }
    pElement = nullptr;
  }

  return S_OK;
}

/*@@@begin
HRESULT listElementsByFindAll(IUIAutomationElement *pParent) {
  if (pParent == nullptr) {
    Log->Warn(L"element is nullptr", GetCurrentThreadId(),  __LONGFILE__);
    return S_OK;
  }

  IUIAutomationCondition *pCondition{nullptr};
  IUIAutomationElementArray *pElements{nullptr};

  if (FAILED(pUIAutomation->CreateTrueCondition(&pCondition))) {
    goto CLEANUP;
  }
  if (FAILED(pParent->FindAll(TreeScope_Subtree, pCondition, &pElements))) {
    goto CLEANUP;
  }

  int length;

  if (FAILED(pElements->get_Length(&length))) {
    return E_FAIL;
  }

  IUIAutomationElement *pElement{nullptr};

  for (int i = 0; i < length; i++) {
    if (FAILED(pElements->GetElement(i, &pElement))) {
      continue;
    }

    printElement(pElement);

    if (pElement != nullptr) {
      pElement->Release();
      pElement = nullptr;
    }
  }

CLEANUP:
  if (pElements != nullptr) {
    pElements->Release();
    pElements = nullptr;
  }
  if (pCondition != nullptr) {
    pCondition->Release();
    pCondition = nullptr;
  }

  return S_OK;
}
@@@end*/

/*@@@begin
HRESULT listElementsByTreeWalker(int depth, IUIAutomationElement *pParent) {
  if (pParent == nullptr) {
    Log->Warn(L"element is nullptr", GetCurrentThreadId(),  __LONGFILE__);
    return S_OK;
  }

  IUIAutomationElement *pElement{nullptr};

  if (FAILED(pWalker->GetFirstChildElement(pParent, &pElement))) {
    goto CLEANUP;
  }

  printElement(depth, 0, pElement);

  if (pElement != nullptr) {
    listElementsByTreeWalker(depth + 1, pElement);
  }

  IUIAutomationElement *pNext{nullptr};
  int index{};

  while (true) {
    index += 1;

    if (FAILED(pWalker->GetNextSiblingElement(pElement, &pNext))) {
      break;
    }
    if (pNext == nullptr) {
      break;
    }

    printElement(depth, index, pNext);

    if (pNext != nullptr) {
      listElementsByTreeWalker(depth + 1, pNext);
    }

    pElement->Release();
    pElement = pNext;
  }

CLEANUP:
  if (pNext != nullptr) {
    pNext->Release();
    pNext = nullptr;
  }
  if (pElement != nullptr) {
    pElement->Release();
    pElement = nullptr;
  }

  return S_OK;
}
@@@end*/
