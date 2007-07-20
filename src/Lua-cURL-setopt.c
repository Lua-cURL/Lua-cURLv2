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

static int l_easy_setopt_long(lua_State *L) {
  CURL *curl = LUACURL_PRIVATEP_UPVALUE(L, 1)->curl;
  CURLoption *optionp = LUACURL_OPTIONP_UPVALUE(L, 2);
  long value = luaL_checklong(L,1);

  if (curl_easy_setopt(curl, *optionp, value) != CURLE_OK)
    luaL_error(L, "%s", LUACURL_PRIVATEP_UPVALUE(L, 1)->error);
  return 0;
}

static int l_easy_setopt_string(lua_State *L) {
  CURL *curl = LUACURL_PRIVATEP_UPVALUE(L, 1)->curl;
  CURLoption *optionp = LUACURL_OPTIONP_UPVALUE(L, 2);  
  const char *value = luaL_checkstring(L, 1);

  if (curl_easy_setopt(curl, *optionp, value) != CURLE_OK)
    luaL_error(L, "%s", LUACURL_PRIVATEP_UPVALUE(L, 1)->error);
  return 0;
}

/* closures assigned to setopt in setopt table */
static struct {
  const char *name;
  CURLoption option; 
  lua_CFunction func;
} luacurl_setopt_c[] = {
  /* behavior options */
  {"verbose", CURLOPT_VERBOSE, l_easy_setopt_long},
  {"header", CURLOPT_HEADER, l_easy_setopt_long},
  {"noprogrss", CURLOPT_NOPROGRESS, l_easy_setopt_long},
  {"nosignal", CURLOPT_NOSIGNAL, l_easy_setopt_long},
  /* callback options */
  /* network options */
  {"url", CURLOPT_URL, l_easy_setopt_string},

  {"ssl_verifypeer", CURLOPT_SSL_VERIFYPEER, l_easy_setopt_long},
  /* dummy opt value */
  {NULL, CURLOPT_VERBOSE, NULL}};	

/* create setopt subtable */
int l_easy_setopt_newtable(lua_State *L, l_private *privp) {
  int i;

  lua_newtable(L);		
  /* assign setopt closures to setopt subtable */
  for (i=0; luacurl_setopt_c[i].name != NULL; i++) {
    CURLoption *optionp = &(luacurl_setopt_c[i].option);
    lua_pushlightuserdata(L, privp); 
    lua_pushlightuserdata(L, optionp);
    lua_pushcclosure(L, luacurl_setopt_c[i].func, 2);
    lua_setfield(L, -2, luacurl_setopt_c[i].name);
  }

  /* setopt table is on top of the stack */
  return 1;
}


