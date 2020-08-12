// d3dslide-image_loader.cpp, v1.1 2012/06/08
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

#include "d3dslide-image_loader.h"
#include "d3dslide-config.h"
#include <algorithm>

namespace d3dslide {

  /*************************************************************************
   * image_loader_t
   *************************************************************************/

  static const bool           SCAN_SUBDIRECTORIES
    = config_t::get(L"image_loader.SCAN_SUBDIRECTORIES", true);

  static const bool           RANDOM_IMAGE_ORDER
    = config_t::get(L"image_loader.RANDOM_IMAGE_ORDER", true);

  static const bool           SET_THREAD_LOW_PRIORITY
    = config_t::get(L"image_loader.SET_THREAD_LOW_PRIORITY", false);

  static const unsigned long  MIN_IMAGE_WIDTH
    = config_t::get(L"image_loader.MIN_IMAGE_WIDTH", 0);

  static const unsigned long  MIN_IMAGE_HEIGHT
    = config_t::get(L"image_loader.MIN_IMAGE_HEIGHT", 0);

  /*------------------------------------------------------------------------
   * Constructor
   */
  image_loader_t::
  image_loader_t(d3d_control_t& d3d_control,
                 const wstring&  start_directory)
    : d3d_control(d3d_control),
      start_directory(start_directory)
  {
    InitializeCriticalSection(&lock);
    EnterCriticalSection(&lock);

    thread_handle = CreateThread(0,
                                 0,
                                 thread_entry_cb,
                                 this,
                                 CREATE_SUSPENDED,
                                 &tid);

    if (SET_THREAD_LOW_PRIORITY) {
      SetThreadPriority(thread_handle, THREAD_PRIORITY_IDLE);
    }

    ResumeThread(thread_handle);
  }

  /*------------------------------------------------------------------------
   * Destructor
   */
  image_loader_t::
  ~image_loader_t(void) {
    CloseHandle(thread_handle);
  }

  /*------------------------------------------------------------------------
   * get_next
   */
  d3d_image_t*
  image_loader_t::
  get_next(void) {
    image_info_t info;

    while (true) {
      if (curr_images.empty()) {
        EnterCriticalSection(&lock);
        curr_images = images;
        LeaveCriticalSection(&lock);
      }

      if (curr_images.size()) {        
        if (RANDOM_IMAGE_ORDER) {
          images_t::iterator i = curr_images.begin();
          advance(i, rand_int(0, curr_images.size() - 1));           

          info = *i;

          curr_images.erase(i);
        } else {
          info = curr_images.front();
          curr_images.pop_front();          
        }
      }
       
      if (!info.filename.empty() && select_check(info.filename)) {
        break;
      }
      
      Sleep(100);
    }

    return new d3d_image_t(d3d_control, info.filename);
  }

  /*------------------------------------------------------------------------
   * thread_entry_cb
   */
  DWORD WINAPI 
  image_loader_t::
  thread_entry_cb(void* pArg) {    
    image_loader_t* me = (image_loader_t*) pArg;
    me->run();
    return 0;
  }

  /*------------------------------------------------------------------------
   * run
   */
  void 
  image_loader_t::
  run(void) {
    LeaveCriticalSection(&lock);

    load_file_info(start_directory);

    if (images.size() == 0) {
      d3d_control.error_quit(start_directory + L": no images found");
    }

    // TODO: Add re-scanning of directory?

    Sleep(INFINITE);
  }

  /*------------------------------------------------------------------------
   * load_file_info
   */
  void 
  image_loader_t::
  load_file_info(const wstring& directory) {
    WIN32_FIND_DATA result;
    HANDLE          h;

    wstring path = directory + L"\\*";

    h = FindFirstFile(path.c_str(), &result);

    if (h == INVALID_HANDLE_VALUE) {
      return;
    }

    do {
      wstring fn_lower = wstring_tolower(result.cFileName);

      if (result.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY && 
          fn_lower != L"." && fn_lower != L"..")
      {
        if (SCAN_SUBDIRECTORIES) {
          load_file_info(directory + L"/" + result.cFileName);
        }
      } else {
        if (fn_lower.find(L".png") != wstring::npos ||
            fn_lower.find(L".jpg") != wstring::npos ||
            fn_lower.find(L".bmp") != wstring::npos ||
            fn_lower.find(L".tga") != wstring::npos)
        {        
          image_info_t info;
          info.filename  = directory + L"\\" + result.cFileName;
          info.d3d_image = NULL;

          EnterCriticalSection(&lock);
          images.push_back(info);
          LeaveCriticalSection(&lock);
        }
      }
    } while (FindNextFile(h, &result));
    
    FindClose(h);
  }

  /*------------------------------------------------------------------------
   * select_check
   *
   * D3DXGetImageInfoFromFile() is really slow!  We'll use this hack until
   * I implement a set of custom image loaders.  Try not to look at this
   * awful code. :)
   */
  bool 
  image_loader_t::
  select_check(const wstring& filename) {
    if (!MIN_IMAGE_WIDTH && !MIN_IMAGE_HEIGHT) {
      return true;
    }

    unsigned long width  = 0;
    unsigned long height = 0;

    HANDLE file = CreateFile(filename.c_str(),
                             GENERIC_READ,
                             FILE_SHARE_READ,
                             NULL,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL,
                             NULL);
    if (file == INVALID_HANDLE_VALUE) return false;

    // 1MB should always be enough for stupid JPEG ... right?
    DWORD len = min(1 * 1024 * 1024, GetFileSize(file, NULL));

    HANDLE map = CreateFileMapping(file, NULL, PAGE_READONLY, 0, 0, NULL); 
    if (map == NULL) return false;

    BYTE* data = (BYTE*) MapViewOfFile(map, FILE_MAP_READ, 0, 0, len);
    if (data == NULL) return false;

    BYTE* p   = data;
    BYTE* end = data + len;

    // JPEG/JFIF
    if (*(unsigned short*)p == 0xD8FF) {
      p += 2;

      while (p < end) {
        while (p < end && *p != 0xFF) p++;
        while (p < end && *p == 0xFF) p++;

        if (*p >= 0xC0 && *p <= 0xC3 && end - p >= 8) {
          width  = (p[4] << 8) | p[5];
          height = (p[6] << 8) | p[7];
          break;
        } else if (end - p >= 3) {
          unsigned short len = (p[1] << 8) | p[2];
          p += 1 + len;
        }
      }

    // PNG
    } else if (*(unsigned long*)p == 0x474E5089 && end - p >= 24) {
      width  = (p[16] << 24) | (p[17] << 16) | (p[18] << 8) | p[19];
      height = (p[20] << 24) | (p[21] << 16) | (p[22] << 8) | p[23];

    // Windows bitmap
    } else if (*(unsigned short*)p == 0x4D42 && end - p >= 26) {
      width  = *(unsigned long*) (p + 18);
      height = *(unsigned long*) (p + 22);
    }

    UnmapViewOfFile(data);
    CloseHandle(map);
    CloseHandle(file);

    return width && height &&
           (!MIN_IMAGE_WIDTH  || width  >= MIN_IMAGE_WIDTH) &&
           (!MIN_IMAGE_HEIGHT || height >= MIN_IMAGE_HEIGHT);
  }

}