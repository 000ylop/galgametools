// crx-common.h, v1.0 2012/06/08
// coded by asmodean

// contact: 
//   web:   http://asmodean.reverse.net
//   email: asmodean [at] hush.com
//   irc:   asmodean on efnet (irc.efnet.net)

// This file contains shared functions for processing CRXG (*.crx) graphics.

#ifndef CRXG_H
#define CRXG_H

#include "as-util.h"

bool proc_crxg(const string&  in_filename,
               const string&  out_filename,
               unsigned char* buff,
               unsigned long  len,
               const string&  base_filename = "");

#endif /* CRXG_H */