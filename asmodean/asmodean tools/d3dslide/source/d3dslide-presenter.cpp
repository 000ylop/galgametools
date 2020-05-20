// d3dslide-presenter.cpp, v1.11 2012/06/26
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

#include "d3dslide-presenter.h"
#include "d3dslide-config.h"
#include "d3dslide-image_loader.h"
#include "d3dslide-util.h"
#include <algorithm>

namespace d3dslide {

  /*************************************************************************
   * presenter_t
   *************************************************************************/

  static const bool SET_THREAD_LOW_PRIORITY
    = config_t::get(L"presenter.SET_THREAD_LOW_PRIORITY", false);

  static const unsigned long BASE_PRESENTATION_SEC
    = config_t::get(L"presenter.BASE_PRESENTATION_SEC", 30);

  static const unsigned long TRANSITION_SEC   
    = config_t::get(L"presenter.TRANSITION_SEC", 3);

  static const unsigned long REPEAT_COUNT
    = config_t::get(L"presenter.REPEAT_COUNT", 1);

  static const unsigned long METHOD_WEIGHTS[presenter_t::PM_MAX] = {
    config_t::get(L"presenter.METHOD_WEIGHT_GLIDE_LINEAR", 0),
    config_t::get(L"presenter.METHOD_WEIGHT_GLIDE", 0),
    config_t::get(L"presenter.METHOD_WEIGHT_GLIDE_ZOOMIN", 0),
    config_t::get(L"presenter.METHOD_WEIGHT_GLIDE_ZOOMOUT", 0),
    config_t::get(L"presenter.METHOD_WEIGHT_GLIDE_ZOOMOUT_MAX", 4),
    config_t::get(L"presenter.METHOD_WEIGHT_ZOOMIN_FAST_GLIDE", 0),
    config_t::get(L"presenter.METHOD_WEIGHT_GLIDE_FAST_ZOOMOUT", 0),
    config_t::get(L"presenter.METHOD_WEIGHT_ZOOMIN", 0),
    config_t::get(L"presenter.METHOD_WEIGHT_ZOOMOUT", 0)
  };

  // Limit how many times we'll try to randomly selected a better method
  static const unsigned long MAX_SELECT_RETRIES = 25;

  /*------------------------------------------------------------------------
   * Constructor
   */
  presenter_t::
  presenter_t(d3d_control_t&  d3d_control,
              image_loader_t& image_loader) 
    : d3d_control(d3d_control),
      image_loader(image_loader),
      action(PA_NORMAL)
  {
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
  presenter_t::
  ~presenter_t(void) {
    CloseHandle(thread_handle);
  }

  /*------------------------------------------------------------------------
   * next
   */
  void 
  presenter_t::
  next(void) {
    // TODO: signal the duration
    action = PA_NEXT;
    duration.interrupt();
  }

  /*------------------------------------------------------------------------
   * previous
   */
  void 
  presenter_t::
  previous(void) {
    action = PA_PREV;
    duration.interrupt();
  }

  /*------------------------------------------------------------------------
   * hold
   */
  void
  presenter_t::
  hold(void) {
    if (action == PA_HOLD) {
      action = PA_NORMAL;
    } else {
      action = PA_HOLD;
    }
  }

  /*------------------------------------------------------------------------
   * thread_entry_cb
   */
  DWORD WINAPI 
  presenter_t::
  thread_entry_cb(void* pArg) {
    presenter_t* me = (presenter_t*) pArg;
    me->run();
    return 0;
  }

  /*------------------------------------------------------------------------
   * run
   */
  void 
  presenter_t::
  run(void) {
    // The random state is per-thread
#ifdef _DEBUG
    srand(31337); 
#else
    srand((unsigned int)time(NULL));
#endif

    // Fill methods with method indexes with multiple entries weighting the
    // random selection.
    for (unsigned long i = 0; i < PM_MAX; i++) {
      for (unsigned long j = 0; j < METHOD_WEIGHTS[i]; j++) {
        methods.push_back(presentation_method_t(i));
      }
    }

    if (methods.size() == 0) {
      d3d_control.error_quit(L"No presentation methods -- bad config?");
    }

    std::random_shuffle(methods.begin(), methods.end());

    d3d_image_t* curr_image = NULL;
    d3d_image_t* cull_image = NULL;
    d3d_image_t* next_image = NULL;

    duration_t duration_nointerrupt;

    unsigned long repeat_count = 0;

    while (true) {
      if (curr_image) {
        duration_nointerrupt.set(TRANSITION_SEC);
        duration_nointerrupt.wait();
        d3d_control.remove_object(cull_image);     
      }

      if (!next_image) {
        next_image = image_loader.get_next();
      }

      duration.wait();

      switch (action) {
        case PA_NEXT:
          action       = PA_NORMAL;
          repeat_count = 0;
          break;

        case PA_PREV:
          action       = PA_NORMAL;
          repeat_count = 0;

          delete next_image;
          next_image   = cull_image;          
          cull_image   = NULL;
          break;

        case PA_HOLD:
          repeat_count = 1;
          break;
      }

      bool do_hold = action == PA_HOLD || repeat_count;
      
      if (next_image && !do_hold) {
        delete cull_image;
        cull_image = curr_image;
        curr_image = next_image;       
        next_image = NULL;

        repeat_count = REPEAT_COUNT;
      }

      bool is_repeat = repeat_count != REPEAT_COUNT || action == PA_HOLD;

      repeat_count--;

#if 0
      // Delay a bit to avoid a rapid bounce.
      if (is_repeat) {
        duration_nointerrupt.set(2);
        duration_nointerrupt.wait();
      }      
#endif

      duration = present(curr_image, BASE_PRESENTATION_SEC, TRANSITION_SEC, is_repeat);        
      duration.subtract(TRANSITION_SEC);
    }
  }

  /*------------------------------------------------------------------------
   * rand_zoom
   *
   * Generate a random zoom percentage in [min, max] biased by aspect
   * ratio difference.
   */
  static float rand_zoom(float min, float max, float aspect_diff, float max_scale_diff) {
    float sign = min < 0 ? -1.0f : 1.0f;

    min = abs(min);
    max = abs(max);

    float z = rand_float(min, max);

    // Images that are already off screen by a large amount don't look good
    // with a large zoom amount.  Try to avoid it.
    if (aspect_diff > 1.0f) {
      z = max(0.0f, z - 0.35f);
    }

    z *= sign;
    z += max_scale_diff - 1;

    return z;
  }

  /*------------------------------------------------------------------------
   * present
   */
  duration_t 
  presenter_t::    
  present(d3d_image_t* image,
          unsigned int period_sec, 
          unsigned int transition_sec,
          bool         is_repeat)
  {
    if (is_repeat) {
      transition_sec = 0;
      image->anim_reset();
    } else {
      image->fit_max(); 
    }

    period_sec = int(rand_float(0.5f, 1.0f) * period_sec);
    period_sec = max(period_sec, transition_sec);

    bool            selected    = false;
    unsigned long   retries     = MAX_SELECT_RETRIES;
    float           aspect_diff = image->get_aspect_diff();
    float           msd         = image->get_max_scale() / image->get_scale();    

    while (!selected) {
      presentation_method_t method = methods[rand_int(0, methods.size() - 1)];

      // These could probably all be implemented as a table of flags and
      // parameters ... or entirely config driven.  Maybe later.

      // Note that some of the methods try to avoid unnatural or boring
      // effects depending on the aspect ratio.

      bool picky = retries != 0 && !is_repeat;

      switch (method) {
      // Glide in a single direction across long images
      case PM_GLIDE_LINEAR:        
        if (!picky || aspect_diff > 0.75) {
          if (!is_repeat) image->set_pos_random_corner();
          image->anim_glide(period_sec);
          selected = true;
        }
        break;

      // Glide from a random corner
      case PM_GLIDE:
        {
          float zoom_p = rand_zoom(0.5, 0.75f, aspect_diff, msd);

          if (!is_repeat) image->zoom(zoom_p);
          if (!is_repeat) image->set_pos_random_corner();          
          image->anim_glide(period_sec);

          selected = true;
        }
        break;

      // Glide from a random corner while zooming in
      case PM_GLIDE_ZOOMIN:
        {
          float zoom_p = rand_zoom(0.5, 1.0, aspect_diff, msd);

          if (!is_repeat) image->set_pos_random_corner();
          image->anim_zoom(zoom_p, period_sec);
          image->anim_glide(period_sec);

          selected = true;
        }
        break;

      // Fast glide from a random corner then zoom out
      case PM_GLIDE_FAST_ZOOMOUT:
        if (!picky || aspect_diff <= 0.5) {
          float zoom_p       = rand_zoom(-0.5, -1.0, aspect_diff, msd);
          int   glide_period = int(period_sec * rand_float(0.25, 0.75));
          
          if (!is_repeat) image->zoom(-zoom_p);          
          if (!is_repeat) image->set_pos_random_corner();
          image->anim_zoom(zoom_p, duration_t(period_sec, glide_period));
          image->anim_glide(glide_period);

          selected = true;
        }
        break;

      // Fast zoom in then glide
      case PM_ZOOMIN_FAST_GLIDE:
        {
          float zoom_p      = rand_zoom(0.5, 0.75, aspect_diff, msd);
          int   zoom_period = int(period_sec * rand_float(0.25, 0.5));
          
          if (!is_repeat) image->zoom(zoom_p);          
          if (!is_repeat) image->set_pos_random_corner();          
          image->anim_zoom(zoom_p, zoom_period);
          image->anim_glide(period_sec);

          selected = true;
        }
        break;

      // Glide from a random corner while zooming out
      case PM_GLIDE_ZOOMOUT:
        {
          float zoom_p = rand_zoom(-0.5, -1.0, aspect_diff, msd);
          
          if (!is_repeat) image->zoom(-zoom_p);          
          if (!is_repeat) image->set_pos_random_corner();
          image->anim_zoom(zoom_p, period_sec);          
          image->anim_glide(period_sec);

          selected = true;
        }
        break;

      // Glide from a random corner while zooming all the way out (repeat only)
      case PM_GLIDE_ZOOMOUT_MAX:
        if (!picky) {          
          image->anim_zoom(msd - 1, period_sec);
          image->anim_glide(period_sec);

          selected = true;
        }
        break;

      // Zoom in on the center
      case PM_ZOOMIN:
        if (!picky || aspect_diff <= 0.5) {
          float zoom_p = rand_zoom(0.75, 1.0, 0, msd);

          if (!is_repeat) image->set_pos_center();
          image->anim_zoom(zoom_p, period_sec); 

          selected = true;
        }
        break;

      // Zoom out from the center
      case PM_ZOOMOUT:
        if (!picky || aspect_diff <= 0.5) {
          float zoom_p = rand_zoom(-0.75, -1.0, 0, msd);

          if (!is_repeat) image->zoom(-zoom_p);
          if (!is_repeat) image->set_pos_center();
          image->anim_zoom(zoom_p, period_sec); 

          selected = true;
        }
        break;
      }

      retries--;
    }

    if (transition_sec) {
      image->set_alpha(0.0f);
      image->anim_fade(1.0f, transition_sec);
    }

    if (!is_repeat) {
      d3d_control.add_object(image);
    }

    return duration_t(period_sec);
  }
}
