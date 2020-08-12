// d3dslide-image_loader.h, v1.1 2012/06/08
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

#ifndef D3DSLIDE_IMAGE_LOADER_H
#define D3DSLIDE_IMAGE_LOADER_H

#include "d3dslide-d3d_control.h"
#include "d3dslide-d3d_image.h"
#include <list>

namespace d3dslide {

  /***************************************************************************
   * image_loader_t
   *
   * Description:
   *   Scans a directory for images in a background thread.
   */
  class image_loader_t {
  public:
    /*------------------------------------------------------------------------
     * Constructor
     *
     * Description:
     *   Creates a new thread and runs itself.
     */
    image_loader_t(d3d_control_t& d3d_control,
                   const wstring&  start_directory);

    /*------------------------------------------------------------------------
     * Destructor
     *
     * Description:
     *   Destroys the thread associated with this instance.
     */
    ~image_loader_t(void);

    /*------------------------------------------------------------------------
     * get_next
     *
     * Description:
     *   Gets the next image.  This currently causes the image to be
     *   decoded synchronously.
     */
    d3d_image_t* get_next(void);

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

    /*------------------------------------------------------------------------
     * load_file_info
     *
     * Description:
     *   Scans the directory for image files.
     */
    void load_file_info(const wstring& directory);

    /*------------------------------------------------------------------------
     * select_check
     *
     * Description:
     *   Checks whether the image should be selected.
     */
    bool select_check(const wstring& filename);

    CRITICAL_SECTION lock;
    d3d_control_t&   d3d_control;
    wstring          start_directory;
    HANDLE           thread_handle;
    DWORD            tid;

    struct image_info_t {
      wstring       filename;
      d3d_image_t* d3d_image;
    };

    typedef std::list<image_info_t> images_t;
    images_t images;
    images_t curr_images;
  };

}

#endif /* D3DSLIDE_IMAGE_LOADER_H */
