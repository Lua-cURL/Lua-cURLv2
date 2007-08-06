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

#include <string.h>

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

static int l_easy_setopt_proxytype(lua_State *L) {
  l_easy_private *privatep = luaL_checkudata(L, 1, LUACURL_EASYMETATABLE);
  CURL *curl = privatep->curl;
  CURLoption *optionp = LUACURL_OPTIONP_UPVALUE(L, 1);
  const char *value = luaL_checkstring(L, 2);

  /* check for valid OPTION: */
  curl_proxytype type;

  if (!strcmp("HTTP", value)) 
    type = CURLPROXY_HTTP;
  else if (!strcmp("SOCKS4", value)) 
    type = CURLPROXY_SOCKS4;
  else if (!strcmp("SOCKS5", value)) 
    type = CURLPROXY_SOCKS5;
  else 
    luaL_error(L, "Invalid proxytype: %s", value);

  if (curl_easy_setopt(curl, *optionp, type) != CURLE_OK)
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
  /* network options */
  /* names and passwords options  */
  /* http options */
  /* ftp options */
  /* protocol options */
  /* network options */
  {P"url", CURLOPT_URL, l_easy_setopt_string},
  {P"proxy", CURLOPT_PROXY, l_easy_setopt_string},
  {P"proxyport", CURLOPT_PROXYPORT, l_easy_setopt_long},
  {P"proxytype", CURLOPT_PROXYTYPE, l_easy_setopt_proxytype},
  {P"httpproxytunnel", CURLOPT_HTTPPROXYTUNNEL, l_easy_setopt_long},
  {P"interface", CURLOPT_INTERFACE, l_easy_setopt_string},
  {P"localport", CURLOPT_LOCALPORT, l_easy_setopt_long},
  {P"localportrange", CURLOPT_LOCALPORTRANGE, l_easy_setopt_long},
  {P"dns_cache_timeout", CURLOPT_DNS_CACHE_TIMEOUT, l_easy_setopt_long},
  {P"dns_use_global_cache", CURLOPT_DNS_USE_GLOBAL_CACHE, l_easy_setopt_long},
  {P"buffersize", CURLOPT_BUFFERSIZE, l_easy_setopt_long},
  {P"port", CURLOPT_PORT, l_easy_setopt_long},
  {P"TCP_nodelay", CURLOPT_TCP_NODELAY, l_easy_setopt_long},
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


