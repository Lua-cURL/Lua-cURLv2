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

/* closures assigned to easy table */
static const struct luaL_Reg luacurl_c[] = {
  {"easy_escape", l_easy_escape},
  {"easy_perform", l_easy_perform},
  {"easy_unescape", l_easy_unescape},
  {NULL, NULL}};

/* functions in module namespace*/
static const struct luaL_Reg luacurl_f[] = {
  {"easy_init", l_easy_init},
  {"getdate", l_getdate},
  {"unescape", l_unescape},
  {"version", l_version},
  {"version_info", l_version_info},
  {NULL, NULL}};

/* functions assigned to metatable */
static const struct luaL_Reg luacurl_m[] = {
  {"__gc", l_easy_gc},
  {NULL, NULL}};

int l_easy_escape(lua_State *L) {
  size_t length = 0;
  CURL *curl = LUACURL_PRIVATEP_UPVALUE(L, 1)->curl;
  const char *url = luaL_checklstring(L, 1, &length);
  char *rurl = curl_easy_escape(curl, url, length);
  lua_pushstring(L, rurl);
  curl_free(rurl);
  return 1;
}


int l_easy_perform(lua_State *L) {
  luaL_checkudata(L, lua_upvalueindex(1), LUACURL_EASYMETATABLE);
  CURL *curl = LUACURL_PRIVATEP_UPVALUE(L, 1)->curl;
  
  l_easy_setup_writefunction(L, curl);
  
  if (curl_easy_perform(curl) != CURLE_OK) 
    luaL_error(L, "%s", LUACURL_PRIVATEP_UPVALUE(L, 1)->error);
  return 0;
}

int l_easy_init(lua_State *L) {
  l_private *privp = (l_private *) lua_newuserdata(L, sizeof(l_private));
  luaL_getmetatable(L, LUACURL_EASYMETATABLE);
  lua_setmetatable(L, -2);

  if ((privp->curl = curl_easy_init()) == NULL)
    return luaL_error(L, "something went wrong and you cannot use the other curl functions");

  /* set error buffer */
  if (curl_easy_setopt(privp->curl, CURLOPT_ERRORBUFFER, privp->error) != CURLE_OK)
    return luaL_error(L, "cannot set error buffer");

  /* check optional callback table */
  luaL_opt(L, luaL_checktable, 1, lua_newtable(L));

  /* set table of callback functions  as environment for userdata*/
  lua_pushvalue(L, 1);		
  lua_setfenv(L, -2);
  stackDump(L);
  /* easy table */
  lua_newtable(L);

  /* Use userdata as upvalue 1 */
  lua_pushvalue(L, -2);
  stackDump(L);
  luaI_openlib (L, NULL, luacurl_c, 1);

    /* create the setopt table */
  l_easy_setopt_newtable(L, privp);
  /* and assign to easy table */
  lua_setfield(L, -2, "setopt");  

  /* create the getinfo subtable */
  l_easy_getinfo_newtable(L, privp);
  /* and assign to the easy table */
  lua_setfield(L, -2, "getinfo");  

  /* return easy table */
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
  CURL *curl = LUACURL_PRIVATEP_UPVALUE(L, 1)->curl;
  const char *url = luaL_checklstring(L, 1, &inlength);
  char *rurl = curl_easy_unescape(curl, url, inlength, &outlength);
  lua_pushlstring(L, rurl, outlength);
  curl_free(rurl);
  return 1;
}

int l_tostring (lua_State *L) {
  CURL *curl = LUACURL_CHECKEASY(L);
  lua_pushfstring(L, "cURL (%p)", curl);
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
  CURL *curl = LUACURL_CHECKEASY(L);
  curl_easy_cleanup(curl);
  printf("Handle %p gone\n", curl);
  return 0;
}

/* registration hook function */
int luaopen_cURL(lua_State *L) {
  luaL_newmetatable(L, LUACURL_EASYMETATABLE);
  
  /* set easymetatable.__index = easymetatable */
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");

  /* register in easymetatable */
  luaL_register(L, NULL, luacurl_m);  

  luaL_register(L, "cURL", luacurl_f);
  return 1;
}

