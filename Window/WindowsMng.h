#pragma once
#include <memory>
#include <Windows.h>

#define WinMngIns IWindowsMng::GetInstance()

class IWindowsMng
{
public:
	static IWindowsMng& GetInstance(void);

	bool GenerateWindow(LPCWSTR classname, LPCTSTR title, long windowWidth, long windowHeight);
	HWND GetWindowHandle(LPCWSTR classname);
	bool Update(void);
private:
	IWindowsMng();
	~IWindowsMng();

	class WindowsMng;
	std::unique_ptr<WindowsMng> wndmng_;
};

bool CreateMyWindow(LPCWSTR classname, LPCTSTR title, long windowWidth, long windowHeight);
