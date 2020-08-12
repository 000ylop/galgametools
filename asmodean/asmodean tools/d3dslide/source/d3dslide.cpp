// d3dslide.cpp, v1.1 2012/06/08
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

#include "d3dslide-config.h"
#include "d3dslide-d3d_control.h"
#include "d3dslide-image_loader.h"
#include "d3dslide-presenter.h"
#include <shlobj.h>
#include <objbase.h>
#include <string>
#include <cstdlib>

using std::wstring;

static const wstring VERSION_STRING = L"d3dslide v1.11 by asmodean (http://asmodean.reverse.net)\n\n";

static const bool SET_PROCESS_HIGH_PRIORITY
  = d3dslide::config_t::get(L"d3dslide.PROCESS_HIGH_PRIORITY", true);

static const bool RENDER_ON_DESKTOP
  = d3dslide::config_t::get(L"d3dslide.RENDER_ON_DESKTOP", false);

// Presenter associated with the input focus
d3dslide::presenter_t* focus_presenter = NULL;

/*------------------------------------------------------------------------
 * get_folder_selection
 *
 * Description:
 *   Prompts the user to choose a folder and returns the path to it as a
 *   directory name.
 */
wstring get_folder_selection(HWND owner, const wstring& title) {
  TCHAR buff[MAX_PATH];
  memset(buff, 0, sizeof(buff));

	BROWSEINFO bi = { 0 };

	bi.hwndOwner      = owner;
	bi.pszDisplayName = buff;
	bi.pidlRoot       = NULL;
	bi.lpszTitle      = title.c_str();
	bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;

  LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

	if (pidl) {
		SHGetPathFromIDList(pidl, buff);
		CoTaskMemFree(pidl);
    return buff;
	}

	return L"";
}

/*------------------------------------------------------------------------
 * window_msg_cb (callback)
 *
 * Description:
 *   Processes window messages.
 */
LRESULT CALLBACK window_msg_cb(HWND   hWnd, 
                               UINT   msg, 
                               WPARAM wParam, 
                               LPARAM lParam)
{
  switch(msg) {
  case WM_LBUTTONDOWN:
    if (focus_presenter) focus_presenter->next();
    break;

  case WM_RBUTTONDOWN:
    if (focus_presenter) focus_presenter->previous();
    break;

  case WM_MBUTTONDOWN:
  case WM_CLOSE:
  case WM_DESTROY:
    PostQuitMessage(0);
    break;

  case WM_KEYDOWN:
    switch(wParam) {
    case VK_ESCAPE:
      PostQuitMessage(0);
      break;

    case VK_DOWN:
    case VK_RIGHT:
    case VK_NEXT:
      if (focus_presenter) focus_presenter->next();
      break;

    case VK_UP:
    case VK_LEFT:
    case VK_PRIOR:
      if (focus_presenter) focus_presenter->previous();
      break;

    case VK_SPACE:
      if (focus_presenter) focus_presenter->hold();
      break;
    }

  default:
    return DefWindowProc(hWnd, msg, wParam, lParam);
		break;
	}

	return 0;
}

static const wchar_t* D3DSLIDE_WINDOW_CLASS = L"D3DSLIDE_WINDOW_CLASS";

/*------------------------------------------------------------------------
 * WinMain
 *
 * Description:
 *   Initializes win32 basics and d3dslide objects then manages window
 *   message pump.
 */
int WINAPI wWinMain(HINSTANCE hInstance, 
                    HINSTANCE hPrevInstance, 
                    PWSTR     pCmdLine, 
                    int       nCmdShow)
{
#ifdef _DEBUG
  srand(31337); 
#else
  srand((unsigned int)time(NULL));
#endif

  wstring directory = pCmdLine;

	MSG msg;
  memset(&msg, 0, sizeof(msg));

  CoInitialize(NULL);

  if (SET_PROCESS_HIGH_PRIORITY) {
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
  }

	WNDCLASSEX win_class;     
	win_class.lpszClassName = D3DSLIDE_WINDOW_CLASS;
	win_class.cbSize        = sizeof(WNDCLASSEX);
	win_class.style         = CS_HREDRAW | CS_VREDRAW;
	win_class.lpfnWndProc   = window_msg_cb;
	win_class.hInstance     = hInstance;
	win_class.hIcon	        = LoadIcon(hInstance, L"d3dslide.ico");
  win_class.hIconSm	      = LoadIcon(hInstance, L"d3dslide.ico");
	win_class.hCursor       = LoadCursor(NULL, IDC_ARROW);
	win_class.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	win_class.lpszMenuName  = NULL;
	win_class.cbClsExtra    = 0;
	win_class.cbWndExtra    = 0;

  if (!RegisterClassEx(&win_class)) {
		return E_FAIL;
  }

  HWND parent        = NULL;
  int  parent_width  = 0;
  int  parent_height = 0;

  DWORD ws_flags = WS_VISIBLE | WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

  // XXX: This used to work, what happened? :P
  if (RENDER_ON_DESKTOP) {
    //HWND progman = FindWindow(L"Progman", NULL);
       
    while (parent = FindWindowEx(NULL, parent, L"WorkerW", L"")) {
      HWND temp = FindWindowEx(parent, 0, L"SHELLDLL_DefView", NULL);
      temp = FindWindowEx(temp, 0, L"SysListView32", NULL);

      if (temp) {
        ws_flags = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_DISABLED;

        parent = temp;

        WINDOWPLACEMENT wp = { sizeof(WINDOWPLACEMENT) };
	      GetWindowPlacement(parent, &wp);

        parent_width  = wp.rcNormalPosition.right  - wp.rcNormalPosition.left;
        parent_height = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
        break;
      }
    }

    if (!parent) {
      MessageBox(NULL, 
                 L"RENDER_ON_DESKTOP requested but failed to find desktop window.", 
                 L"d3dslide error",
		             MB_OK | MB_ICONEXCLAMATION);
      return 1;
    }
  }

	HWND wnd = CreateWindowEx(0, 
                            D3DSLIDE_WINDOW_CLASS, 
                            L"d3dslide window",
                            ws_flags,
					                  0, 
                            0, 
                            parent_width, 
                            parent_height,
                            parent,
                            NULL,
                            hInstance, 
                            NULL);

  if (wnd == NULL) {
		return E_FAIL;
  }

  if (directory.empty()) {
    directory = get_folder_selection(wnd, VERSION_STRING + L"Select Source Image Folder:");
  } else {
    if (directory[0] == L'\"') {
      directory = directory.substr(1, directory.length() - 2);
    }    
  }

  if (!directory.empty()) {
    if (!parent) {
      ShowCursor(FALSE);
    }

    d3dslide::d3d_control_t  d3d_control(wnd);
    d3dslide::image_loader_t image_loader(d3d_control, directory);
    d3dslide::presenter_t    presenter(d3d_control, image_loader);

    focus_presenter = &presenter;

	  while (msg.message != WM_QUIT) {
		  if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { 
        TranslateMessage(&msg);
			  DispatchMessage(&msg);
      } else {
        d3d_control.render();
      }
	  }
  }

  UnregisterClass(D3DSLIDE_WINDOW_CLASS, win_class.hInstance);

  CoUninitialize();

	return msg.wParam;
}
