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

static int l_multi_internalcallback(void *ptr, size_t size, size_t nmemb, void *stream) {
  lua_State* L = (lua_State*)stream;
  printf("InternalCallback\n");
  lua_pushlstring(L, ptr, nmemb*size);
  stackDump(L);
  return nmemb*size;
}


int l_multi_add_handle (lua_State *L) {
  l_multi_private *privatep = luaL_checkudata(L, 1, LUACURL_MULTIMETATABLE);  
  CURLM *curlm = privatep->curlm;
  CURLMcode rc;

  /* get easy userdata */
  l_easy_private *easyp = luaL_checkudata(L, 2, LUACURL_EASYMETATABLE);

  if ((rc = curl_multi_add_handle(curlm, easyp->curl)) != CURLM_OK)
    luaL_error(L, "cannot add handle: %s", curl_multi_strerror(rc));

  /* settup internal callback */
  if (curl_easy_setopt(easyp->curl, CURLOPT_WRITEDATA ,L) != CURLE_OK)
    luaL_error(L, "%s", easyp->error);
  if (curl_easy_setopt(easyp->curl, CURLOPT_WRITEFUNCTION, l_multi_internalcallback) != CURLE_OK)
    luaL_error(L, "%s", easyp->error);
  return 0;
}

static int l_multi_perform_internal (lua_State *L) {
  l_multi_private *privatep = LUACURL_PRIVATE_MULTIP_UPVALUE(L, 1);
  CURLM *curlm = privatep->curlm;
  CURLMcode rc;
  int remain = 1;

  int prev_top = lua_gettop(L);
  int current_top;
  /* need to read more data */
  while (remain) {
    rc = curl_multi_perform(curlm, &remain);
    switch (rc) {
    case CURLM_CALL_MULTI_PERFORM:
      break;
    case CURLM_OK:
      current_top = lua_gettop(L);
      if ( current_top > prev_top) { 
	lua_concat(L, current_top - prev_top);
	/* return strings from callbacks */
	return 1;
      }
      break;
    default:
      luaL_error(L, "cannot perform: %s", curl_multi_strerror(rc));
    }
  }

  /* no more data */
  return 0;			/* nil */
}

/* return closure */
int l_multi_perform (lua_State *L) {
  luaL_checkudata(L, 1, LUACURL_MULTIMETATABLE);  
  lua_pushcclosure(L, l_multi_perform_internal, 1);
  return 1;
}

int l_multi_gc (lua_State *L) {
  printf("Not implemented\n");
  return 0;
}
