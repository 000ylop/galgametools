// d3dslide-config.cpp, v1.1 2012/06/08
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

#include <windows.h>
#include "d3dslide-config.h"

namespace d3dslide {

  /*************************************************************************
   * config_t
   *************************************************************************/

  // Use defines so we don't have an order-of-static-initialization problem.
  #define APP_NAME    L"d3dslide"
  #define CONFIG_FILE L".\\d3dslide.ini"

  /*------------------------------------------------------------------------
   * get (char*)
   */
  wstring 
  config_t::
  get(const wstring& name, const wchar_t* def) {
    wchar_t buff[1024] = { 0 };

    DWORD rc = GetPrivateProfileString(APP_NAME,
                                       name.c_str(),
                                       def,
                                       buff,
                                       sizeof(buff),
                                       CONFIG_FILE);

    return buff;
  }

  /*------------------------------------------------------------------------
   * get (string)
   */
  wstring 
  config_t::
  get(const wstring& name, const wstring& def) {
    return get(name, def.c_str());
  }

  /*------------------------------------------------------------------------
   * get (int)
   */
  int 
  config_t::    
  get(const wstring& name, int def) {
    wstring s = get(name, L"");

    return s == L"" ? def : _wtoi(s.c_str());
  }

  /*------------------------------------------------------------------------
   * get (float)
   */
  float 
  config_t::  
  get(const wstring& name, float def) {
    wstring s = get(name, L"");

    return s == L"" ? def : (float)_wtof(s.c_str());
  }

  /*------------------------------------------------------------------------
   * get (bool)
   */
  bool 
  config_t::  
  get(const wstring& name, bool def) {
    wstring s = get(name, L"");

    return s == L"" ? def : (s == L"true");
  }

}