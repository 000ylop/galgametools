// d3dslide-d3d_image.h, v1.1 2012/06/08
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

#ifndef D3DSLIDE_D3D_IMAGE_H
#define D3DSLIDE_D3D_IMAGE_H

#include "d3dslide-d3d_control.h"
#include "d3dslide-util.h"
#include <limits>

namespace d3dslide {

  /***************************************************************************
   * d3d_image_t
   *
   * Description:
   *   Implements a 2D object with an image file rendered on it.
   */
  class d3d_image_t : public d3d_object_t {
  public:
    /*------------------------------------------------------------------------
     * Constructor
     *
     * Description:
     *   Initializes the object from a file on disk.
     */
    d3d_image_t(d3d_control_t& control, const wstring& image_filename);

    /*------------------------------------------------------------------------
     * Destructor
     *
     * Description:
     *   Frees resources and removes this object from the d3d_control.
     */
    ~d3d_image_t(void);

    /*------------------------------------------------------------------------
     * set_position
     */
    void set_position(float x, float y);

    /*------------------------------------------------------------------------
     * set_scale
     */
    void set_scale(float new_scale);

    /*------------------------------------------------------------------------
     * set_alpha
     */
    void set_alpha(float new_alpha);

    /*------------------------------------------------------------------------
     * get_scale
     */
    float get_scale(void) const;

    /*------------------------------------------------------------------------
     * get_max_scale
     *
     * Description:
     *   Retrieves the scale which fits this image to the screen (max).
     */
    float get_max_scale(void) const;

    /*------------------------------------------------------------------------
     * get_aspect_diff
     *
     * Description:
     *   Retrieve the difference in aspect ratio between image and screen.
     */
    float get_aspect_diff(void) const;

    /*------------------------------------------------------------------------
     * get_pos_*
     */
    float get_pos_x(void) const;
    float get_pos_y(void) const;

    /*------------------------------------------------------------------------
     * get_*_dist
     *
     * Description:
     *   Gets the distance in pixels from the the edge of the image.
     */
    float get_left_dist(float x = std::numeric_limits<float>::quiet_NaN()) const;
    float get_top_dist(float y = std::numeric_limits<float>::quiet_NaN()) const;
    float get_right_dist(float x = std::numeric_limits<float>::quiet_NaN()) const;
    float get_bottom_dist(float y = std::numeric_limits<float>::quiet_NaN()) const;

    /*------------------------------------------------------------------------
     * get_*_remain
     *
     * Description:
     *   Gets the count of pixels remaining on each side relative to the
     *   specified position.
     */
    float get_left_remain(float x = std::numeric_limits<float>::quiet_NaN()) const;
    float get_top_remain(float y = std::numeric_limits<float>::quiet_NaN()) const;
    float get_right_remain(float x = std::numeric_limits<float>::quiet_NaN()) const;
    float get_bottom_remain(float y = std::numeric_limits<float>::quiet_NaN()) const;

    /*------------------------------------------------------------------------
     * is_fullscreen
     *
     * Description:
     *   Queries whether the image fills the screen (i.e., no black bars).
     */
    bool is_fullscreen(float x = std::numeric_limits<float>::quiet_NaN(),
                       float y = std::numeric_limits<float>::quiet_NaN()) const;

    /*------------------------------------------------------------------------
     * fit_min
     *
     * Description:
     *   Fits the image to the screen on the longest dimension (minimum zoom).
     */
    void fit_min(void);

    /*------------------------------------------------------------------------
     * fit_max
     *
     * Description:
     *   Fits the image to the screen on the shorest dimension (fill screen).
     */
    void fit_max(void);

    /*------------------------------------------------------------------------
     * set_pos_center
     *
     * Description:
     *   Moves the visible region to the center.
     */
    void set_pos_center(void);

    /*------------------------------------------------------------------------
     * set_pos_random
     *
     * Description:
     *   Moves the visible region to a random location.
     */
    void set_pos_random(void);

    /*------------------------------------------------------------------------
     * set_pos_random_corner
     *
     * Description:
     *   Moves the visible region to a random corner.
     */
    void set_pos_random_corner(void);

    /*------------------------------------------------------------------------
     * zoom
     *
     * Description:
     *   Change zoom by a percentage.
     */
    void zoom(float amount);

    /*------------------------------------------------------------------------
     * anim_reset
     *
     * Description:
     *   Resets all animation parameters (not animating).
     */
    void anim_reset(void);

    /*------------------------------------------------------------------------
     * anim_zoom
     *
     * Description:
     *   Starts an animated zoom.
     */
    void anim_zoom(float amount, duration_t duration);

    /*------------------------------------------------------------------------
     * anim_glide
     *
     * Description:
     *   Starts an animated glide towards the farthest edge.
     */
    void anim_glide(duration_t duration);

    /*------------------------------------------------------------------------
     * anim_fade
     *
     * Description:
     *   Starts an animated fade.
     */
    void anim_fade(float amount, duration_t duration);

    /*------------------------------------------------------------------------
     * preload (override from d3d_object_t)
     */
    void preload(void);

    /*------------------------------------------------------------------------
     * render (override from d3d_object_t)
     */
    void render(void);

  protected:
    /*------------------------------------------------------------------------
     * init_geometry
     */
    void init_geometry(void);

    /*------------------------------------------------------------------------
     * init_texture
     */
    void init_texture(void);

    /*------------------------------------------------------------------------
     * resample_texture
     */
    void resample_texture(void);

    /*------------------------------------------------------------------------
     * init_effects
     */
    void init_effects(void);

    /*------------------------------------------------------------------------
     * anim_update
     *
     * Description:
     *   Updates image parameters based on the animation states.
     */
    void anim_update(void);

  protected:
    // General
    d3d_control_t&     control;
    LPDIRECT3DDEVICE9  device;
    float              screen_width;
    float              screen_height;
    float              screen_aspect;

    // Texture
    wstring            image_filename;
    LPDIRECT3DTEXTURE9 texture;
    bool               texture_set;
    float              width;
    float              height;
    float              pos_x;
    float              pos_y;
    float              scale;
    float              scaled_width;
    float              scaled_height;
    float              aspect;    
    float              aspect_diff;
    float              alpha;

    // Geometry
    static const unsigned long VERTEX_FVF = D3DFVF_XYZ | D3DFVF_TEX1;

    struct vertex_t {
      D3DXVECTOR3 position;
      D3DXVECTOR2 texcoord;
    };
    
    LPDIRECT3DVERTEXBUFFER9 strip_vb;
    D3DXMATRIX              projection;  

    // Effects
    LPD3DXEFFECT effect;

    // Animation
    duration_t anim_zoom_duration;
    float      anim_zoom_start_scale;
    float      anim_zoom_amount;
    
    duration_t anim_glide_duration;
    float      anim_glide_start_x;
    float      anim_glide_start_y;
    bool       anim_glide_left;
    bool       anim_glide_top;

    bool       anim_fade_active;
    duration_t anim_fade_duration;
    float      anim_fade_amount;
    float      anim_fade_start_alpha;
  };

}

#endif /* D3DSLIDE_D3D_IMAGE_H */
