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


int l_multi_init(lua_State *L) {
  l_multi_private *privp;
  
  /* create userdata and assign metatable */
  privp = (l_multi_private *) lua_newuserdata(L, sizeof(l_multi_private));
  luaL_getmetatable(L, LUACURL_MULTIMETATABLE);
  lua_setmetatable(L, -2);

  if ((privp->curlm = curl_multi_init()) == NULL)
    return luaL_error(L, "something went wrong and you cannot use the other curl functions");

  /* return userdata */
  return 1;			
}

int l_multi_add_handle (lua_State *L) {
  l_multi_private *privatep = luaL_checkudata(L, 1, LUACURL_MULTIMETATABLE);  
  CURLM *curlm = privatep->curlm;
  CURLMcode rc;

  /* get easy userdata */
  l_easy_private *easyp = luaL_checkudata(L, 2, LUACURL_EASYMETATABLE);

  if ((rc = curl_multi_add_handle(curlm, easyp->curl)) != CURLM_OK)
    luaL_error(L, "cannot add handle: %s", curl_multi_strerror(rc));

  return 0;
}

int l_multi_perform (lua_State *L) {
  CURLMcode rc;
  luaL_checkudata(L, lua_upvalueindex(1), LUACURL_MULTIMETATABLE);  
  CURLM *curlm = LUACURL_PRIVATE_MULTIP_UPVALUE(L, 1)->curlm;
  int value;

  return 0;
  rc = curl_multi_perform(curlm, &value);
  if (rc == CURLM_CALL_MULTI_PERFORM) {
    printf("Try again\n");
  }
  else if (rc != CURLM_OK)
    luaL_error(L, "cannot perform: %s", curl_multi_strerror(rc));
  
  lua_pushinteger(L, value);
  return 1;
}

int l_multi_gc (lua_State *L) {
  printf("Not implemented\n");
  return 0;
}
