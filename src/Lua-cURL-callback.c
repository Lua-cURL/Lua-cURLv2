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

static int l_easy_writefunction(void *ptr, size_t size, size_t nmemb, void *stream) {
  lua_State* L = (lua_State*)stream;
  lua_getfenv(L, lua_upvalueindex(1));
  stackDump(L);
  lua_getfield(L, -1, "writefunction");
  lua_pushlstring(L, (char*) ptr, nmemb * size);
  lua_call(L, 1, 0);
  lua_pop(L, 1);		/* remove fenv from stack */
  return nmemb*size;
}

static int l_easy_headerfunction(void *ptr, size_t size, size_t nmemb, void *stream) {
  lua_State* L = (lua_State*)stream;
  lua_getfenv(L, lua_upvalueindex(1));
  stackDump(L);
  lua_getfield(L, -1, "headerfunction");
  lua_pushlstring(L, (char*) ptr, nmemb * size);
  lua_call(L, 1, 0);
  lua_pop(L, 1);		/* remove fenv from stack */
  return nmemb*size;
}


int l_easy_setup_writefunction(lua_State *L, CURL* curl) {
  
  /* Lua State as userdata argument */
  if (curl_easy_setopt(curl, CURLOPT_WRITEDATA ,L) != CURLE_OK)
    luaL_error(L, "%s", LUACURL_PRIVATEP_UPVALUE(L, 1)->error);

  if (curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, l_easy_writefunction) != CURLE_OK)
    luaL_error(L, "%s", LUACURL_PRIVATEP_UPVALUE(L, 1)->error);
  return 0;
}

int l_easy_setup_headerfunction(lua_State *L, CURL* curl) {
  
  /* Lua State as userdata argument */
  if (curl_easy_setopt(curl, CURLOPT_WRITEHEADER ,L) != CURLE_OK)
    luaL_error(L, "%s", LUACURL_PRIVATEP_UPVALUE(L, 1)->error);

  if (curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, l_easy_headerfunction) != CURLE_OK)
    luaL_error(L, "%s", LUACURL_PRIVATEP_UPVALUE(L, 1)->error);
  return 0;
}

/* create callback subtable */
int l_easy_callback_newtable(lua_State *L) {
  int i = 0;
  char *callbacks[] = {"readfunction", "writefunction" , NULL};

  lua_newtable(L);		
  while (callbacks[i] != NULL) {
    printf("Registration");
    lua_pushboolean(L, 0);	/* default no callback */
    lua_setfield(L, -2, callbacks[i++]);
  }
		 
  /* callback table is on top of the stack */
  return 1;
}
