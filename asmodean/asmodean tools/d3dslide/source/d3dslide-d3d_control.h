// d3dslide-d3d_control.h, v1.1 2012/06/08
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

#ifndef D3DSLIDE_D3D_CONTROL_H
#define D3DSLIDE_D3D_CONTROL_H

#include <windows.h>
#include <mmsystem.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <list>
#include <string>

namespace d3dslide {

  using std::wstring;

  /***************************************************************************
   * d3d_object_t (abstract)
   *
   * Description:
   *   Interface for renderable objects to implement.
   */
  class d3d_object_t {
  public:    
    /*------------------------------------------------------------------------
     * Constructor
     */
    d3d_object_t(void);

    /*------------------------------------------------------------------------
     * Destructor
     */
    virtual ~d3d_object_t(void);

    /*------------------------------------------------------------------------
     * set_preloaded
     *
     * Description:
     *   Specifies whether the object resources are preloaded.
     */
    void set_preloaded(bool preloaded);

    /*------------------------------------------------------------------------
     * is_preloaded
     *
     * Description:
     *   Queries whether object resources are preloaded.
     */
    bool is_preloaded(void) const;

    /*------------------------------------------------------------------------
     * preload
     *
     * Description:
     *   Called to indicate that the object should preload resources.  Default
     *   implementation does nothing.
     */
    virtual void preload(void);

    /*------------------------------------------------------------------------
     * render (abstract)
     *
     * Description:
     *   Called to indicate that the object should be rendered.
     */
    virtual void render(void) = 0;

  private:
    bool preloaded;    
  };

  /***************************************************************************
   * d3d_control_t
   *
   * Description:
   *   Initializes Direct3D device and renders objects.
   */
  class d3d_control_t {
  public:
    /*------------------------------------------------------------------------
     * Constructor
     *
     * Description:
     *   Initializes using the supplied window as the target.
     */
    d3d_control_t(HWND wnd);

    /*------------------------------------------------------------------------
     * Destructor
     */
    ~d3d_control_t(void);

    /*------------------------------------------------------------------------
     * error_quit
     *
     * Description:
     *   Hides the render window, shows an error dialog box, and then exits
     *   the application.
     */
    void error_quit(const wstring& msg);

    /*------------------------------------------------------------------------
     * get_window
     */
    HWND get_window(void);

    /*------------------------------------------------------------------------
     * get_device
     */
    LPDIRECT3DDEVICE9 get_device(void);

    /*------------------------------------------------------------------------
     * get_screen_width
     */
    unsigned int get_screen_width(void) const;

    /*------------------------------------------------------------------------
     * get_screen_height
     */
    unsigned int get_screen_height(void) const;

    /*------------------------------------------------------------------------
     * add_object
     *
     * Description:
     *   Adds an object to the render list.
     */
    void add_object(d3d_object_t* object);

    /*------------------------------------------------------------------------
     * remove_object
     *
     * Description:
     *   Removes an object from the render list.
     */
    void remove_object(d3d_object_t* object);

    /*------------------------------------------------------------------------
     * render
     *
     * Description:
     *   Renders all objects on the render list.
     */
    void render(void);

    /*------------------------------------------------------------------------
     * get_frame_rate
     *
     * Description:
     *   Returns the frame rate (screen refresh rate).
     */
    static unsigned long get_frame_rate(void);

    /*------------------------------------------------------------------------
     * get_frame_number
     *
     * Description:
     *   Returns the number of rendered frames.
     */
    static unsigned long get_frame_number(void);

  private:
    /*------------------------------------------------------------------------
     * init_device
     *
     * Description:
     *   Initializes the Direct3D device.
     */
    void init_device(void);

    HWND              wnd;
    LPDIRECT3D9       d3d;
    LPDIRECT3DDEVICE9 device;
    unsigned int      screen_width;
    unsigned int      screen_height;

    static unsigned long frame_rate;
    static LONG          frame_number;

    typedef std::list<d3d_object_t*> d3d_objects_t;
    d3d_objects_t    d3d_objects;
    CRITICAL_SECTION d3d_objects_lock;
  };

}

#endif /* D3DSLIDE_D3D_CONTROL_H */
