// d3dslide-d3d_control.cpp, v1.1 2012/06/08
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

#include "d3dslide-d3d_control.h"
#include "d3dslide-config.h"

#include "d3dslide-d3d_image.h"

namespace d3dslide {

  /*************************************************************************
   * d3d_object_t
   *************************************************************************/

 /*------------------------------------------------------------------------
   * Constructor
   */
  d3d_object_t::
  d3d_object_t(void)
    : preloaded(false)
  {}

  /*------------------------------------------------------------------------
   * Destructor
   */
  d3d_object_t::
  ~d3d_object_t(void) {}

  /*------------------------------------------------------------------------
   * set_preloaded
   *
   * Description:
   *   Specifies whether the object resources are preloaded.
   */
  void 
  d3d_object_t::
  set_preloaded(bool preloaded) {
    this->preloaded = preloaded;
  }

  /*------------------------------------------------------------------------
   * is_preloaded
   */
  bool 
  d3d_object_t::
  is_preloaded(void) const {
    return preloaded;
  }

  /*------------------------------------------------------------------------
   * preload
   */
  void 
  d3d_object_t::
  preload(void) {
    set_preloaded(true);
  }

  /*************************************************************************
   * d3d_control_t
   *************************************************************************/

  static const bool SET_THREAD_HIGH_PRIORITY
    = config_t::get(L"d3d_control.SET_THREAD_HIGH_PRIORITY", true);

  static const bool FULL_SCREEN_DISPLAYMODE
    = config_t::get(L"d3d_control.FULL_SCREEN_DISPLAYMODE", false);

  static const bool USE_SOFTWARE_VERTEXPROCESSING
    = config_t::get(L"d3d_control.USE_SOFTWARE_VERTEXPROCESSING", false);

  unsigned long
  d3d_control_t::
  frame_rate = 0;

  LONG
  d3d_control_t::
  frame_number = 0;

  /*------------------------------------------------------------------------
   * Constructor
   */
  d3d_control_t::
  d3d_control_t(HWND wnd) 
    : wnd(wnd),
      d3d(NULL),
      device(NULL),
      screen_width(0),
      screen_height(0)
  {
    InitializeCriticalSection(&d3d_objects_lock);

    init_device();

    if (SET_THREAD_HIGH_PRIORITY) {
      SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    }
  }

  /*------------------------------------------------------------------------
   * Destructor
   */
  d3d_control_t::
  ~d3d_control_t(void) {
    DeleteCriticalSection(&d3d_objects_lock);

    if (device) {
      device->Release();
    }

    if (d3d) {
      d3d->Release();
    }
  }

  /*------------------------------------------------------------------------
   * error_quit
   */
  void 
  d3d_control_t::  
  error_quit(const wstring& msg) {
    ShowCursor(TRUE);

    SetWindowPos(wnd, HWND_TOP, 0, 0, screen_width, screen_height, SWP_HIDEWINDOW);

    MessageBox(wnd, 
               msg.c_str(), 
               L"d3dslide error",
		           MB_OK | MB_ICONEXCLAMATION);

    // This seems hacky, I should probably do something different.
    exit(0);
  }

  /*------------------------------------------------------------------------
   * get_window
   */
  HWND 
  d3d_control_t::
  get_window(void) {
    return wnd;
  }

  /*------------------------------------------------------------------------
   * get_device
   */
  LPDIRECT3DDEVICE9 
  d3d_control_t::
  get_device(void) {
    return device;
  }

  /*------------------------------------------------------------------------
   * get_screen_width
   */
  unsigned int 
  d3d_control_t::
  get_screen_width(void) const {
    return screen_width;
  }

  /*------------------------------------------------------------------------
   * get_screen_height
   */
  unsigned int 
  d3d_control_t::
  get_screen_height(void) const {
    return screen_height;
  }

  /*------------------------------------------------------------------------
   * add_object
   */
  void 
  d3d_control_t::
  add_object(d3d_object_t* object) {
    EnterCriticalSection(&d3d_objects_lock);
    d3d_objects.push_back(object);
    LeaveCriticalSection(&d3d_objects_lock);
  }

  /*------------------------------------------------------------------------
   * remove_object
   */
  void 
  d3d_control_t::
  remove_object(d3d_object_t* object) {
    EnterCriticalSection(&d3d_objects_lock);
    d3d_objects.remove(object);
    LeaveCriticalSection(&d3d_objects_lock);
  }

  /*------------------------------------------------------------------------
   * get_frame_rate
   */
  unsigned long 
  d3d_control_t::    
  get_frame_rate(void) {
    return frame_rate;
  }

  /*------------------------------------------------------------------------
   * get_frame_number
   */
  unsigned long 
  d3d_control_t::    
  get_frame_number(void) {
    return frame_number;
  }

  /*------------------------------------------------------------------------
   * render
   */
  void 
  d3d_control_t::
  render(void) {    
    EnterCriticalSection(&d3d_objects_lock);

    device->Clear(0, 
                  NULL, 
                  D3DCLEAR_TARGET,
                  D3DCOLOR_COLORVALUE(0.0f, 0.0f, 0.0f, 1.0f),
                  0.0f,
                  0);

    device->BeginScene();  
    
    for (d3d_objects_t::iterator i = d3d_objects.begin();
         i != d3d_objects.end();
         ++i)
    {
      if ((*i)->is_preloaded()) {
        (*i)->render();
      }
    }    

    device->EndScene();

    LeaveCriticalSection(&d3d_objects_lock);    

    device->Present(NULL, NULL, NULL, NULL);  

    InterlockedIncrement(&frame_number);

    EnterCriticalSection(&d3d_objects_lock);

    for (d3d_objects_t::iterator i = d3d_objects.begin();
         i != d3d_objects.end();
         ++i)
    {
      if (!(*i)->is_preloaded()) {
        (*i)->preload();
      }
    }    

    LeaveCriticalSection(&d3d_objects_lock);
  }

  /*------------------------------------------------------------------------
   * init_device
   */
  void 
  d3d_control_t::
  init_device(void) {
    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!d3d) error_quit(L"Direct3DCreate9() failed");

    D3DDISPLAYMODE d3ddm;
    d3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm);
    
    screen_width  = d3ddm.Width;
    screen_height = d3ddm.Height;
    frame_rate    = d3ddm.RefreshRate;

    //SetWindowPos(wnd, HWND_TOPMOST, 0, 0, screen_width, screen_height, SWP_SHOWWINDOW);
    SetWindowPos(wnd, HWND_NOTOPMOST, 0, 0, screen_width, screen_height, SWP_SHOWWINDOW);

    D3DPRESENT_PARAMETERS d3dpp;
    d3dpp.BackBufferWidth            = screen_width;
    d3dpp.BackBufferHeight           = screen_height;
    d3dpp.BackBufferFormat           = d3ddm.Format;
    d3dpp.BackBufferCount            = 1;
    d3dpp.MultiSampleType            = D3DMULTISAMPLE_NONE;
    d3dpp.MultiSampleQuality         = 0;
    d3dpp.SwapEffect                 = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow              = wnd;
    d3dpp.Windowed                   = FULL_SCREEN_DISPLAYMODE ? FALSE : TRUE;
    d3dpp.EnableAutoDepthStencil     = FALSE;
    d3dpp.AutoDepthStencilFormat     = D3DFMT_D16;
    d3dpp.Flags                      = 0;
    d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    d3dpp.PresentationInterval       = D3DPRESENT_INTERVAL_ONE;

    DWORD behavior_flags = D3DCREATE_MULTITHREADED;

    if (USE_SOFTWARE_VERTEXPROCESSING) {
      behavior_flags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    } else {
      behavior_flags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;      
    }

    d3d->CreateDevice(D3DADAPTER_DEFAULT, 
                      D3DDEVTYPE_HAL,
                      wnd,
                      behavior_flags,
                      &d3dpp,
                      &device);
    if (!device) error_quit(L"CreateDevice() failed");

    device->SetRenderState(D3DRS_ZENABLE, FALSE);
    device->SetRenderState(D3DRS_LIGHTING, FALSE);
    device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);


    device->SetRenderState(D3DRS_LOCALVIEWER, FALSE);
  }

}