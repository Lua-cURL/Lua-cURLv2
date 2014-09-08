/******************************************************************************
* Author: Alexey Melnichuk <mimir@newmail.ru>
*
* Copyright (C) 2014 Alexey Melnichuk <mimir@newmail.ru>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of lua-lcurl library.
******************************************************************************/

#ifndef _LCHTTPPOST_H_
#define _LCHTTPPOST_H_

#include "lcurl.h"
#include "lcutils.h"
#include <stdlib.h>

typedef struct lcurl_hpost_stream_tag{
  lua_State          *L;
  lcurl_callback_t    rd;
  lcurl_read_buffer_t rbuffer;
  struct lcurl_hpost_stream_tag *next;
}lcurl_hpost_stream_t;

typedef struct lcurl_hpost_tag{
  struct curl_httppost *post;
  struct curl_httppost *last;
  int storage;
  int err_mode;
  lcurl_hpost_stream_t *stream;
}lcurl_hpost_t;

int lcurl_hpost_create(lua_State *L, int error_mode);

void lcurl_hpost_initlib(lua_State *L, int nup);

lcurl_hpost_t *lcurl_gethpost_at(lua_State *L, int i);

#define lcurl_gethpost(L) lcurl_gethpost_at((L),1)


#endif
