#include "WindowsMng.h"
#include <unordered_map>
#include <iostream>
#include <string>
#include <cassert>
#include "DisplayException.h"

LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

class IWindowsMng::WindowsMng
{
public:
	WindowsMng() {};
	~WindowsMng() 
	{
		for (auto& w : wnd_)
		{
			UnregisterClass(w.first.c_str(), w.second.hInst_);
		}
	};

	struct wndDesc
	{
		HWND hwnd_;
		HINSTANCE hInst_;
	};
	
	std::unordered_map<std::wstring, wndDesc> wnd_;

	bool GenerateWindow(LPCWSTR classname, LPCTSTR title, long windowWidth, long windowHeight)
	{
		if (wnd_.find(classname) != wnd_.end())
		{
			OutputDebugString(L"WARNING : generate window multiply in WindowMng::GenerateWindow function");
			assert(false);
			return false;
		}

		HWND wHandle;

		WNDCLASSEX wclass = {};
		wclass.cbSize = sizeof(WNDCLASSEX);
		wclass.hInstance = GetModuleHandle(0);
		wclass.lpszClassName = classname;
		wclass.lpfnWndProc = (WNDPROC)WindowProc;

		RegisterClassEx(&wclass);

		RECT wsize = { 0, 0, windowWidth, windowHeight };
		AdjustWindowRect(&wsize, WS_OVERLAPPEDWINDOW, false);
		

		wHandle = CreateWindow(classname, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
			wsize.right, wsize.bottom, nullptr, nullptr, wclass.hInstance, nullptr);


		if (wHandle == nullptr)
		{
			LPVOID msgbuffer = nullptr;
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&msgbuffer, 0, nullptr);

			OutputDebugString((TCHAR*)msgbuffer);
			std::cout << (TCHAR*)msgbuffer << std::endl;
			LocalFree(msgbuffer);
			return false;
		}

		wnd_.emplace(classname, wndDesc{ wHandle, wclass.hInstance });
		return true;
	};

	HWND GetWindowHandle(LPCWSTR classname)
	{
		if (wnd_.find(classname) == wnd_.end())
		{
			OutputDebugString(classname);
			OutputDebugString(L"is not founded\n");
			GenerateWindow(classname, L"NotFound :(", 320, 240);
		}
		return wnd_.at(classname).hwnd_;
	}

	bool Update(void)
	{
		for (auto& w : wnd_)
		{
			if (UpdateWindow(w.second.hwnd_) == 0)
			{
				return false;
			}
		}

		MSG msg = {};
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT)
		{
			return false;
		}
		if (DisplayException::terminate_)
		{
			PostQuitMessage(0);
			return false;
		}

		return true;
	}
};

IWindowsMng::IWindowsMng():wndmng_(new WindowsMng())
{
	
}

IWindowsMng::~IWindowsMng() = default;

IWindowsMng& IWindowsMng::GetInstance(void)
{
	static IWindowsMng win;
	return win;
}

bool IWindowsMng::GenerateWindow(LPCWSTR classname, LPCTSTR title, long windowWidth, long windowHeight)
{
	return wndmng_->GenerateWindow(classname, title, windowWidth, windowHeight);
}

HWND IWindowsMng::GetWindowHandle(LPCWSTR classname)
{
	return wndmng_->GetWindowHandle(classname);
}

bool IWindowsMng::Update(void)
{
	return wndmng_->Update();
}

LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

bool CreateMyWindow(LPCWSTR classname, LPCTSTR title, long windowWidth, long windowHeight)
{
	if (!WinMngIns.GenerateWindow(classname, title, windowWidth, windowHeight))
	{
		return false;
	}

	ShowWindow(WinMngIns.GetWindowHandle(classname), SW_NORMAL);

	DisplayException::terminate_ = false;
	return true;
}
