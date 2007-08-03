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

/* malloc */
#include <stdlib.h>

/* methods assigned to easy table */
static const struct luaL_Reg luacurl_easy_m[] = {
  {"escape", l_easy_escape},
  {"perform", l_easy_perform},
  {"unescape", l_easy_unescape},
  {"post", l_easy_post},
  {"__gc", l_easy_gc},
  /* not for public use */
  {"userdata", l_easy_userdata},
  {NULL, NULL}};



/* global functions in module namespace*/
static const struct luaL_Reg luacurl_f[] = {
  {"easy_init", l_easy_init},
  {"multi_init", l_multi_init},
  {"getdate", l_getdate},
  {"unescape", l_unescape},
  {"version", l_version},
  {"version_info", l_version_info},
  {NULL, NULL}};

/* functions assigned to metatable */
static const struct luaL_Reg luacurl_m[] = {

  {NULL, NULL}};

int l_easy_escape(lua_State *L) {
  size_t length = 0;
  l_easy_private *privatep = luaL_checkudata(L, 1, LUACURL_EASYMETATABLE);
  CURL *curl = privatep->curl;
  const char *url = luaL_checklstring(L, 1, &length);
  char *rurl = curl_easy_escape(curl, url, length);
  lua_pushstring(L, rurl);
  curl_free(rurl);
  return 1;
}


int l_easy_perform(lua_State *L) {
  l_easy_private *privatep = luaL_checkudata(L, 1, LUACURL_EASYMETATABLE);
  CURL *curl = privatep->curl;
  
  if (curl_easy_perform(curl) != CURLE_OK) 
    luaL_error(L, "%s", privatep->error);
  return 0;
}

int l_easy_init(lua_State *L) {
  l_easy_private *privp;
  
  /* check optional callback table */
  luaL_opt(L, luaL_checktable, 1, lua_newtable(L));

  /* create userdata and assign metatable */
  privp = (l_easy_private *) lua_newuserdata(L, sizeof(l_easy_private));
  luaL_getmetatable(L, LUACURL_EASYMETATABLE);
  lua_setmetatable(L, - 2);

  if ((privp->curl = curl_easy_init()) == NULL)
    return luaL_error(L, "something went wrong and you cannot use the other curl functions");

  /* set error buffer */
  if (curl_easy_setopt(privp->curl, CURLOPT_ERRORBUFFER, privp->error) != CURLE_OK)
    return luaL_error(L, "cannot set error buffer");

/*   /\* setup write callback function only if entry exists in callback-table *\/ */
/*   lua_getfield(L, 1, "writefunction"); */
/*   if (lua_isfunction(L, -1)) */
/*     l_easy_setup_writefunction(L, privp->curl); */
/*   lua_pop(L, 1); */
  
/*   /\* setup header callback function only if entry exists in callback-table *\/   */
/*   lua_getfield(L, 1, "headerfunction"); */
/*   if (lua_isfunction(L, -1))  */
/*     l_easy_setup_headerfunction(L, privp->curl); */
/*   lua_pop(L, 1); */

  /* set table of callback functions  as environment for userdata*/
/*   lua_pushvalue(L, 1);		 */
/*   lua_setfenv(L, -2); */

/*     /\* create the setopt table *\/ */
/*   l_easy_setopt_newtable(L, privp); */
/*   /\* and assign to easy table *\/ */
/*   lua_setfield(L, -2, "setopt");   */



  /* return userdata; */
  stackDump(L);
  lua_remove(L, 1);
  return 1;			
}

int l_getdate(lua_State *L) {
  const char *date = luaL_checkstring(L, 1);
  time_t t = curl_getdate(date, NULL);
  if (t == -1) 
    return luaL_error(L, "fails to parse the date string");
  lua_pushinteger(L, t);
  return 1;
}


int l_easy_unescape(lua_State *L) {
  size_t inlength = 0;
  int outlength;
  l_easy_private *privatep = luaL_checkudata(L, 1, LUACURL_EASYMETATABLE);
  CURL *curl = privatep->curl;
  const char *url = luaL_checklstring(L, 1, &inlength);
  char *rurl = curl_easy_unescape(curl, url, inlength, &outlength);
  lua_pushlstring(L, rurl, outlength);
  curl_free(rurl);
  return 1;
}

int l_easy_userdata (lua_State *L) {
  lua_pushvalue(L, lua_upvalueindex(1));
  return 1;
}

/* deprecated */
int l_unescape(lua_State *L) {
  size_t length;
  const char *url = luaL_checklstring(L, 1, &length);
  char *rurl = curl_unescape(url, length);
  lua_pushstring(L, rurl);
  curl_free(rurl);
  return 1;
}


int l_version(lua_State *L) {
  lua_pushstring(L, curl_version());
  return 1;
}

int l_version_info (lua_State *L) {
  int i;
  curl_version_info_data *d = curl_version_info(CURLVERSION_NOW);	

  lua_newtable(L);

  lua_pushliteral(L, "version");
  lua_pushstring(L, d->version);
  lua_settable(L, -3);

  lua_pushliteral(L, "version_num");
  lua_pushinteger(L, d->version_num);
  lua_settable(L, -3);  

  lua_pushliteral(L, "host");
  lua_pushstring(L, d->host);
  lua_settable(L, -3);  

  /* create features table */
  lua_pushliteral(L, "features");
  lua_newtable(L);
  
  lua_pushliteral(L, "ipv6");
  lua_pushboolean(L, d->features & CURL_VERSION_IPV6);
  lua_settable(L, -3);

  lua_pushliteral(L, "kerberos4");
  lua_pushboolean(L, d->features & CURL_VERSION_KERBEROS4);
  lua_settable(L, -3);

  lua_pushliteral(L, "ssl");
  lua_pushboolean(L, d->features & CURL_VERSION_SSL);
  lua_settable(L, -3);

  lua_pushliteral(L, "libz");
  lua_pushboolean(L, d->features & CURL_VERSION_LIBZ);
  lua_settable(L, -3);

  lua_pushliteral(L, "libz");
  lua_pushboolean(L, d->features & CURL_VERSION_LIBZ);
  lua_settable(L, -3);

  lua_pushliteral(L, "ntlm");
  lua_pushboolean(L, d->features & CURL_VERSION_NTLM);
  lua_settable(L, -3);

  lua_pushliteral(L, "gssnegotiate");
  lua_pushboolean(L, d->features & CURL_VERSION_GSSNEGOTIATE);
  lua_settable(L, -3);

  lua_pushliteral(L, "debug");
  lua_pushboolean(L, d->features & CURL_VERSION_DEBUG);
  lua_settable(L, -3);

  lua_pushliteral(L, "asynchdns");
  lua_pushboolean(L, d->features & CURL_VERSION_ASYNCHDNS);
  lua_settable(L, -3);

  lua_pushliteral(L, "spnego");
  lua_pushboolean(L, d->features & CURL_VERSION_SPNEGO);
  lua_settable(L, -3);

  lua_pushliteral(L, "largefile");
  lua_pushboolean(L, d->features & CURL_VERSION_LARGEFILE);
  lua_settable(L, -3);

  lua_pushliteral(L, "idn");
  lua_pushboolean(L, d->features & CURL_VERSION_IDN);
  lua_settable(L, -3);

  lua_pushliteral(L, "sspi");
  lua_pushboolean(L, d->features & CURL_VERSION_SSPI);
  lua_settable(L, -3);

  lua_pushliteral(L, "conv");
  lua_pushboolean(L, d->features & CURL_VERSION_CONV);
  lua_settable(L, -3);

  lua_settable(L, -3);

  /* ssl */
  lua_pushliteral(L, "ssl_version");
  lua_pushstring(L, d->ssl_version);
  lua_settable(L, -3);

  lua_pushliteral(L, "libz_version");
  lua_pushstring(L, d->libz_version);
  lua_settable(L, -3);

  /* create protocols table*/
  lua_pushstring(L,"protocols");
  lua_newtable(L);

  for(i=0; d->protocols[i] != NULL; i++){
	lua_pushinteger(L, i+1);
	lua_pushstring(L, d->protocols[i]);
	lua_settable(L, -3);
  }
  
  lua_settable(L, -3);

  if (d->age >= 1) {
    lua_pushliteral(L, "ares");
    lua_pushstring(L, d->ares);
    lua_settable(L, -3);

    lua_pushliteral(L, "ares_num");
    lua_pushinteger(L, d->ares_num);
    lua_settable(L, -3);
  }

  if (d->age >= 2) {
    lua_pushliteral(L, "libidn");
    lua_pushstring(L, d->libidn);
    lua_settable(L, -3);
  }

  if (d->age >= 3) {
    lua_pushliteral(L, "iconv_ver_num");
    lua_pushinteger(L, d->iconv_ver_num);
    lua_settable(L, -3);
  }

  /* return table*/
  return 1;			
}

int l_easy_gc(lua_State *L) {
  /* gc resources optained by cURL userdata */
  l_easy_private *privp = lua_touserdata(L, 1);
  curl_easy_cleanup(privp->curl);
  return 0;
}

/* registration hook function */
int luaopen_cURL(lua_State *L) {
  luaL_newmetatable(L, LUACURL_EASYMETATABLE);
  
  /* register in easymetatable */
  luaL_register(L, NULL, luacurl_easy_m);  

  /* easymetatable.__index = easymetatable */
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  stackDump(L);
  /* register getinfo closures  */
  l_easy_getinfo_register(L);
  /* register setopt closures  */
  l_easy_setopt_register(L);  

  luaL_newmetatable(L, LUACURL_MULTIMETATABLE);  
  luaL_register(L, "cURL", luacurl_f);

  return 1;
}

