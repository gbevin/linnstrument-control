#include "gamewave_NativeSystemEvents.h"

#include <iostream>
#include <Windows.h>
#include <tlhelp32.h>
#include <string.h>
#include <PowrProf.h>

#include <JuceHeader.h>

#include "gamewave_Application.h"
#include "gamewave_WinampInfoComponent.h"
#include "gamewave_WindowsMediaPlayerInfoComponent.h"

#define MAKE_APPCOMMAND_LPARAM(command, device, keys) MAKELPARAM(keys, (command & ~FAPPCOMMAND_MASK) | (device & FAPPCOMMAND_MASK))

#define EXE_WINDOWS_MEDIA_PLAYER L"wmplayer.exe"
#define EXE_WINAMP L"winamp.exe"
#define EXE_SPOTIFY L"spotify.exe"
#define EXE_ITUNES L"iTunes.exe"

using namespace gamewave;

namespace
{
	enum MediaApp
	{
		UNKNOWN,
		WINDOWS_MEDIA_PLAYER,
		WINAMP,
		SPOTIFY,
		ITUNES
	};

	struct MediaAppIdent
	{
		MediaApp type_;
		DWORD processId_;
		HWND hwnd_;
	};

	BOOL CALLBACK enumWindowsProc(HWND hWnd, LPARAM lParam)
	{
		MediaAppIdent *ident = (MediaAppIdent *)lParam;
		DWORD processId = 0;
		GetWindowThreadProcessId(hWnd, &processId);
		if (ident->processId_ == processId)
		{
			ident->hwnd_ = hWnd;
			return FALSE;
		}

		return TRUE;
	}

	WORD mediaKeyToVKey(MediaKey m)
	{
		switch (m)
		{
			case MediaPlay:
				return VK_MEDIA_PLAY_PAUSE;
			case MediaStop:
				return VK_MEDIA_STOP;
			case MediaPrevious:
				return VK_MEDIA_PREV_TRACK;
			case MediaNext:
				return VK_MEDIA_NEXT_TRACK;
			case MediaMute:
				return VK_VOLUME_MUTE;
			case MediaQuieter:
				return VK_VOLUME_DOWN;
			case MediaLouder:
				return VK_VOLUME_UP;
		}

		return 0;
	}

    struct ShowInfoMessage: juce::Message
    {
        ShowInfoMessage(const MediaApp type): type_ (type) {}
        ~ShowInfoMessage() {}
        
        const MediaApp type_;
    };
}

class NativeSystemEvents::impl_t : public juce::MessageListener
{
public:
	impl_t()
	{
		winampDialog_ = new GameWaveDialog(new WinampInfoComponent(), GameWaveApplication::getApp().getMainWindow(), 0, 0, DocumentWindow::closeButton|DocumentWindow::minimiseButton, true);
		winampDialog_->setName(String("Configure Winamp preferences"));

		windowsMediaPlayerDialog_ = new GameWaveDialog(new WindowsMediaPlayerInfoComponent(), GameWaveApplication::getApp().getMainWindow(), 0, 0, DocumentWindow::closeButton|DocumentWindow::minimiseButton, true);
		windowsMediaPlayerDialog_->setName(String("Windows Media Player needs manual activation"));
	}

	~impl_t()
	{
	}

    void handleMessage(const juce::Message &m)
	{
		ShowInfoMessage *msg = (ShowInfoMessage *)&m;
		switch(msg->type_)
		{
			case WINDOWS_MEDIA_PLAYER:
				windowsMediaPlayerDialog_->setVisible(true);
				break;
			case WINAMP:
				winampDialog_->setVisible(true);
				break;
			default:
				break;
		}
	}

	MediaAppIdent determineMediaApp()
	{
		MediaAppIdent ident = {UNKNOWN, 0, 0};

		HANDLE hProcessSnap;
		PROCESSENTRY32 pe32;

		hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hProcessSnap == INVALID_HANDLE_VALUE)
		{
            return ident;
		}

		pe32.dwSize = sizeof(PROCESSENTRY32);

		if (Process32First(hProcessSnap, &pe32))
		{
			do
			{
				if (_wcsicmp(pe32.szExeFile, EXE_WINDOWS_MEDIA_PLAYER) == 0)
				{
					ident.type_ = WINDOWS_MEDIA_PLAYER;
					ident.processId_ = pe32.th32ProcessID;

					if (!getGlobalProperties().containsKey(WINDOWS_MEDIA_PLAYER_INFO_SHOWN) || !getGlobalProperties().getBoolValue(WINDOWS_MEDIA_PLAYER_INFO_SHOWN, false))
					{
						postMessage(new ShowInfoMessage(ident.type_));
					}
					getGlobalProperties().setValue(WINDOWS_MEDIA_PLAYER_INFO_SHOWN, true);
					break;
				}
				else if (_wcsicmp(pe32.szExeFile, EXE_WINAMP) == 0)
				{
					ident.type_ = WINAMP;
					ident.processId_ = pe32.th32ProcessID;

					if (!getGlobalProperties().containsKey(WINAMP_INFO_SHOWN) || !getGlobalProperties().getBoolValue(WINAMP_INFO_SHOWN, false))
					{
					postMessage(new ShowInfoMessage(ident.type_));
					}
					getGlobalProperties().setValue(WINAMP_INFO_SHOWN, true);
					break;
				}
				else if (_wcsicmp(pe32.szExeFile, EXE_SPOTIFY) == 0)
				{
					ident.type_ = SPOTIFY;
					ident.processId_ = pe32.th32ProcessID;
					break;
				}
				else if (_wcsicmp(pe32.szExeFile, EXE_ITUNES) == 0)
				{
					ident.type_ = ITUNES;
					ident.processId_ = pe32.th32ProcessID;
					break;
				}
			}
			while(Process32Next(hProcessSnap, &pe32));
		}

		CloseHandle(hProcessSnap);

		/*
		if (ident.processId_ != 0)
		{
			EnumWindows(enumWindowsProc, (LPARAM)&ident);
		}
		*/

        return ident;
	}

	void sendVkKeyEvent(WORD vk, bool down)
	{
		INPUT Input[1];
		ZeroMemory(Input, sizeof(INPUT) * 1);
	
		Input[0].type = INPUT_KEYBOARD;
		Input[0].ki.wVk = vk;
		if (!down)
		{
			Input[0].ki.dwFlags = KEYEVENTF_KEYUP;
		}

		SendInput(1, Input, sizeof(INPUT));
	}

	void sendAppCommand(int command, MediaAppIdent ident)
	{
		if (0 == command) return;

		switch (command)
		{
			case APPCOMMAND_MEDIA_NEXTTRACK:
			case APPCOMMAND_MEDIA_PREVIOUSTRACK:
			case APPCOMMAND_MEDIA_STOP:
			case APPCOMMAND_MEDIA_PLAY_PAUSE:
			case APPCOMMAND_MEDIA_PLAY:
			case APPCOMMAND_MEDIA_PAUSE:
			case APPCOMMAND_MEDIA_RECORD:
			case APPCOMMAND_MEDIA_FAST_FORWARD:
			case APPCOMMAND_MEDIA_REWIND:
			case APPCOMMAND_MEDIA_CHANNEL_UP:
			case APPCOMMAND_MEDIA_CHANNEL_DOWN:
				{
					if (ident.hwnd_ == 0)
					{
						ident = determineMediaApp();
					}
				}
				break;
		}

		LPARAM lparam = MAKE_APPCOMMAND_LPARAM(command, 0, 0);

		HWND hwnd = ident.hwnd_;
		if (!hwnd)
		{
			hwnd = GetForegroundWindow();
		}
		if (!hwnd)
		{
			hwnd = (HWND)GameWaveApplication::getApp().getMainWindow()->getPeer()->getNativeHandle();
		}
		SendMessage(hwnd, WM_APPCOMMAND, (WPARAM)0, lparam);
	}

	void sendScanCodeKeyEvent(WORD scan, bool extended, bool down)
	{
		INPUT Input[1];
		ZeroMemory(Input, sizeof(INPUT) * 1);
	
		Input[0].type = INPUT_KEYBOARD;
		Input[0].ki.wScan = scan;
		Input[0].ki.dwFlags = KEYEVENTF_SCANCODE;
		if (extended)
		{
			Input[0].ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
		}
		if (!down)
		{
			Input[0].ki.dwFlags |= KEYEVENTF_KEYUP;
		}

		SendInput(1, Input, sizeof(INPUT));
	}

	void modifierEvent(const GameWaveModifiers &m, bool down)
    {
		if (m.isAnyModifierKeyDown())
		{
			if (m.isShiftDown())
			{
				sendScanCodeKeyEvent(0x2a, false, down);
			}
			if (m.isCtrlDown())
			{
				sendScanCodeKeyEvent(0x1d, false, down);
			}
			if (m.isAltDown())
			{
				sendScanCodeKeyEvent(0x38, false, down);
			}
			if (m.isWindowsDown())
			{
				sendScanCodeKeyEvent(0x5b, true, down);
			}
		}
    }

	void sendKeyEvent(int keycode, unsigned int nativecode, bool down)
	{
		INPUT Input[1];
		ZeroMemory(Input, sizeof(INPUT) * 1);
	
		Input[0].type = INPUT_KEYBOARD;
		if (keycode == KeyPress::playKey)
		{
			Input[0].ki.wVk = VK_MEDIA_PLAY_PAUSE;
		}
		else if (keycode == KeyPress::stopKey)
		{
			Input[0].ki.wVk = VK_MEDIA_STOP;
		}
		else if (keycode == KeyPress::fastForwardKey)
		{
			Input[0].ki.wVk = VK_MEDIA_NEXT_TRACK;
		}
		else if (keycode == KeyPress::rewindKey)
		{
			Input[0].ki.wVk = VK_MEDIA_PREV_TRACK;
		}
		else
		{
			Input[0].ki.wScan = ((nativecode & 0xff0000) >> 16);
			Input[0].ki.dwFlags = KEYEVENTF_SCANCODE;
			if (nativecode & 0x1000000)
			{
				Input[0].ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
			}
		}

		if (!down)
		{
			Input[0].ki.dwFlags |= KEYEVENTF_KEYUP;
		}

		SendInput(1, Input, sizeof(INPUT));
	}

	bool ensureShutdownPrivilege()
	{
		HANDLE hToken; 
		TOKEN_PRIVILEGES tkp; 
 
		// Get a token for this process. 
		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		{
			return false;
		}
 
		// Get the LUID for the shutdown privilege. 
		LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid); 
 
		tkp.PrivilegeCount = 1;  // one privilege to set    
		tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
 
		// Get the privilege for this process. 
		AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0); 
 
		return GetLastError() == ERROR_SUCCESS;
 	}

	void systemCommand(SystemCommand command)
	{
        switch (command)
        {
            case SystemLock:
				LockWorkStation();
                break;
            case SystemSleep:
				if (ensureShutdownPrivilege())
				{
					SetSuspendState(false, 0, false);
				}
                break;
            case SystemLogout:
				ExitWindowsEx(EWX_LOGOFF, SHTDN_REASON_MAJOR_APPLICATION | SHTDN_REASON_MINOR_MAINTENANCE | SHTDN_REASON_FLAG_PLANNED);
                break;
            case SystemRestart:
				if (ensureShutdownPrivilege())
				{
					ExitWindowsEx(EWX_REBOOT, SHTDN_REASON_MAJOR_APPLICATION | SHTDN_REASON_MINOR_MAINTENANCE | SHTDN_REASON_FLAG_PLANNED);
				}
                break;
            case SystemHalt:
				if (ensureShutdownPrivilege())
				{
					ExitWindowsEx(EWX_POWEROFF, SHTDN_REASON_MAJOR_APPLICATION | SHTDN_REASON_MINOR_MAINTENANCE | SHTDN_REASON_FLAG_PLANNED);
				}
                break;
        }
	}

    ScopedPointer<GameWaveDialog> winampDialog_;
    ScopedPointer<GameWaveDialog> windowsMediaPlayerDialog_;
};

NativeSystemEvents::NativeSystemEvents() : mouseButton1Down_(false),  mouseButton2Down_(false),  mouseButton3Down_(false), offsetX_(0), offsetY_(0), impl_(new impl_t())
{
    for (int i = 0; i < 3 ; ++i)
    {
        mouseDownPosition_[i][0] = -1;
        mouseDownPosition_[i][1] = -1;
    }
}

NativeSystemEvents::~NativeSystemEvents()
{
	delete impl_;
}

void NativeSystemEvents::keyDown(int keycode, unsigned int nativecode, GameWaveModifiers m)
{
	impl_->modifierEvent(m, true);
	impl_->sendKeyEvent(keycode, nativecode, true);
}

void NativeSystemEvents::keyUp(int keycode, unsigned int nativecode, GameWaveModifiers m)
{
	impl_->sendKeyEvent(keycode, nativecode, false);
	impl_->modifierEvent(m, false);
}

void NativeSystemEvents::capsLockDown(GameWaveModifiers m)
{
	impl_->modifierEvent(m, true);
	impl_->sendScanCodeKeyEvent(0x3a, false, true);
}

void NativeSystemEvents::capsLockUp(GameWaveModifiers m)
{
	impl_->sendScanCodeKeyEvent(0x3a, false, false);
	impl_->modifierEvent(m, false);
}

void NativeSystemEvents::modifierDown(GameWaveModifiers m)
{
    impl_->modifierEvent(m, true);
}

void NativeSystemEvents::modifierUp(GameWaveModifiers m)
{
    impl_->modifierEvent(m, false);
}

void NativeSystemEvents::moveMouse(int dx, int dy)
{
	POINT pt;
	if (mouseButton1Down_ && !isMouseDownContinuous(0))
	{
		pt.x = mouseDownPosition_[0][0];
		pt.y = mouseDownPosition_[0][1];
		dx = 0;
		dy = 0;
	}
	else if (mouseButton2Down_ && !isMouseDownContinuous(1))
	{
		pt.x = mouseDownPosition_[1][0];
		pt.y = mouseDownPosition_[1][1];
		dx = 0;
		dy = 0;
	}
	else if (mouseButton3Down_ && !isMouseDownContinuous(2))
	{
		pt.x = mouseDownPosition_[2][0];
		pt.y = mouseDownPosition_[2][1];
		dx = 0;
		dy = 0;
	}
	else
	{
		GetCursorPos(&pt);
		pt.x += dx;
		pt.y += dy;
	}
	SetCursorPos(pt.x, pt.y);

	INPUT Input[1];
    ZeroMemory(Input, sizeof(INPUT) * 1);
	
	Input[0].mi.dx = dx;
	Input[0].mi.dy = dy;
	Input[0].mi.dwFlags = MOUSEEVENTF_MOVE;
	SendInput(1, Input, sizeof(INPUT));
}

bool NativeSystemEvents::isMouseDownContinuous(int button)
{
    if (button >= 0 && button < 3)
    {
        return mouseDownPosition_[button][0] == -1 && mouseDownPosition_[button][1] == -1;
    }
    return false;
}

void NativeSystemEvents::mouseDown(int button, GameWaveModifiers m, bool continuous)
{
	INPUT Input[1];
    ZeroMemory(Input, sizeof(INPUT) * 1);

    if (button >= 0 && button < 3)
    {
        if (continuous)
        {
            mouseDownPosition_[button][0] = -1;
            mouseDownPosition_[button][1] = -1;
        }
        else
        {
			POINT pt;
			GetCursorPos(&pt);
            mouseDownPosition_[button][0] = pt.x;
            mouseDownPosition_[button][1] = pt.y;
        }
    }
	
	if (0 == button && !mouseButton1Down_)
    {
	    impl_->modifierEvent(m, true);
		Input[0].type = INPUT_MOUSE;
		Input[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
		SendInput(1, Input, sizeof(INPUT));
        mouseButton1Down_ = true;
    }
    if (1 == button && !mouseButton2Down_)
    {
	    impl_->modifierEvent(m, true);
		Input[0].type = INPUT_MOUSE;
		Input[0].mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
		SendInput(1, Input, sizeof(INPUT));
        mouseButton2Down_ = true;
    }
    if (2 == button && !mouseButton3Down_)
    {
	    impl_->modifierEvent(m, true);
		Input[0].type = INPUT_MOUSE;
		Input[0].mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;
		SendInput(1, Input, sizeof(INPUT));
        mouseButton3Down_ = true;
    }
}

void NativeSystemEvents::mouseUp(int button, GameWaveModifiers m)
{
	INPUT Input[1];
    ZeroMemory(Input, sizeof(INPUT) * 1);
	
    if (button >= 0 && button < 3)
    {
        mouseDownPosition_[button][0] = -1;
        mouseDownPosition_[button][1] = -1;
    }

    if (0 == button && mouseButton1Down_)
    {
		Input[0].type = INPUT_MOUSE;
		Input[0].mi.dwFlags = MOUSEEVENTF_LEFTUP;
		SendInput(1, Input, sizeof(INPUT));
	    impl_->modifierEvent(m, false);
        mouseButton1Down_ = false;
    }
    if (1 == button && mouseButton2Down_)
    {
		Input[0].type = INPUT_MOUSE;
		Input[0].mi.dwFlags = MOUSEEVENTF_RIGHTUP;
		SendInput(1, Input, sizeof(INPUT));
	    impl_->modifierEvent(m, false);
        mouseButton2Down_ = false;
    }
    if (2 == button && mouseButton3Down_)
    {
		Input[0].type = INPUT_MOUSE;
		Input[0].mi.dwFlags = MOUSEEVENTF_MIDDLEUP;
		SendInput(1, Input, sizeof(INPUT));
	    impl_->modifierEvent(m, false);
        mouseButton3Down_ = false;
    }
}

void NativeSystemEvents::mouseDoubleClick(GameWaveModifiers m)
{
	if (!mouseButton1Down_)
    {
	    impl_->modifierEvent(m, true);

		INPUT Input[4];
		ZeroMemory(Input, sizeof(INPUT) * 4);
		Input[0].type = INPUT_MOUSE;
		Input[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
		Input[1].type = INPUT_MOUSE;
		Input[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
		Input[2].type = INPUT_MOUSE;
		Input[2].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
		Input[3].type = INPUT_MOUSE;
		Input[3].mi.dwFlags = MOUSEEVENTF_LEFTUP;
		SendInput(4, Input, sizeof(INPUT));

	    impl_->modifierEvent(m, false);
	}
}

void NativeSystemEvents::moveScrollWheel(float dx, float dy, GameWaveModifiers m)
{
    impl_->modifierEvent(m, true);

	if (dx)
	{
		INPUT Input[1];
		ZeroMemory(Input, sizeof(INPUT) * 1);
		Input[0].type = INPUT_MOUSE;
		Input[0].mi.mouseData = DWORD(dx*3);
		Input[0].mi.dwFlags = MOUSEEVENTF_WHEEL;
		SendInput(1, Input, sizeof(INPUT));
	}
	if (dy)
	{
		INPUT Input[1];
		ZeroMemory(Input, sizeof(INPUT) * 1);
		Input[0].type = INPUT_MOUSE;
		Input[0].mi.mouseData = DWORD(dy*4);
		Input[0].mi.dwFlags = MOUSEEVENTF_HWHEEL;
		SendInput(1, Input, sizeof(INPUT));
	}

    impl_->modifierEvent(m, false);
}

void NativeSystemEvents::mediaDown(MediaKey m)
{
	if (m != MediaQuieter && m != MediaLouder && m != MediaMute)
	{
		impl_->determineMediaApp();
	}

	switch (m)
	{
		case MediaPlay:
		case MediaStop:
		case MediaPrevious:
		case MediaNext:
		case MediaMute:
			impl_->sendVkKeyEvent(mediaKeyToVKey(m), true);
			break;
		case MediaQuieter:
			appCommandDown(APPCOMMAND_VOLUME_DOWN);
			break;
		case MediaLouder:
			appCommandDown(APPCOMMAND_VOLUME_UP);
			break;
	}
}

void NativeSystemEvents::mediaUp(MediaKey)
{	
}

void NativeSystemEvents::appCommandDown(int command)
{
	MediaAppIdent ident = {UNKNOWN, 0, 0};
	impl_->sendAppCommand(command, ident);
}

void NativeSystemEvents::appCommandUp(int)
{
}

void NativeSystemEvents::systemCommand(SystemCommand command)
{
	impl_->systemCommand(command);
}

void NativeSystemEvents::resetOffset()
{
	POINT pt;
	GetCursorPos(&pt);
    juce::Rectangle<int> area = Desktop::getInstance().getDisplays().getDisplayContaining(Point<int>(pt.x, pt.y)).totalArea;
    offsetX_ = pt.x + area.getX() - area.getWidth()/2;
    offsetY_ = pt.y + area.getY() - area.getHeight()/2;
}

void NativeSystemEvents::clearOffset()
{
    offsetX_ = 0;
    offsetY_ = 0;
}

void NativeSystemEvents::prepareNotificationHandle(void *)
{
    // not required for Windows
}
