#pragma once
#include <Windows.h>
#include "help_func.hpp"
#include "inline_hook.hpp"

#include <d3d9.h>
#pragma comment(lib,"d3d9.lib")
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"


IDirect3D9* g_direct3d9 = nullptr;
IDirect3DDevice9* g_direct3dDevice = nullptr;

D3DPRESENT_PARAMETERS g_present;


inline_hook* g_Reset_hook = nullptr;

inline_hook* g_EndScene_hook = nullptr;
inline_hook* g_DrawIndexedPrimitive = nullptr;


WNDPROC g_original_proc = nullptr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK self_proc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, Msg, wParam, lParam))
		return true;

	CallWindowProc(g_original_proc, hWnd, Msg, wParam, lParam);
}

void init_imgui(IDirect3DDevice9* direct3dDevice) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsLight();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.IniFilename = nullptr;
	io.LogFilename = nullptr;
	ImGui_ImplWin32_Init(FindWindowA("your exe className", nullptr));
	ImGui_ImplDX9_Init(direct3dDevice);
}

//劫持这个函数当窗口变化执行这里面的
HRESULT __stdcall self_Reset(IDirect3DDevice9* direct3dDevice, D3DPRESENT_PARAMETERS* pPresentationParameters) {

	g_Reset_hook->restory_address();

	//------------------
	ImGui_ImplDX9_InvalidateDeviceObjects();
	HRESULT result = direct3dDevice->Reset(pPresentationParameters);
	ImGui_ImplDX9_CreateDeviceObjects();
	//------------------

	g_Reset_hook->motify_address();
	return result;
}
//劫持这个函数绘制方框在这里面
HRESULT __stdcall self_EndScene(IDirect3DDevice9* direct3dDevice) {

	static bool first_call = true;
	if (first_call) {
		first_call = false;
		init_imgui(direct3dDevice);
		g_original_proc = (WNDPROC)SetWindowLongA(FindWindowA("your exe className", nullptr), GWL_WNDPROC, (LONG)self_proc);
	}
	g_EndScene_hook->restory_address();
	//这里写绘制菜单的逻辑

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("start");
	ImGui::Text("My test");
	ImGui::End();
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());



	HRESULT result = direct3dDevice->EndScene();
	g_EndScene_hook->motify_address();
	return result;
}
//劫持这个函数直接实现纯透
HRESULT __stdcall self_DrawIndexedPrimitive(IDirect3DDevice9* direct3dDevice, D3DPRIMITIVETYPE type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
	return 1;
}

//初始化D3D9
//全部都是复制的 忘记就去例子里面复制
unsigned int __stdcall initialze_d3d9(void* data) {
#ifdef _DEBUG
	//AllocConsole();
	//SetConsoleTitleA("test");
	//freopen("CON", "w", stdout);
#endif //DEBUG


	g_direct3d9 = Direct3DCreate9(D3D_SDK_VERSION);
	check_err(g_direct3d9, "Direct3DCreate9调用失败！");

	memset(&g_present, 0, sizeof(g_present));
	g_present.Windowed = TRUE;
	g_present.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_present.BackBufferFormat = D3DFMT_UNKNOWN;

	HRESULT result = g_direct3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, FindWindowA("your exe className", nullptr),
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&g_present, &g_direct3dDevice);
	check_err(result == 0, "CreateDevice失败！");

	//读取虚表固定写法就得这么写 
	int* direct3d9_table = (int*)*(int*)g_direct3d9;
	int* direct3dDevice_table = (int*)*(int*)g_direct3dDevice;

	g_Reset_hook = new inline_hook(direct3dDevice_table[16], (int)self_Reset);
	g_EndScene_hook = new inline_hook(direct3dDevice_table[42], (int)self_EndScene);
	g_Reset_hook->motify_address();
	g_EndScene_hook->motify_address();

	return 0;
}


void un_load() {
	g_Reset_hook->restory_address();
	g_EndScene_hook->restory_address();
}