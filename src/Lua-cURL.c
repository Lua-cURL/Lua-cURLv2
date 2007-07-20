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


/* closures assigned to setopt in setopt table */
static struct {
  const char *name;
  CURLoption option; 
  lua_CFunction func;
} luacurl_setopt_c[] = {
  /* behavior options */
  {"verbose", CURLOPT_VERBOSE, l_easy_opt_long},
  {"header", CURLOPT_HEADER, l_easy_opt_long},
  {"noprogrss", CURLOPT_NOPROGRESS, l_easy_opt_long},
  {"nosignal", CURLOPT_NOSIGNAL, l_easy_opt_long},
  /* callback options */
  /* network options */
  {"url", CURLOPT_URL, l_easy_opt_string},
  /* dummy opt value */
  {NULL, CURLOPT_VERBOSE, NULL}};	

/* closures assigned to getinfo in setopt table */
static struct {
  const char *name;
  CURLINFO info; 
  lua_CFunction func;
} luacurl_getinfo_c[] = {
  {"effective_url", CURLINFO_EFFECTIVE_URL, l_easy_getinfo_string},
  {"response_code", CURLINFO_RESPONSE_CODE, l_easy_getinfo_long},
  {"http_connectcode", CURLINFO_HTTP_CONNECTCODE, l_easy_getinfo_long},
  {"filetime", CURLINFO_FILETIME, l_easy_getinfo_long},
  {"total_time", CURLINFO_TOTAL_TIME, l_easy_getinfo_double},
  {"namelookup_time", CURLINFO_NAMELOOKUP_TIME, l_easy_getinfo_double},
  {"connect_time", CURLINFO_CONNECT_TIME, l_easy_getinfo_double},
  {"pretransfer", CURLINFO_PRETRANSFER_TIME, l_easy_getinfo_double},
  {"starttransfer_time", CURLINFO_STARTTRANSFER_TIME, l_easy_getinfo_double},  
  {"redirect_time", CURLINFO_REDIRECT_TIME, l_easy_getinfo_double},  
  {"redirect_count", CURLINFO_REDIRECT_COUNT, l_easy_getinfo_long},  
  {"size_upload", CURLINFO_SIZE_UPLOAD, l_easy_getinfo_double},
  {"size_download", CURLINFO_SIZE_DOWNLOAD, l_easy_getinfo_double},
  {"speed_download", CURLINFO_SPEED_DOWNLOAD, l_easy_getinfo_double},
  {"speed_upload", CURLINFO_SPEED_UPLOAD, l_easy_getinfo_double},
  {"header_size", CURLINFO_HEADER_SIZE, l_easy_getinfo_long},
  {"request_size", CURLINFO_REQUEST_SIZE, l_easy_getinfo_long},
  {"ssl_verifyresult", CURLINFO_SSL_VERIFYRESULT, l_easy_getinfo_long},
  {"ssl_engines", CURLINFO_SSL_ENGINES, l_easy_getinfo_curl_slist},
  {"content_length_download", CURLINFO_CONTENT_LENGTH_DOWNLOAD, l_easy_getinfo_double},
  {"content_length_upload", CURLINFO_CONTENT_LENGTH_UPLOAD, l_easy_getinfo_double},
  {"content_type", CURLINFO_CONTENT_TYPE, l_easy_getinfo_string},
  {"private", CURLINFO_PRIVATE, l_easy_getinfo_string},
  {"httpauth_avail", CURLINFO_HTTPAUTH_AVAIL, l_easy_getinfo_long},
  {"proxyauth_avail", CURLINFO_PROXYAUTH_AVAIL, l_easy_getinfo_long},
  {"os_errno", CURLINFO_OS_ERRNO, l_easy_getinfo_long},
  {"num_connects", CURLINFO_NUM_CONNECTS, l_easy_getinfo_long},
  {"cookielist", CURLINFO_COOKIELIST, l_easy_getinfo_curl_slist},
  {"lastsocket", CURLINFO_LASTSOCKET, l_easy_getinfo_long},
  {"ftp_entry_path" , CURLINFO_FTP_ENTRY_PATH , l_easy_getinfo_string},  
  {NULL, CURLINFO_EFFECTIVE_URL, NULL}};	

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
  {"__tostring", l_tostring},
  {"__gc", l_easy_gc},
  {"__newindex", l_setopt},
  {"__index", l_getopt},
  {NULL, NULL}};


int l_easy_escape(lua_State *L) {
  int length = 0;
  CURL *curl = LUACURL_PRIVATEP_UPVALUE(L, 1)->curl;
  const char *url = luaL_checklstring(L, 1, &length);
  char *rurl = curl_easy_escape(curl, url, length);
  lua_pushstring(L, rurl);
  curl_free(rurl);
  return 1;
}


int l_easy_perform(lua_State *L) {
  CURL *curl = LUACURL_PRIVATEP_UPVALUE(L, 1)->curl;
  printf("CurlP: %p\n", curl);
  if (curl_easy_perform(curl) != CURLE_OK) 
    luaL_error(L, "%s", LUACURL_PRIVATEP_UPVALUE(L, 1)->error);
  return 0;
}

int l_easy_init(lua_State *L) {
  int i;
  l_private *privp = malloc(sizeof(l_private)); 
  
  if ((privp->curl = curl_easy_init()) == NULL)
    return luaL_error(L, "something went wrong and you cannot use the other curl functions");

  /* set error buffer */
  if (curl_easy_setopt(privp->curl, CURLOPT_ERRORBUFFER, privp->error) != CURLE_OK)
    return luaL_error(L, "cannot set error buffer");

  /* easy table (reverences setopt table) */
  lua_newtable(L);

  /* create closures using private data as upvalue */
  lua_pushlightuserdata(L, privp); 
  luaI_openlib (L, NULL, luacurl_c, 1);

  /* setopt subtable */
  lua_newtable(L);		
  
  /* assign setopt closures to setopt subtable */
  for (i=0; luacurl_setopt_c[i].name != NULL; i++) {
    CURLoption *optionp = &(luacurl_setopt_c[i].option);
    lua_pushlightuserdata(L, privp); 
    lua_pushlightuserdata(L, optionp);
    lua_pushcclosure(L, luacurl_setopt_c[i].func, 2);
    lua_setfield(L, -2, luacurl_setopt_c[i].name);
  }

  /* assign setopt table to easy table */
  lua_setfield(L, -2, "setopt");  




  /* getinfo subtable */
  lua_newtable(L);		

  /* assign getinfo closures to getinfo subtable */
  for (i=0; luacurl_getinfo_c[i].name != NULL; i++) {
    CURLINFO *infop = &(luacurl_getinfo_c[i].info);
    lua_pushlightuserdata(L, privp); 
    lua_pushlightuserdata(L, infop);
    lua_pushcclosure(L, luacurl_getinfo_c[i].func, 2);
    lua_setfield(L, -2, luacurl_getinfo_c[i].name);
  }  
  /* assign getinfo table to easy table */
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
  int inlength = 0;
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
  int length;
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


int l_easy_getinfo_string(lua_State *L) {
  CURL *curl = LUACURL_PRIVATEP_UPVALUE(L, 1)->curl;  
  CURLINFO *infop = LUACURL_INFOP_UPVALUE(L, 2);
  char *value;

  if (curl_easy_getinfo(curl, *infop, &value) != CURLE_OK)  
    luaL_error(L, "%s", LUACURL_PRIVATEP_UPVALUE(L, 1)->error);
  
  lua_pushstring(L, value);
  return 1;
}

int l_easy_getinfo_long(lua_State *L) {
  CURL *curl = LUACURL_PRIVATEP_UPVALUE(L, 1)->curl;  
  CURLINFO *infop = LUACURL_INFOP_UPVALUE(L, 2);
  long value;

  if (curl_easy_getinfo(curl, *infop, &value) != CURLE_OK)  
    luaL_error(L, "%s", LUACURL_PRIVATEP_UPVALUE(L, 1)->error);
  
  lua_pushinteger(L, value);
  return 1;
} 

int l_easy_getinfo_double(lua_State *L) {
  CURL *curl = LUACURL_PRIVATEP_UPVALUE(L, 1)->curl;  
  CURLINFO *infop = LUACURL_INFOP_UPVALUE(L, 2);
  double value;

  if (curl_easy_getinfo(curl, *infop, &value) != CURLE_OK)  
    luaL_error(L, "%s", LUACURL_PRIVATEP_UPVALUE(L, 1)->error);
  
  lua_pushnumber(L, value);
  return 1;
} 

int l_easy_getinfo_curl_slist(lua_State *L) {
  CURL *curl = LUACURL_PRIVATEP_UPVALUE(L, 1)->curl;  
  CURLINFO *infop = LUACURL_INFOP_UPVALUE(L, 2);  
  struct curl_slist *list;
  struct curl_slist *next;
  int i;
  if (curl_easy_getinfo(curl, *infop, &list) != CURLE_OK)
    luaL_error(L, "%s", LUACURL_PRIVATEP_UPVALUE(L, 1)->error);
  
  i = 1;
  next = list;
  lua_newtable(L);		

  while (next) {
    lua_pushstring(L, (char*) next->data);
    lua_rawseti(L, -2, i++);
    next = next->next;
  }

  curl_slist_free_all(list);

  return 1;
}

int l_easy_opt_long(lua_State *L) {
  CURL *curl = LUACURL_PRIVATEP_UPVALUE(L, 1)->curl;
  CURLoption *optionp = LUACURL_OPTIONP_UPVALUE(L, 2);
  long value = luaL_checklong(L,1);
  int rc = curl_easy_setopt(curl, *optionp, value);
  /* TODO Error Handling */
  lua_pushboolean(L, rc == CURLE_OK);
  return 1;
}

l_easy_opt_string(lua_State *L) {
  CURL *curl = LUACURL_PRIVATEP_UPVALUE(L, 1)->curl;
  CURLoption *optionp = LUACURL_OPTIONP_UPVALUE(L, 2);  
  const char *value = luaL_checkstring(L, 1);
  int rc = curl_easy_setopt(curl, *optionp, value);
  lua_pushboolean(L, rc == CURLE_OK);
  printf("CurlP: %p\n", curl);
  return 1;
}

int l_easy_gc(lua_State *L) {
  CURL *curl = LUACURL_CHECKEASY(L);
  curl_easy_cleanup(curl);
  printf("Handle %x gone\n", curl);
  return 0;
}
int l_setopt(lua_State *L) {
  CURL *curl = LUACURL_CHECKEASY(L);
  const char *url = luaL_checkstring(L, 2);
  printf("Trying to set %s\n", url);
  return 0;
}

int l_getopt(lua_State *L) {
  CURL *curl = LUACURL_CHECKEASY(L);
  const char *url = luaL_checkstring(L, 2);
  printf("Trying to get %s\n", url);
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

