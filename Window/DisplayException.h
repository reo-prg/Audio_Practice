#pragma once
#include <Windows.h>

class DisplayException
{
public:
	static void DisplayError(LPCWSTR message)
	{
		MessageBox(NULL, message, L"TurnZ Application Exception", MB_OK | MB_ICONERROR);
		DisplayException::terminate_ = true;
	}
	static bool terminate_;
};
