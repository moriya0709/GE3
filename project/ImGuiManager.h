#pragma once
#include <externals/imgui\imgui.h>
#include <externals/imgui/imgui_impl_dx12.h>
#include <externals/imgui/imgui_impl_win32.h>

class WindowAPI;
class DirectXCommon;
class SrvManager;

class ImGuiManager {
public:
	void Initialize(WindowAPI* windowAPI,DirectXCommon* dxCommon, SrvManager* srvManager);

	
};

