/******************************************************************************
* Copyright (C) 2007 Juergen Hoetzel
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/

#include "Lua-cURL.h"
#include "Lua-utility.h"

#define P "setopt_"

static int l_easy_setopt_long(lua_State *L) {
  l_easy_private *privatep = luaL_checkudata(L, 1, LUACURL_EASYMETATABLE);
  CURL *curl = privatep->curl;
  CURLoption *optionp = LUACURL_OPTIONP_UPVALUE(L, 2);
  long value = luaL_checklong(L,1);

  if (curl_easy_setopt(curl, *optionp, value) != CURLE_OK)
    luaL_error(L, "%s", privatep->error);
  return 0;
}

static int l_easy_setopt_string(lua_State *L) {
  l_easy_private *privatep = luaL_checkudata(L, 1, LUACURL_EASYMETATABLE);
  CURL *curl = privatep->curl;
  CURLoption *optionp = LUACURL_OPTIONP_UPVALUE(L, 1);
  const char *value = luaL_checkstring(L, 2);

  if (curl_easy_setopt(curl, *optionp, value) != CURLE_OK)
    luaL_error(L, "%s", privatep->error);
  return 0;
}

/* closures assigned to setopt in setopt table */
static struct {
  const char *name;
  CURLoption option; 
  lua_CFunction func;
} luacurl_setopt_c[] = {
  /* behavior options */
  {P"verbose", CURLOPT_VERBOSE, l_easy_setopt_long},
  {P"header", CURLOPT_HEADER, l_easy_setopt_long},
  {P"noprogrss", CURLOPT_NOPROGRESS, l_easy_setopt_long},
  {P"nosignal", CURLOPT_NOSIGNAL, l_easy_setopt_long},
  /* callback options */
  /* implemented differently */
  /* network options */
  {P"url", CURLOPT_URL, l_easy_setopt_string},

  {P"ssl_verifypeer", CURLOPT_SSL_VERIFYPEER, l_easy_setopt_long},
  /* dummy opt value */
  {NULL, CURLOPT_VERBOSE, NULL}};	

int l_easy_setopt_register(lua_State *L) {
  int i;

  /* register setopt closures */
  for (i=0; luacurl_setopt_c[i].name != NULL; i++) {
    CURLoption *optionp = &(luacurl_setopt_c[i].option);
    lua_pushlightuserdata(L, optionp);
    lua_pushcclosure(L, luacurl_setopt_c[i].func, 1);
    lua_setfield(L, -2, luacurl_setopt_c[i].name);
  }

  return 0;
}


