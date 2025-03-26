#pragma once
#include <Windows.h>

//检查错误 返回值
void check_err(bool state,const char* str = nullptr) {
	if (state) {
		return;
	}
	char buffer[1024 * 2];
	wsprintf(reinterpret_cast<LPWSTR>(buffer), L"发生错误:%s", str);
	MessageBox(nullptr, reinterpret_cast<LPCWSTR>(buffer), nullptr, MB_OK | MB_ICONHAND);
	exit(-1);
}
