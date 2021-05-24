#include "StdAfx.h"
#include "AnticheatManager.h"
#include "CheatQueueManager.h"

void CAnticheatManager::OnKeyPress(int32_t iKeyIdx, bool bStatus)
{
	m_mapKeyStatusList[iKeyIdx] = bStatus;
}
bool CAnticheatManager::GetKeyStatus(int32_t iKeyIdx)
{
	if (m_mapKeyStatusList.find(iKeyIdx) == m_mapKeyStatusList.end())
		return true;
	return m_mapKeyStatusList[iKeyIdx];
}

LRESULT CALLBACK WindowMessageHandler(int32_t code, WPARAM wParam, LPARAM lParam)
{
	MSG * message = (MSG*)lParam;

	if (CAnticheatManager::Instance().GetMainWindow() == message->hwnd)
	{
		switch (message->message)
		{
			case WM_LBUTTONDOWN: 
			{
				CAnticheatManager::Instance().OnKeyPress(0, true);
				break;
			}
			case WM_LBUTTONUP:
			{
				CAnticheatManager::Instance().OnKeyPress(0, false);
				break;
			}
			case WM_MBUTTONDOWN: 
			{
				CAnticheatManager::Instance().OnKeyPress(1, true);
				break;
			}
			case WM_MBUTTONUP:
			{
				CAnticheatManager::Instance().OnKeyPress(1, false);
				break;
			}
			case WM_RBUTTONDOWN: 
			{
				CAnticheatManager::Instance().OnKeyPress(2, true);
				break;
			}
			case WM_RBUTTONUP:
			{
				CAnticheatManager::Instance().OnKeyPress(2, false);
				break;
			}

			default:
				break;
		}

		return CallNextHookEx(0, code, wParam, lParam);
	}

	return 1;
}

void CAnticheatManager::InitializeWindowMessageHook()
{
	m_hkMessageHook = SetWindowsHookExA(WH_GETMESSAGE, WindowMessageHandler, 0, GetWindowThreadProcessId(m_hWnd, 0));
}
void CAnticheatManager::DestroyWindowMessageHook()
{
	UnhookWindowsHookEx(m_hkMessageHook);
}


LRESULT CALLBACK RawWindowHookMsgProc(HWND hWnd, uint32_t message, WPARAM wParam, LPARAM lParam) 
{
	if (CAnticheatManager::Instance().GetMainWindow() == hWnd)
	{
		switch (message)
		{
			// Mouse
		case WM_LBUTTONDOWN: 
		{
			if (CAnticheatManager::Instance().GetKeyStatus(0) == false)
			{
				CCheatDetectQueueMgr::Instance().AppendDetection(LMOUSE_INJECTION, GetLastError(), "");
			}
			break;
		}
		case WM_MBUTTONDOWN: 
		{
			if (CAnticheatManager::Instance().GetKeyStatus(1) == false)
			{
				CCheatDetectQueueMgr::Instance().AppendDetection(MMOUSE_INJECTION, GetLastError(), "");
			}
			break;
		}
		case WM_RBUTTONDOWN: 
		{	
			if (CAnticheatManager::Instance().GetKeyStatus(2) == false)
			{
				CCheatDetectQueueMgr::Instance().AppendDetection(RMOUSE_INJECTION, GetLastError(), "");
			}
			break;
		}

		case WM_INPUT: 
		{
			if (GET_RAWINPUT_CODE_WPARAM(wParam) == RIM_INPUT)
			{
				uint8_t *lpb = nullptr;
				uint32_t dwSize = sizeof(RAWINPUT);
				GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));

				lpb = new uint8_t[dwSize];

				if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize) 
				{
					delete[] lpb;
					return 0;
				}

				auto pInput = (RAWINPUT*)(lpb);
				if (!pInput->header.hDevice)
				{
					CCheatDetectQueueMgr::Instance().AppendDetection(WMINPUT_NULL_DEVICE, GetLastError(), "");

					delete[] lpb;
					return 0;
				}

#if 0
				char szDeviceName[1024];
				uint32_t uiDeviceNameSize = sizeof(szDeviceName);
				if (GetRawInputDeviceInfoA(pInput->header.hDevice, RIDI_DEVICENAME, szDeviceName, &uiDeviceNameSize) != -1)
				{
					TraceError("Raw device: %s", szDeviceName);
				}

				if (pInput->header.dwType == RIM_TYPEMOUSE)
				{
					auto pMouseData = pInput->data.mouse;

					auto mouseFlags = pMouseData.usFlags;

					auto bInjected = (mouseFlags & LLMHF_INJECTED) == LLMHF_INJECTED;
					auto bInjected2 = (mouseFlags & LLMHF_LOWER_IL_INJECTED) == LLMHF_LOWER_IL_INJECTED;
					auto bInjected3 = (mouseFlags & MOUSE_MOVE_ABSOLUTE);

					TraceError("inj: %d-%d-%d", bInjected, bInjected2, bInjected3);
				}
#endif

				delete[] lpb;
			}
			break;
		}

		default: 
			break;
		}

		return CAnticheatManager::Instance().GetWindowMsgProc()(hWnd, message, wParam, lParam);
	}

	return 0;
}

void CAnticheatManager::InitializeRawWindowHook()
{
	WNDPROC wndpOldProc = nullptr;
	if (IsWindowUnicode(m_hWnd))
		wndpOldProc = (WNDPROC)SetWindowLongPtrW(m_hWnd, GWLP_WNDPROC, (LONG_PTR)RawWindowHookMsgProc);
	else
		wndpOldProc = (WNDPROC)SetWindowLongPtrA(m_hWnd, GWLP_WNDPROC, (LONG_PTR)RawWindowHookMsgProc);

	if (wndpOldProc)
	{
		m_wndpOldProc = wndpOldProc;

		RAWINPUTDEVICE rid;
		rid.usUsagePage = 0x01;
		rid.usUsage = 0x06;
		rid.dwFlags = RIDEV_INPUTSINK;
		rid.hwndTarget = m_hWnd;

		if (!RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE))) 
		{
			if (IsWindowUnicode(m_hWnd))
				SetWindowLongPtrW(m_hWnd, GWLP_WNDPROC, (LONG_PTR)wndpOldProc);
			else 
				SetWindowLongPtrA(m_hWnd, GWLP_WNDPROC, (LONG_PTR)wndpOldProc);
		}
	}
}

void CAnticheatManager::DestroyRawWindowHook()
{
	if (IsWindowUnicode(m_hWnd))
		SetWindowLongPtrW(m_hWnd, GWLP_WNDPROC, (LONG_PTR)m_wndpOldProc);
	else
		SetWindowLongPtrA(m_hWnd, GWLP_WNDPROC, (LONG_PTR)m_wndpOldProc);
}


