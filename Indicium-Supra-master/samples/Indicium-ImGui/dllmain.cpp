#include "dllmain.h"
#include "../../include/IndiciumPlugin.h"

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <psapi.h>

// DX
#include <d3d9.h>
#include <dxgi.h>
#include <d3d11.h>

// MinHook
#include <MinHook.h>

// STL
#include <mutex>
#include <map>

// POCO
#include <Poco/Message.h>
#include <Poco/Logger.h>
#include <Poco/FileChannel.h>
#include <Poco/AutoPtr.h>
#include <Poco/PatternFormatter.h>
#include <Poco/FormattingChannel.h>
#include <Poco/Path.h>

using Poco::Message;
using Poco::Logger;
using Poco::FileChannel;
using Poco::AutoPtr;
using Poco::PatternFormatter;
using Poco::FormattingChannel;
using Poco::Path;

// ImGui includes
#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_dx10.h>
#include <imgui_impl_dx11.h>

//my own includes
#include <iostream>     // std::cout
#include <sstream>      // std::stringstream

//CppWrapper includes
//#pragma comment(lib, "GGXrdWakeupDPUtilCppWrapper.lib")
//#include <C:\Work\rev2-wakeup-tool\GGXrdWakeupDPUtilCppWrapper\ReversalToolWrapper.h>

#pragma comment(lib, "GGRev2CrmDllCppWrapper.lib")
#include "C:\Work\GGRev2ComboRecipeManager\GGRev2CrmDllCppWrapper\ComboRecipeWrapper.h"
#include "C:\Work\GGRev2ComboRecipeManager\GGRev2CrmDllCppWrapper\ComboRecipeManagerWrapper.h"

typedef void functionPtr();
t_WindowProc OriginalWindowProc = nullptr;

static ID3D10Device*            g_pd3d10Device = nullptr;
static ID3D11Device*            g_pd3d11Device = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;

static std::once_flag d3d9Init;
static std::once_flag d3d9exInit;
static std::once_flag d3d10Init;
static std::once_flag d3d11Init;

static std::map<Direct3DVersion, bool> g_Initialized;

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID)
{
    DisableThreadLibraryCalls(static_cast<HMODULE>(hInstance));

    if (dwReason != DLL_PROCESS_ATTACH)
        return TRUE;

    std::string logfile("%TEMP%\\Indicium-ImGui.Plugin.log");

    AutoPtr<FileChannel> pFileChannel(new FileChannel);
    pFileChannel->setProperty("path", Poco::Path::expand(logfile));
    AutoPtr<PatternFormatter> pPF(new PatternFormatter);
    pPF->setProperty("pattern", "%Y-%m-%d %H:%M:%S.%i %s [%p]: %t");
    AutoPtr<FormattingChannel> pFC(new FormattingChannel(pPF, pFileChannel));

    Logger::root().setChannel(pFC);

    auto& logger = Logger::get("DLL_PROCESS_ATTACH");

    logger.information("Loading ImGui plugin");

    return CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(init), nullptr, 0, nullptr) > nullptr;
}

int init()
{
    auto& logger = Logger::get("init");

    logger.information("Initializing hook engine...");

    if (MH_Initialize() != MH_OK)
    {
        logger.fatal("Couldn't initialize hook engine");
        return -1;
    }

    logger.information("Hook engine initialized");

    return 0;
}

INDICIUM_EXPORT Present(IID guid, LPVOID unknown, Direct3DVersion version)
{
    static auto& logger = Logger::get(__func__);

    switch (version)
    {
    case Direct3DVersion::Direct3D9:

        std::call_once(d3d9Init, [&](LPVOID pUnknown)
        {
            auto pd3dDevice = static_cast<IDirect3DDevice9*>(unknown);

            D3DDEVICE_CREATION_PARAMETERS params;

            auto hr = pd3dDevice->GetCreationParameters(&params);
            if (FAILED(hr))
            {
                logger.error("Couldn't get creation parameters from device");
                return;
            }

            ImGui_ImplDX9_Init(params.hFocusWindow, pd3dDevice);

            logger.information("ImGui (DX9) initialized");

            HookWindowProc(params.hFocusWindow);

            g_Initialized[Direct3DVersion::Direct3D9] = true;

        }, unknown);

        if (g_Initialized[Direct3DVersion::Direct3D9])
        {
            ImGui_ImplDX9_NewFrame();
            RenderScene();
        }

        break;
    case Direct3DVersion::Direct3D9Ex:

        std::call_once(d3d9exInit, [&](LPVOID pUnknown)
        {
            auto pd3dDevice = static_cast<IDirect3DDevice9Ex*>(unknown);

            D3DDEVICE_CREATION_PARAMETERS params;

            auto hr = pd3dDevice->GetCreationParameters(&params);
            if (FAILED(hr))
            {
                logger.error("Couldn't get creation parameters from device");
                return;
            }

            ImGui_ImplDX9_Init(params.hFocusWindow, pd3dDevice);

            logger.information("ImGui (DX9Ex) initialized");

            HookWindowProc(params.hFocusWindow);

            g_Initialized[Direct3DVersion::Direct3D9Ex] = true;

        }, unknown);

        if (g_Initialized[Direct3DVersion::Direct3D9Ex])
        {
            ImGui_ImplDX9_NewFrame();
            RenderScene();
        }

        break;
    case Direct3DVersion::Direct3D10:

        std::call_once(d3d10Init, [&](LPVOID pChain)
        {
            logger.information("Grabbing device and context pointers");

            g_pSwapChain = static_cast<IDXGISwapChain*>(pChain);

            // get device
            auto hr = g_pSwapChain->GetDevice(__uuidof(g_pd3d10Device), reinterpret_cast<void**>(&g_pd3d10Device));
            if (FAILED(hr))
            {
                logger.error("Couldn't get device from swapchain");
                return;
            }

            DXGI_SWAP_CHAIN_DESC sd;
            g_pSwapChain->GetDesc(&sd);

            logger.information("Initializing ImGui");

            ImGui_ImplDX10_Init(sd.OutputWindow, g_pd3d10Device);

            logger.information("ImGui (DX10) initialized");

            HookWindowProc(sd.OutputWindow);

            g_Initialized[Direct3DVersion::Direct3D10] = true;

        }, unknown);

        if (g_Initialized[Direct3DVersion::Direct3D10])
        {
            ImGui_ImplDX10_NewFrame();
            RenderScene();
        }

        break;
    case Direct3DVersion::Direct3D11:

        std::call_once(d3d11Init, [&](LPVOID pChain)
        {
            logger.information("Grabbing device and context pointers");

            g_pSwapChain = static_cast<IDXGISwapChain*>(pChain);

            // get device
            auto hr = g_pSwapChain->GetDevice(__uuidof(g_pd3d11Device), reinterpret_cast<void**>(&g_pd3d11Device));
            if (FAILED(hr))
            {
                logger.error("Couldn't get device from swapchain");
                return;
            }

            // get device context
            g_pd3d11Device->GetImmediateContext(&g_pd3dDeviceContext);

            DXGI_SWAP_CHAIN_DESC sd;
            g_pSwapChain->GetDesc(&sd);

            logger.information("Initializing ImGui");

            ImGui_ImplDX11_Init(sd.OutputWindow, g_pd3d11Device, g_pd3dDeviceContext);

            logger.information("ImGui (DX11) initialized");

            HookWindowProc(sd.OutputWindow);

            g_Initialized[Direct3DVersion::Direct3D11] = true;

        }, unknown);

        if (g_Initialized[Direct3DVersion::Direct3D11])
        {
            ImGui_ImplDX11_NewFrame();
            RenderScene();
        }

        break;

    default:
        break;
    }
}

INDICIUM_EXPORT Reset(IID guid, LPVOID unknown, Direct3DVersion version)
{
    switch (version)
    {
    case Direct3DVersion::Direct3D9:
        ImGui_ImplDX9_InvalidateDeviceObjects();
        ImGui_ImplDX9_CreateDeviceObjects();
        break;
    }
}

DWORD pid;
HWND *my_hWnd;
HANDLE phandle;
void HookWindowProc(HWND hWnd)
{
    auto& logger = Logger::get(__func__);

    auto lptrWndProc = reinterpret_cast<t_WindowProc>(GetWindowLongPtr(hWnd, GWLP_WNDPROC));

    if (MH_CreateHook(lptrWndProc, &DetourWindowProc, reinterpret_cast<LPVOID*>(&OriginalWindowProc)) != MH_OK)
    {
        logger.error("Coudln't create hook for WNDPROC");
        return;
    }

    if (MH_EnableHook(lptrWndProc) != MH_OK)
    {
        logger.error("Couldn't enable DefWindowProc hooks");
        return;
    }

    logger.information("WindowProc hooked");

	GetWindowThreadProcessId(hWnd, &pid);

	phandle = OpenProcess(PROCESS_VM_READ, 0, pid);
	my_hWnd = &hWnd;
}

LRESULT WINAPI DetourWindowProc(
    _In_ HWND hWnd,
    _In_ UINT Msg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
)
{
    static std::once_flag flag;
    std::call_once(flag, []() {Logger::get("DetourWindowProc").information("++ DetourWindowProc called"); });

    ImGui_ImplDX9_WndProcHandler(hWnd, Msg, wParam, lParam);
    ImGui_ImplDX10_WndProcHandler(hWnd, Msg, wParam, lParam);
    ImGui_ImplDX11_WndProcHandler(hWnd, Msg, wParam, lParam);

    return OriginalWindowProc(hWnd, Msg, wParam, lParam);
}

int ReadMemoryInt(HANDLE processHandle, LPCVOID address) {
	int buffer = 0;
	SIZE_T NumberOfBytesToRead = sizeof(buffer); //this is equal to 4
	SIZE_T NumberOfBytesActuallyRead;
	BOOL err = ReadProcessMemory(processHandle, address, &buffer, NumberOfBytesToRead, &NumberOfBytesActuallyRead);
	if (err || NumberOfBytesActuallyRead != NumberOfBytesToRead)
		/*an error occured*/;
	return buffer;
}


HMODULE GetBaseAddressByName(DWORD processId, TCHAR *processName)
{
	TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
		PROCESS_VM_READ,
		FALSE, processId);

	if (NULL != hProcess)
	{
		HMODULE hMod;
		DWORD cbNeeded;

		if (EnumProcessModulesEx(hProcess, &hMod, sizeof(hMod),
			&cbNeeded, LIST_MODULES_32BIT | LIST_MODULES_64BIT))
		{
			GetModuleBaseName(hProcess, hMod, szProcessName,
				sizeof(szProcessName) / sizeof(TCHAR));
			if (!_tcsicmp(processName, szProcessName)) {
				//_tprintf(TEXT("0x%p\n"), hMod);
				return hMod;
			}
		}
	}

	CloseHandle(hProcess);
}

/*
#define Naked __declspec(naked)
Naked void changeAsm()
{
	_asm
	{
		test [edx],[edx]
	}
}
*/

HMODULE my_hMod = NULL;
DWORD my_ptr = 0;

std::string concatenateStrInt(std::string str, int i)
{
	std::ostringstream ss;
	ss << i;
	return str.append(ss.str());
}

void RenderScene()
{
    static std::once_flag flag;
    std::call_once(flag, []() {Logger::get("RenderScene").information("++ RenderScene called"); });

    static bool show_overlay = false;
    static bool show_test_window = true;
    static bool show_another_window = false;
    static ImVec4 clear_col = ImColor(114, 144, 154);


    // 1. Show a simple window
    // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
    {
        /*
		if (ImGui::Button("GGXrdDummyTest"))
		{
			my_hMod = GetModuleHandle(0);
			my_ptr = (DWORD)my_hMod + 0xb825e6;
		}

		if (ImGui::Button("asm")) {
			int(__stdcall* originalAsm)(int) = (int(__stdcall*)(int))(my_ptr);
			DetourFunction((PVOID*)(&originalAsm), (PVOID)changeAsm);
		}

		if (ImGui::Button("NUKE")) {
			functionPtr *func;
			func = (functionPtr*)my_ptr;
			func();
		}
		*/

		if (ImGui::Button("NUKE")) {
			//ReversalToolWrapper* reversalTool = new ReversalToolWrapper();
			//reversalTool->AttachToProcess();

			ComboRecipeManagerWrapper* c = new ComboRecipeManagerWrapper();
			c->ReadComboRecipes();
		}
		
		int test2 = ReadMemoryInt(phandle, (LPCVOID)0xb825e6);
		ImGui::Text("(LPCVOID)0xb825e6: %i", test2);
		ImGui::Text("base address: 0x%p", my_hMod);
		ImGui::Text("base address+offset: 0x%p", my_ptr);
    }

	char* charCombo = "Unknown";
	ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Combo manager");
	for (int i = 0; i < 5; i++)
	{
		ImGui::Text("Slot %i", i + 1);
		ImGui::SameLine();
		std::string strImport = concatenateStrInt("Import###", i);
		const char* labelImport = strImport.c_str();
		ImGui::Button(labelImport);
		ImGui::SameLine();
		std::string strExport = concatenateStrInt("Export###", i);
		const char* labelExport = strExport.c_str();
		ImGui::Button(labelExport);
		ImGui::SameLine();
		ImGui::Text(charCombo);
	}
	ImGui::End();

    // 2. Show another simple window, this time using an explicit Begin/End pair
    if (show_another_window)
    {
        ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
        ImGui::Begin("Another Window", &show_another_window);
        ImGui::Text("Hello");
        ImGui::End();
    }

    // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
    if (show_test_window)
    {
        ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
        ImGui::ShowTestWindow(&show_test_window);
    }

    static auto pressedPast = false, pressedNow = false;
    if (GetAsyncKeyState(VK_F3) & 0x8000)
    {
        pressedNow = true;
    }
    else
    {
        pressedPast = false;
        pressedNow = false;
    }

    if (!pressedPast && pressedNow)
    {
        show_overlay = !show_overlay;

        pressedPast = true;
    }

    if (show_overlay)
    {
        ImGui::Render();
    }
}
