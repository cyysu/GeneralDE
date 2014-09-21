#include <windows.h>
#include "RRenderWindowGLEE.h"
#include "m3eTypes.h"
#include "RGUIDesktop.h"
#include "RGUIImeManagerWin32.h"
#include "gdpp/app/Log.hpp"
#include "gdpp/app/Module.hpp"
#include "../EnvExt.hpp"
#include "../RuningExt.hpp"
#include "RRender.h"

int g_Device_Screen_W = 960;
int g_Device_Screen_H = 640;

void LoadDeviceScreenSize(const std::string& filename) {
	FILE* fp = fopen( filename.c_str(), "rt" );
	if ( fp ) {
		char buff[256];
		fgets(buff, 256, fp);
		sscanf(buff, "%d,%d", &g_Device_Screen_W, &g_Device_Screen_H);
		fclose(fp);
	}
}

int old_x;
int old_y;

LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	static bool s_interrupted = false;
	switch (message)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
    {
        RGUIImeManagerWin32* imeManager = (RGUIImeManagerWin32*)RGUIImeManager::GetIns();
        if (imeManager)
            imeManager->OnKeyDown(wParam, lParam);

        if ( wParam == VK_SHIFT )
        {
            //UI::App::EnvExt::instance(Gd::App::Application::instance())
            //    .runing()
            //    .processInput(M3E_Touch_MOUSEBEGAN ,2 , DEVICE_SCREEN_W - 10  , 480);
        }
    }
    break;
	case WM_KEYUP:
    {
        if ( wParam == VK_SHIFT )
        {
            //UI::App::EnvExt::instance(Gd::App::Application::instance())
            //    .runing()
            //    .processInput(M3E_Touch_MOUSEENDED ,2 , DEVICE_SCREEN_W - 10  , 480);
        }
    }
    break;
	case WM_CHAR:
    {
        RGUIImeManagerWin32* imeManager = (RGUIImeManagerWin32*)RGUIImeManager::GetIns();
        if (imeManager)
            imeManager->OnKeyChar(wParam, lParam);
    }
    break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		if(HIWORD(lParam) > 0)
		{
			SetCapture(hWnd);
			old_x = LOWORD(lParam);
			old_y = HIWORD(lParam);
            UI::App::EnvExt::instance(Gd::App::Application::instance())
                .runing()
                .processInput(M3E_Touch_MOUSEBEGAN,0 ,LOWORD(lParam)  , HIWORD(lParam), old_x, old_y);
		}
		break;

	case WM_MOUSEMOVE:
    {
		UI::App::EnvExt & env = UI::App::EnvExt::instance(Gd::App::Application::instance());

        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        char szBuf[64];
        sprintf(szBuf, "%s | %d %d", env.appName(), x, y);
        SetWindowText(hWnd, szBuf);
        if(GetCapture() == hWnd)
        {
            env.runing().processInput(M3E_Touch_MOUSEMOVED,0 ,LOWORD(lParam)  , HIWORD(lParam),old_x, old_y);
			old_x = LOWORD(lParam);
			old_y = HIWORD(lParam);
        }
        break;
    }

	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		if(GetCapture() == hWnd)
		{
			ReleaseCapture();
		}

        UI::App::EnvExt::instance(Gd::App::Application::instance())
            .runing()
            .processInput(M3E_Touch_MOUSEENDED ,0 ,LOWORD(lParam)  , HIWORD(lParam), old_x, old_y);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

int CALLBACK WinMain(__in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in_opt LPSTR lpCmdLine, __in int nShowCmd ) {
	//Load Screen Size
	LoadDeviceScreenSize( "Win32_Size.txt" );

	WNDCLASS wc;
	memset(&wc, 0, sizeof(WNDCLASS));

#if defined(_DEBUG)
 	//create console 
 	FILE *console; 
 	AllocConsole(); 
	freopen_s(&console, "CONOUT$", "w", stdout); 
#endif

	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wc.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	wc.hInstance		= hInstance;
	wc.lpfnWndProc		= WndProc;
	wc.lpszClassName	= "GameApp";
	wc.lpszMenuName		= NULL;
	wc.style			= CS_VREDRAW | CS_HREDRAW | CS_OWNDC;

	if (!RegisterClass(&wc))
		return 0;

	DWORD style = WS_SYSMENU | WS_BORDER | WS_CAPTION | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

	RECT clientSize;
	clientSize.top = 0;
	clientSize.left = 0;
	clientSize.right = g_Device_Screen_W;
	clientSize.bottom = g_Device_Screen_H;

	AdjustWindowRectEx(&clientSize, style, FALSE, 0);

	const int realWidth = clientSize.right - clientSize.left;
	const int realHeight = clientSize.bottom - clientSize.top;

	int windowLeft = (GetSystemMetrics(SM_CXSCREEN) - realWidth) / 2;
	int windowTop = (GetSystemMetrics(SM_CYSCREEN) - realHeight) / 2;
 
	HWND hWnd = CreateWindow("GameApp", "GameApp", style, windowLeft, windowTop, g_Device_Screen_W, g_Device_Screen_H, NULL, NULL, hInstance, NULL);
	if (hWnd == NULL)
		return 0;

	ShowWindow(hWnd, SW_NORMAL);
	UpdateWindow(hWnd);

	::GetClientRect(hWnd, &clientSize);
	int w  = g_Device_Screen_W - (clientSize.right-clientSize.left) + g_Device_Screen_W;
	int h = g_Device_Screen_H - (clientSize.bottom -clientSize.top) + g_Device_Screen_H;
	MoveWindow(hWnd, windowLeft, windowTop, w, h, FALSE);


	RRenderWindowGLEE* m_pWindow = new RRenderWindowGLEE(hWnd, true);
    m_pWindow->Current();

    char prog_name[] = "PlayerShareData";
    char * argv[] = { prog_name};

    gd_app_context_t app = gd_app_context_create_main(NULL, 0, CPE_ARRAY_SIZE(argv), argv);
    if (app == NULL) {
        assert(false);
        return 0;
    }

    gd_app_set_debug(app, 1);
    gd_app_ins_set(app);

    if (gd_app_cfg_reload(app) != 0) {
        APP_CTX_ERROR(app, "PlayerShareData: load cfg fail!");
        gd_app_context_free(app);
        assert(false);
        return -1;
    }

    if (gd_app_modules_load(app) != 0) {
        gd_app_context_free(app);
        APP_CTX_ERROR(app, "PlayerShareData: create app load module fail!");
        assert(false);
        return -1;
    }
    
    UI::App::EnvExt & env = UI::App::EnvExt::instance(Gd::App::Application::_cast(app));

	env.runing().init();
    env.runing().setSize(w, h);
    env.uiCenter().initPhase();

	MSG msg;
	memset(&msg, 0, sizeof(MSG));

	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			env.runing().update();
            env.runing().rend();

            if (env.runing().rendEnable()) {
				m_pWindow->Present();
            }
		}
	}

#if defined(_DEBUG)
 	FreeConsole();
 	fclose(console);
#endif

    gd_app_context_free(app);

	TerminateProcess(GetCurrentProcess(), EXIT_SUCCESS);

	return 0;
}

