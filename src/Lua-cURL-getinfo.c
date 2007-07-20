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

/* closures assigned to getinfo in easy table */

static int l_easy_getinfo_string(lua_State *L) {
  CURL *curl = LUACURL_PRIVATEP_UPVALUE(L, 1)->curl;  
  CURLINFO *infop = LUACURL_INFOP_UPVALUE(L, 2);
  char *value;

  if (curl_easy_getinfo(curl, *infop, &value) != CURLE_OK)  
    luaL_error(L, "%s", LUACURL_PRIVATEP_UPVALUE(L, 1)->error);
  
  lua_pushstring(L, value);
  return 1;
}

static int l_easy_getinfo_long(lua_State *L) {
  CURL *curl = LUACURL_PRIVATEP_UPVALUE(L, 1)->curl;  
  CURLINFO *infop = LUACURL_INFOP_UPVALUE(L, 2);
  long value;

  if (curl_easy_getinfo(curl, *infop, &value) != CURLE_OK)  
    luaL_error(L, "%s", LUACURL_PRIVATEP_UPVALUE(L, 1)->error);
  
  lua_pushinteger(L, value);
  return 1;
} 

static int l_easy_getinfo_double(lua_State *L) {
  CURL *curl = LUACURL_PRIVATEP_UPVALUE(L, 1)->curl;  
  CURLINFO *infop = LUACURL_INFOP_UPVALUE(L, 2);
  double value;

  if (curl_easy_getinfo(curl, *infop, &value) != CURLE_OK)  
    luaL_error(L, "%s", LUACURL_PRIVATEP_UPVALUE(L, 1)->error);
  
  lua_pushnumber(L, value);
  return 1;
} 

static int l_easy_getinfo_curl_slist(lua_State *L) {
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


/* create getinfo subtable */
int l_easy_getinfo_newtable(lua_State *L, l_private *privp) {
  int i;

  lua_newtable(L);		
  /* assign getinfo closures to getinfo subtable */
  for (i=0; luacurl_getinfo_c[i].name != NULL; i++) {
    CURLINFO *infop = &(luacurl_getinfo_c[i].info);
    lua_pushlightuserdata(L, privp); 
    lua_pushlightuserdata(L, infop);
    lua_pushcclosure(L, luacurl_getinfo_c[i].func, 2);
    lua_setfield(L, -2, luacurl_getinfo_c[i].name);
  }  

  /* getinfo table is on top of the stack */
  return 1;
}
