// d3dslide-util.cpp, v1.1 2012/06/08
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

#include "d3dslide-util.h"
#include "d3dslide-d3d_control.h"
#include <windows.h>
#include <algorithm>
#include <cctype>
#include <cstdlib>

namespace d3dslide {

  /*------------------------------------------------------------------------
   * rand_float
   */
  float rand_float(float min, float max) {
    float r = float(rand()) / float(RAND_MAX);

    return min + r * (max - min);
  }

  /*------------------------------------------------------------------------
   * rand_int
   */
  int rand_int(int min, int max) {
    // Doing this right is a lot harder than you'd think at first. :)
    return (int(rand_float() * float(max - min + 1)) % (max - min + 1)) + min;
  }

  /*------------------------------------------------------------------------
   * wstring_tolower
   */
  wstring wstring_tolower(const wstring& s) {
    // Probably faster than appending to an empty wstring
    wstring lower = s;
    std::transform(lower.begin(), lower.end(), lower.begin(), std::tolower);

    return lower;
  }

  /*************************************************************************
   * duration_t
   *************************************************************************/

  /*------------------------------------------------------------------------
   * Constructor
   */
  duration_t::
  duration_t(void) {
    init();
  }

  /*------------------------------------------------------------------------
   * Constructor
   */
  duration_t::
  duration_t(const duration_t& other) {
    operator=(other);
  }

  /*------------------------------------------------------------------------
   * Constructor
   */
  duration_t::
  duration_t(int seconds) {
    init();

    set(seconds, 0);
  }

  /*------------------------------------------------------------------------
   * Constructor
   */
  duration_t::
  duration_t(int seconds, int delay_seconds) {
    init();

    set(seconds, delay_seconds);
  }

  /*------------------------------------------------------------------------
   * Destructor
   */
  duration_t::
  ~duration_t(void) {
    if (th) {
      CloseHandle(th);
    }
  }

  /*------------------------------------------------------------------------
   * operator=
   */
  duration_t& 
  duration_t::
  operator=(const duration_t& rhs) {
    // This function exists to avoid copying the timer handle
    th             = NULL;

    time_duration  = rhs.time_duration;
    time_start     = rhs.time_start;
    time_end       = rhs.time_end;

    frame_duration = rhs.frame_duration;
    frame_start    = rhs.frame_start;
    frame_end      = rhs.frame_end;

    return *this;
  }

  /*------------------------------------------------------------------------
   * set
   */
  void 
  duration_t::
  set(int seconds, int delay_seconds) {
    time_duration  = CLOCKS_PER_SEC * (seconds - delay_seconds);
    time_start     = clock() + delay_seconds * CLOCKS_PER_SEC;
    time_end       = time_start + time_duration;

    static const unsigned long fps = d3d_control_t::get_frame_rate();

    frame_duration = fps * seconds;
    frame_start    = d3d_control_t::get_frame_number() + fps * delay_seconds;
    frame_end      = frame_start + frame_duration;
  }

  /*------------------------------------------------------------------------
   * subtract
   *
   * Description:
   *   Reduces the duration.
   */
  void 
  duration_t::
  subtract(int seconds) {
    set(get() - seconds);
  }

  /*------------------------------------------------------------------------
   * get
   */
  int 
  duration_t::
  get(void) const {
    return time_duration / CLOCKS_PER_SEC;
  }

  /*------------------------------------------------------------------------
   * done
   */
  bool 
  duration_t::
  done(void) const {
    return clock() > time_end;
  }

  /*------------------------------------------------------------------------
   * wait
   */
  void 
  duration_t::
  wait(int seconds) const {    
    clock_t now    = clock();
    clock_t remain = time_end - now;
    
    if (seconds != INT_MAX) {
      remain = min(clock_t(seconds * CLOCKS_PER_SEC), remain);
    }

    if (remain > 0) {
      LARGE_INTEGER due_time = { 0 };
      due_time.QuadPart = remain * -10000LL;

      if (!th) {
        th = CreateWaitableTimer(NULL, TRUE, NULL);
      }

      SetWaitableTimer(th, &due_time, 0, NULL, NULL, FALSE);
      WaitForSingleObject(th, INFINITE);
    }
  }

  /*------------------------------------------------------------------------
   * interrupt
   */
  void 
  duration_t::  
  interrupt(void) {
    set(0);
    
    LARGE_INTEGER due_time = { 0 };
    due_time.QuadPart = -1LL;
    SetWaitableTimer(th, &due_time, 0, NULL, NULL, FALSE);
  }

  /*------------------------------------------------------------------------
   * progress
   */
  float 
  duration_t::
  progress(void) const {
    clock_t now = clock();

    if (now < time_start) {
      return 0.0f;
    } else if (now > time_end) {
      return 1.0f;
    } else {
      return 1.0f - float(time_end - now) / float(time_duration);
    }
  }

  /*------------------------------------------------------------------------
   * progress_frames
   */
  float 
  duration_t::
  progress_frames(void) const {
    unsigned long now = d3d_control_t::get_frame_number();

    if (now < frame_start) {
      return 0.0f;
    } else if (now > frame_end) {
      return 1.0f;
    } else {
      return 1.0f - float(frame_end - now) / float(frame_duration);
    }
  }

  /*------------------------------------------------------------------------
   * init
   */
  void 
  duration_t::  
  init(void) {
    th             = NULL;

    time_duration  = 0;
    time_start     = 0;
    time_end       = 0;

    frame_duration = 0;
    frame_start    = 0;
    frame_end      = 0;
  }

}
