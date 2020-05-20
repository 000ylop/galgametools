// d3dslide-util.h, v1.1 2012/06/08
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

#ifndef D3DSLIDE_UTIL_H
#define D3DSLIDE_UTIL_H

#include <windows.h>
#include <ctime>
#include <climits>
#include <string>

namespace d3dslide {

  using std::wstring;
  using std::wstring;

  /*------------------------------------------------------------------------
   * rand_float
   *
   * Description:
   *   Computes a random float int [min, max].
   */
  float rand_float(float min = 0.0f, float max = 1.0f);

  /*------------------------------------------------------------------------
   * rand_int
   *
   * Description:
   *   Computes a random integer in [min, max].
   */
  int rand_int(int min = 0, int max = RAND_MAX);

  /*------------------------------------------------------------------------
   * wstring_tolower
   *
   * Description:
   *   Yes I know lower case doesn't make sense for all locale, but this
   *   should still be in std ... ;p
   */
  wstring wstring_tolower(const wstring& s);

  /***************************************************************************
   * duration_t
   *
   * Description:
   *   Encapsulates management of a duration of time.
   */
  class duration_t {
  public:
    /*------------------------------------------------------------------------
     * Constructor
     *
     * Description:
     *   Initializes the duration to done.
     */
    duration_t(void);

    /*------------------------------------------------------------------------
     * Constructor
     *
     * Description:
     *   Copies a duration.
     */
    duration_t(const duration_t &other);

    /*------------------------------------------------------------------------
     * Constructor
     *
     * Description:
     *   Initializes the duration to a value.
     */
    duration_t(int seconds);

    /*------------------------------------------------------------------------
     * Constructor
     *
     * Description:
     *   Initializes the duration to a value with a delay.
     */
    duration_t(int seconds, int delay_seconds);

    /*------------------------------------------------------------------------
     * Destrructor
     *
     * Description:
     *   Destroys this duration.
     */
    ~duration_t(void);

    /*------------------------------------------------------------------------
     * operator=
     *
     * Description:
     *   Assignment operator.
     */
    duration_t& operator=(const duration_t& rhs);

    /*------------------------------------------------------------------------
     * set
     *
     * Description:
     *   Sets the duration period and starts the timing.
     */
    void set(int seconds, int delay_seconds = 0);

    /*------------------------------------------------------------------------
     * subtract
     *
     * Description:
     *   Reduces the duration.
     */
    void subtract(int seconds);

    /*------------------------------------------------------------------------
     * get
     *
     * Description:
     *   Gets the original duration period.
     */
    int get(void) const;

    /*------------------------------------------------------------------------
     * done
     *
     * Description:
     *   Queries whether the duration is over.
     */
    bool done(void) const;

    /*------------------------------------------------------------------------
     * wait
     *
     * Description:
     *   Blocks at most seconds or the remaining duration.
     */
    void wait(int seconds = INT_MAX) const;

    /*------------------------------------------------------------------------
     * interrupt
     *
     * Description:
     *   Clears the duration and interrupts any waiter.
     */
    void interrupt(void);

    /*------------------------------------------------------------------------
     * progress
     *
     * Description:
     *   Gets the elapsed time of the duration as a percentage between 0 - 1.
     */
    float progress(void) const;

    /*------------------------------------------------------------------------
     * progress_frames
     *
     * Description:
     *   Gets the elapsed frames of the duration as a percentage between 0 - 1.
     */
    float progress_frames(void) const;

  private:
    /*------------------------------------------------------------------------
     * init
     *
     * Description:
     *   Initializes members.
     */
    void init(void);

    mutable HANDLE th;

    clock_t        time_duration;
    clock_t        time_start;
    clock_t        time_end;

    unsigned long  frame_duration;
    unsigned long  frame_start;
    unsigned long  frame_end;
  };

}

#endif /* D3DSLIDE_UTIL_H */
