// d3dslide-d3d_image.cpp, v1.1 2012/06/08
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

#include "d3dslide-d3d_image.h"
#include "d3dslide-config.h"
#include "FreeImage.h"

namespace d3dslide {

  /*************************************************************************
   * d3d_image_t
   *************************************************************************/

  // I've noticed stuttering that seems to be caused by large textures in
  // GPU memory (i.e., it delays the render thread).  Cache misses?
  static unsigned int MAX_TEXTURE_PIXELS 
    = config_t::get(L"d3d_image.MAX_TEXTURE_PIXELS_X", 0) *
      config_t::get(L"d3d_image.MAX_TEXTURE_PIXELS_Y", 0);

  static const bool PRESCALE_IMAGES
    = config_t::get(L"d3d_image.PRESCALE_IMAGES", true);

  static const int PRESCALE_METHOD
    = config_t::get(L"d3d_image.PRESCALE_METHOD", FILTER_LANCZOS3);

  static const unsigned int PRESCALE_MIN_WIDTH
    = config_t::get(L"d3d_image.PRESCALE_MIN_WIDTH", 0);

  static const unsigned int PRESCALE_MIN_HEIGHT
    = config_t::get(L"d3d_image.PRESCALE_MIN_HEIGHT", 0);

  static const unsigned int PRESCALE_TARGET_WIDTH
    = config_t::get(L"d3d_image.PRESCALE_TARGET_WIDTH", 0);

  static const unsigned int PRESCALE_TARGET_HEIGHT
    = config_t::get(L"d3d_image.PRESCALE_TARGET_HEIGHT", 0);

  static const wstring EFFECT_FILENAME = L"d3dslide.fx";

  /*------------------------------------------------------------------------
   * Constructor
   */
  d3d_image_t::
  d3d_image_t(d3d_control_t& control, const wstring& image_filename)
    : control(control),
      device(control.get_device()),
      screen_width((float)control.get_screen_width()),
      screen_height((float)control.get_screen_height()),
      screen_aspect(screen_width / screen_height),
      image_filename(image_filename),
      strip_vb(NULL),
      texture(NULL),
      width(0),
      height(0),
      pos_x(0.0f),
      pos_y(0.0f),
      scale(1.0f),
      scaled_width(0),
      scaled_height(0),
      effect(NULL)
  {
    anim_reset();
    init_texture();
    init_geometry();
    init_effects();
  }

  /*------------------------------------------------------------------------
   * Destructor
   */
  d3d_image_t::
  ~d3d_image_t(void) {
    control.remove_object(this);
    if (effect) effect->Release();
    if (strip_vb) strip_vb->Release();    
    if (texture) texture->Release();     
  }

  /*------------------------------------------------------------------------
   * set_position
   */
  void 
  d3d_image_t::
  set_position(float x, float y) {
    pos_x = x;
    pos_y = y;

    if (effect) {
      effect->SetFloat("pos_x", pos_x);
      effect->SetFloat("pos_y", pos_y);
    }
  }

  /*------------------------------------------------------------------------
   * set_scale
   */
  void 
  d3d_image_t::
  set_scale(float new_scale) {
    scale         = new_scale;
    scaled_width  = width * scale;
    scaled_height = height * scale;

    if (effect) effect->SetFloat("scale", scale);
  }

  /*------------------------------------------------------------------------
   * set_alpha
   */
  void 
  d3d_image_t::
  set_alpha(float new_alpha) {
    if (new_alpha > 1.0f) new_alpha = 1.0f;
    if (new_alpha < 0.0f) new_alpha = 0.0f;

    alpha = new_alpha;
    
    if (effect) effect->SetFloat("alpha", alpha);
  }

  /*------------------------------------------------------------------------
   * get_scale
   */
  float 
  d3d_image_t::
  get_scale(void) const {
    return scale;
  }

  /*------------------------------------------------------------------------
   * get_max_scale
   *
   * Description:
   *   Retrieves the scale which fits this image to the screen (max).
   */
  float 
  d3d_image_t::
  get_max_scale(void) const {
    float x_scale = 1.0f + (screen_width - width) / width;
    float y_scale = 1.0f + (screen_height - height) / height;

    return max(x_scale, y_scale);
  }

  /*------------------------------------------------------------------------
   * get_aspect_diff
   */
  float 
  d3d_image_t::
  get_aspect_diff(void) const {
    return aspect_diff;
  }

  /*------------------------------------------------------------------------
   * get_pos_x
   */
  float 
  d3d_image_t::
  get_pos_x(void) const {
    return pos_x;
  }

  /*------------------------------------------------------------------------
   * get_pos_y
   */
  float 
  d3d_image_t::
  get_pos_y(void) const {
    return pos_y;
  }

  /*------------------------------------------------------------------------
   * get_left_dist
   */
  float 
  d3d_image_t::
  get_left_dist(float x) const {
    if (_isnan(x)) x = pos_x;

    return x;
  }

  /*------------------------------------------------------------------------
   * get_top_dist
   */
  float 
  d3d_image_t::
  get_top_dist(float y) const {
    if (_isnan(y)) y = pos_y;

    return y;
  }

  /*------------------------------------------------------------------------
   * get_right_dist
   */
  float 
  d3d_image_t::
  get_right_dist(float x) const {
    if (_isnan(x)) x = pos_x;

    return screen_width - scaled_width - x;
  }

  /*------------------------------------------------------------------------
   * get_bottom_dist
   */
  float 
  d3d_image_t::
  get_bottom_dist(float y) const {
    if (_isnan(y)) y = pos_y;

    return screen_height - scaled_height - y;
  }

  /*------------------------------------------------------------------------
   * get_left_remain
   */
  float 
  d3d_image_t::
  get_left_remain(float x) const {
    return max(0, -get_left_dist(x));
  }

  /*------------------------------------------------------------------------
   * get_top_remain
   */
  float 
  d3d_image_t::
  get_top_remain(float y) const {
    return max(0, -get_top_dist(y));
  }

  /*------------------------------------------------------------------------
   * get_right_remain
   */
  float 
  d3d_image_t::
  get_right_remain(float x) const {
    return max(0, -get_right_dist(x));
  }

  /*------------------------------------------------------------------------
   * get_bottom_remain
   */
  float 
  d3d_image_t::
  get_bottom_remain(float y) const {
    return max(0, -get_bottom_dist(y));
  }

  /*------------------------------------------------------------------------
   * is_fullscreen
   */
  bool 
  d3d_image_t::
  is_fullscreen(float x, float y) const {
    return get_left_dist(x)   <= 0 &&
           get_top_dist(y)    <= 0 && 
           get_right_dist(x)  <= 0 &&
           get_bottom_dist(y) <= 0;
  }

  /*------------------------------------------------------------------------
   * fit_min
   */
  void 
  d3d_image_t::
  fit_min(void) {
    float x_scale = 1.0f + (screen_width - width) / width;
    float y_scale = 1.0f + (screen_height - height) / height;

    set_scale(min(x_scale, y_scale));

    set_position(get_right_dist() / 2, pos_y);
  }

  /*------------------------------------------------------------------------
   * fit_max
   */
  void 
  d3d_image_t::
  fit_max(void) {
    set_scale(get_max_scale());
  }

  /*------------------------------------------------------------------------
   * set_pos_center
   */
  void
  d3d_image_t::
  set_pos_center(void) {
    set_position(get_right_remain(0) / -2.0f, get_bottom_remain(0) / -2.0f);
  }

  /*------------------------------------------------------------------------
   * set_pos_random
   */
  void 
  d3d_image_t::
  set_pos_random(void) {
    set_position(rand_float(-get_right_remain(), 0), 
                 rand_float(-get_bottom_remain(), 0));
  }

  /*------------------------------------------------------------------------
   * set_pos_random_corner
   */
  void
  d3d_image_t::
  set_pos_random_corner(void) {
    int corner = rand_int(0, 3);

    switch (corner) {
      // top left
      case 0:
        set_position(0, 0);
        break;

      // top right
      case 1:
        set_position(screen_width - scaled_width, 0);
        break;

      // bottom right
      case 2:
        set_position(screen_width - scaled_width, screen_height - scaled_height);
        break;

      // bottom left
      case 3:
        set_position(0, screen_height - scaled_height);
        break;
    }
  }

  /*------------------------------------------------------------------------
   * zoom
   */
  void 
  d3d_image_t::
  zoom(float amount) {
    set_scale(scale * (1.0f + amount));
  }

  /*------------------------------------------------------------------------
   * anim_reset
   */
  void 
  d3d_image_t::
  anim_reset(void) {
    anim_zoom_amount      = 0.0f;
    anim_zoom_start_scale = 0.0f;
    anim_zoom_duration.set(0);

    anim_glide_start_x = 0;
    anim_glide_start_y = 0;
    anim_glide_left    = false;
    anim_glide_top     = false;
    anim_glide_duration.set(0);

    anim_fade_active      = false;
    anim_fade_amount      = 0.0f;
    anim_fade_start_alpha = 0.0f;
    anim_fade_duration.set(0);
  }

  /*------------------------------------------------------------------------
   * anim_zoom
   */
  void 
  d3d_image_t::
  anim_zoom(float amount, duration_t duration) {
    float full_scale   = get_max_scale();
    
    // Clamp the minimum zoom to be at least full-screen
    if (scale * (1.0f + amount) < full_scale) {
      amount = (full_scale / scale) - 1.0f;
    }

    anim_zoom_amount      = amount;
    anim_zoom_start_scale = scale;
    anim_zoom_duration    = duration;
  }

  /*------------------------------------------------------------------------
   * anim_glide
   */
  void 
  d3d_image_t::
  anim_glide(duration_t duration) {
    anim_glide_left     = get_left_remain() > get_right_remain();
    anim_glide_top      = get_top_remain()  > get_bottom_remain();
    anim_glide_start_x  = pos_x;
    anim_glide_start_y  = pos_y;
    anim_glide_duration = duration;
  }

  /*------------------------------------------------------------------------
   * anim_fade
   */
  void 
  d3d_image_t::
  anim_fade(float amount, duration_t duration) {
    anim_fade_active      = true;
    anim_fade_amount      = amount;
    anim_fade_start_alpha = alpha;
    anim_fade_duration    = duration;
  }

  /*------------------------------------------------------------------------
   * preload (override from d3d_object_t)
   */
  void
  d3d_image_t::
  preload(void) {
    texture->PreLoad();
    set_preloaded(true);
  }

  /*------------------------------------------------------------------------
   * render
   */
  void 
  d3d_image_t::
  render(void) {
    anim_update();    

    UINT num_passes;
    effect->Begin(&num_passes, 0);
      
    for (UINT i = 0; i < num_passes; i++) {
      effect->BeginPass(i);
      
      device->SetStreamSource(0, strip_vb, 0, sizeof(vertex_t));
      device->SetFVF(VERTEX_FVF);
      device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

      effect->EndPass();
    }

    effect->End();
  }

  /*------------------------------------------------------------------------
   * init_geometry
   */
  void 
  d3d_image_t::
  init_geometry(void) {
    D3DXMATRIX mat;
    D3DXMatrixOrthoOffCenterLH(&mat, 
                               0.0f,
                               screen_width,
                               screen_height, // flip vertical axis
                               0.0f,
                               0.0f,
                               1.0f);

    D3DXMatrixTranspose(&projection, &mat);

    vertex_t strip[] = 
    {
      { D3DXVECTOR3(0.0f,  0.0f,   1.0f), D3DXVECTOR2(0.0f, 0.0f) },
      { D3DXVECTOR3(0.0f,  height, 1.0f), D3DXVECTOR2(0.0f, 1.0f) },
      { D3DXVECTOR3(width, 0.0f,   1.0f), D3DXVECTOR2(1.0f, 0.0f) },
      { D3DXVECTOR3(width, height, 1.0f), D3DXVECTOR2(1.0f, 1.0f) },
    };

    device->CreateVertexBuffer(sizeof(strip), 
	                             0,
                               VERTEX_FVF,
                               D3DPOOL_MANAGED,
                               &strip_vb,
                               NULL);

    if (!strip_vb) {
      control.error_quit(L"CreateVertexBuffer");
    }

    void* buff = NULL;
    strip_vb->Lock( 0, sizeof(strip), (void**)&buff, 0);
    memcpy(buff, strip, sizeof(strip));
    strip_vb->Unlock();
  }

  /*------------------------------------------------------------------------
   * init_texture
   */
  void 
  d3d_image_t::
  init_texture(void) {
    D3DXIMAGE_INFO info;
    D3DXGetImageInfoFromFile(image_filename.c_str(), &info);

    if (MAX_TEXTURE_PIXELS) {
      int width_pix  = 64;
      int height_pix = (int) (((float)info.Height / (float)info.Width) * width_pix);

      while (info.Width * info.Height > MAX_TEXTURE_PIXELS) {
        info.Width  -= width_pix;
        info.Height -= height_pix;
      }
    }

    width       = (float)info.Width;
    height      = (float)info.Height;
    aspect      = width / height;
    aspect_diff = abs(screen_aspect - aspect);

    D3DXCreateTextureFromFileEx(device, 
                                image_filename.c_str(),
                                info.Width,
                                info.Height,
                                D3DX_FROM_FILE,
                                0,
                                D3DFMT_A8R8G8B8,
                                D3DPOOL_MANAGED, 
                                D3DX_DEFAULT, 
                                D3DX_FILTER_NONE,//D3DX_DEFAULT,
                                0, 
                                NULL, 
                                NULL, 
                                &texture);

    if (!texture) {
      control.error_quit(L"D3DXCreateTextureFromFileEx() failed: " + image_filename);
    }

    if (PRESCALE_IMAGES) {
      resample_texture();
    }
  } 

  /*------------------------------------------------------------------------
   * resample_texture
   */
  void 
  d3d_image_t::
  resample_texture(void) {
    unsigned long min_width
      = PRESCALE_MIN_WIDTH ? PRESCALE_MIN_WIDTH 
                           : control.get_screen_width();

    unsigned long min_height
      = PRESCALE_MIN_HEIGHT ? PRESCALE_MIN_HEIGHT 
                            : control.get_screen_height();

    unsigned long target_width
      = PRESCALE_TARGET_WIDTH  ? PRESCALE_TARGET_WIDTH 
                               : (unsigned long) (control.get_screen_width() * 1.5);

    unsigned long target_height
      = PRESCALE_TARGET_HEIGHT ? PRESCALE_TARGET_HEIGHT 
                               : (unsigned long) (control.get_screen_height() * 1.5);

    if (width >= min_width && height >= min_height) {
      return;
    }

    unsigned long new_width  = (unsigned long) width;
    unsigned long new_height = (unsigned long) height;

    if (new_width < target_width) {
      new_width  = target_width;
      new_height = (unsigned long) (new_width / aspect);
    }

    if (new_height < target_height) {
      new_height = target_height;
      new_width = (unsigned long) (new_height * aspect);
    }

    while (MAX_TEXTURE_PIXELS && new_width * new_height > MAX_TEXTURE_PIXELS) {
      new_width  -= 64;
      new_height  = (unsigned long) (new_width / aspect);
    }

    if (new_width < width || new_height < height) {
      return;
    }

    FIBITMAP* fib = FreeImage_Allocate((int) width, (int) height, 32);
    if (!fib) control.error_quit(L"FreeImage_Allocate() failed: " + image_filename);

    D3DLOCKED_RECT src_rect = { 0 };
    texture->LockRect(0, &src_rect, NULL, D3DLOCK_READONLY);
    memcpy(FreeImage_GetBits(fib), src_rect.pBits, src_rect.Pitch * (unsigned long)height);
    texture->UnlockRect(0);    

    FIBITMAP* scaled_fib = FreeImage_Rescale(fib, 
                                             new_width,
                                             new_height,
                                             FREE_IMAGE_FILTER(PRESCALE_METHOD));
    if (!scaled_fib) control.error_quit(L"FreeImage_Rescale() failed: " + image_filename);   

    FreeImage_Unload(fib);

    LPDIRECT3DTEXTURE9 new_texture = NULL;
    D3DXCreateTexture(device,
                      new_width,
                      new_height,
                      1,
                      0,
                      D3DFMT_A8R8G8B8,
                      D3DPOOL_MANAGED,
                      &new_texture);

    D3DLOCKED_RECT dst_rect = { 0 };
    new_texture->LockRect(0, &dst_rect, NULL, 0);    
    memcpy(dst_rect.pBits, FreeImage_GetBits(scaled_fib), dst_rect.Pitch * new_height);
    new_texture->UnlockRect(0);

    FreeImage_Unload(scaled_fib);

    texture->Release();

    texture = new_texture;
    width   = (float) new_width;
    height  = (float) new_height;
  }

  /*------------------------------------------------------------------------
   * init_effects
   */
  void 
  d3d_image_t::
  init_effects(void) {
    LPD3DXBUFFER error_buff = NULL;
    HRESULT      hr         = D3DXCreateEffectFromFile(device, 
                                                       EFFECT_FILENAME.c_str(),
	                                                     NULL, 
	                                                     NULL, 
	                                                     0, 
	                                                     NULL, 
	                                                     &effect, 
	                                                     &error_buff);

	  if (FAILED(hr)) {
	    const wchar_t* error_msg 
        = error_buff ? (const wchar_t*)error_buff->GetBufferPointer() // XXX: is this cast right?
                     : L"failed to load";

      control.error_quit(L"D3DXCreateEffectFromFile(" + EFFECT_FILENAME + L") failed: " + error_msg);
    }

    effect->SetTechnique("show_image");
    effect->SetTexture("image", texture);
    effect->SetMatrix("proj_matrix", &projection);

    set_scale(scale);
    set_position(pos_x, pos_y);
    set_alpha(alpha);
  }

  /*------------------------------------------------------------------------
   * anim_update
   */
  void 
  d3d_image_t::
  anim_update(void) {
    // Update zoom animation
    if (!anim_zoom_duration.done()) {
      float old_scaled_width  = scaled_width;
      float old_scaled_height = scaled_height;
      float old_scale         = scale;
      set_scale(anim_zoom_start_scale * (1.0f + anim_zoom_duration.progress_frames() * anim_zoom_amount));

      // Compute a scaled position
      float pos_delta_x = (old_scaled_width  - scaled_width)  / 2.0f;
      float pos_delta_y = (old_scaled_height - scaled_height) / 2.0f;      

      float new_pos_x = pos_x + pos_delta_x;
      float new_pos_y = pos_y + pos_delta_y;   

      // If it doesn't fit on the screen anymore, tinker with the position
      // trying to make it fit.
      if (!is_fullscreen(new_pos_x, new_pos_y)) {
        float d = 0;

        d = get_left_dist(new_pos_x);
        if (d > 0) {
          pos_delta_x -= d;
        } else {
          d = get_right_dist(new_pos_x);
          if (d > 0) pos_delta_x += d;
        }

        d = get_top_dist(new_pos_y);
        if (d > 0) {
          pos_delta_y -= d;
        } else {
          d = get_bottom_dist(new_pos_y);
          if (d > 0) pos_delta_y += d;
        }       

        new_pos_x = pos_x + pos_delta_x;
        new_pos_y = pos_y + pos_delta_y;
      }      

      // If it fits now, keep the position otherwise give up
      if (is_fullscreen(new_pos_x, new_pos_y)) {
        set_position(new_pos_x, new_pos_y);

        // It's messy having to muck with the other animation's data.. :(
        anim_glide_start_x += pos_delta_x;
        anim_glide_start_y += pos_delta_y;
      } else {
        set_scale(old_scale);
      }
    }

    // Update glide animation
    if (!anim_glide_duration.done()) {
      float progress = anim_glide_duration.progress_frames();

      float new_x = anim_glide_start_x;      
      if (anim_glide_left) {
        new_x += progress * get_left_remain(anim_glide_start_x);
        if (new_x < pos_x) new_x = pos_x;
      } else {
        new_x -= progress * get_right_remain(anim_glide_start_x);
        if (new_x > pos_x) new_x = pos_x;
      }

      float new_y = anim_glide_start_y;
      if (anim_glide_top) {
        new_y += progress * get_top_remain(anim_glide_start_y);
        if (new_y < pos_y) new_y = pos_y;
      } else {
        new_y -= progress * get_bottom_remain(anim_glide_start_y);
        if (new_y > pos_y) new_y = pos_y;
      }

      set_position(new_x, new_y);
    }

    // Update fade animation.  Use this boolean instead of the done() call to
    // make sure we set the final alpha value during a jitter.
    if (anim_fade_active) {
      set_alpha(anim_fade_start_alpha + 
                  anim_fade_duration.progress() * anim_fade_amount);

      anim_fade_active = !anim_fade_duration.done();
    }
  }

}
