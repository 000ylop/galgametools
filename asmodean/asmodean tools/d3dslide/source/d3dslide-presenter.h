// d3dslide-presenter.h, v1.1 2012/06/08
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

#ifndef D3DSLIDE_PRESENTER_H
#define D3DSLIDE_PRESENTER_H

#include "d3dslide-d3d_control.h"
#include "d3dslide-d3d_image.h"
#include "d3dslide-image_loader.h"
#include <vector>

namespace d3dslide {

  /***************************************************************************
   * presenter_t
   *
   * Description:
   *   Handles presentation of images including effect parameters.  This class
   *   creates a background thread which schedules presentation changes.
   */
  class presenter_t {
  public:
    enum presentation_method_t {
      PM_GLIDE_LINEAR,
      PM_GLIDE,
      PM_GLIDE_ZOOMIN,
      PM_GLIDE_ZOOMOUT,
      PM_GLIDE_ZOOMOUT_MAX,
      PM_ZOOMIN_FAST_GLIDE,
      PM_GLIDE_FAST_ZOOMOUT,
      PM_ZOOMIN,
      PM_ZOOMOUT,
      PM_MAX
    };

    enum presenter_action_t {
      PA_NORMAL,
      PA_HOLD,
      PA_NEXT,
      PA_PREV
    };

    /*------------------------------------------------------------------------
     * Constructor
     *
     * Description:
     *   Creates a new thread to execute the presenter in.
     */
    presenter_t(d3d_control_t&  d3d_control,
                image_loader_t& image_loader);

    /*------------------------------------------------------------------------
     * Destructor
     *
     * Description:
     *   Destroys thread.
     */
    ~presenter_t(void);

    /*------------------------------------------------------------------------
     * next
     *
     * Description:
     *   Cancels hold and immediately transitions to the next image.
     */
    void next(void);

    /*------------------------------------------------------------------------
     * previous
     *
     * Description:
     *   Cancels hold and immediately transitions to the previous image.
     *   TODO: implement me. :)
     */
    void previous(void);

    /*------------------------------------------------------------------------
     * hold
     *
     * Description:
     *   Continues presenting the current image until cancelled.
     */
    void hold(void);

    /*------------------------------------------------------------------------
     * present
     *
     * Description:
     *   Picks a random set of presentation parameters and generates an
     *   appropriate duration.
     */
    duration_t present(d3d_image_t* image,
                       unsigned int period_sec, 
                       unsigned int transition_sec,
                       bool         is_repeat);

  private:
    /*------------------------------------------------------------------------
     * thread_entry (callback)
     *
     * Description:
     *   Entrypoint for thread startup.
     */
    static DWORD WINAPI thread_entry_cb(void* pArg);

    /*------------------------------------------------------------------------
     * run
     *
     * Description:
     *   Loops forever loading images and displaying them.
     */
    void run(void);

    typedef std::vector<presentation_method_t> methods_t;

    d3d_control_t&     d3d_control;
    image_loader_t&    image_loader;
    HANDLE             thread_handle;
    DWORD              tid;
    methods_t          methods;
    duration_t         duration;
    presenter_action_t action;
  };

}

#endif /* D3DSLIDE_PRESENTER_H */
