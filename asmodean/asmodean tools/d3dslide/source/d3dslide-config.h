// d3dslide-config.h, v1.1 2012/06/08
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

#ifndef D3DSLIDE_CONFIG_H
#define D3DSLIDE_CONFIG_H

#include <string>

namespace d3dslide {

  using std::wstring;

  /***************************************************************************
   * config_t
   *
   * Description:
   *   Encapsulates configuration parameters.
   */
  class config_t {
  public:
    /*------------------------------------------------------------------------
     * get (char*)
     *
     * Description:
     *   Gets a named parameter as a wstring.
     */
    static wstring get(const wstring& name, const wchar_t* def);

    /*------------------------------------------------------------------------
     * get (string)
     *
     * Description:
     *   Gets a named parameter as a wstring.
     */
    static wstring get(const wstring& name, const wstring& def);

    /*------------------------------------------------------------------------
     * get (int)
     *
     * Description:
     *   Gets a named parameter as an integer.
     */
    static int get(const wstring& name, int def);

    /*------------------------------------------------------------------------
     * get (float)
     *
     * Description:
     *   Gets a named parameter as an integer.
     */
    static float get(const wstring& name, float def);

    /*------------------------------------------------------------------------
     * get (bool)
     *
     * Description:
     *   Gets a named parameter as an boolean.
     */
    static bool get(const wstring& name, bool def);
  };
}

#endif /* D3DSLIDE_CONFIG_H */
