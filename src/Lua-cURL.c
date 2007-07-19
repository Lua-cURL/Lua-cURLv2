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

/* closures assigned to setopt in easy table */
static struct luaL_Reg_Setopt luacurl_opt_c[] = {
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

/* functions in module namespace*/
static const struct luaL_Reg luacurl_f[] = {
  {"easy_init", l_easy_init},
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
  luaL_Reg_Setopt *setopts;
  l_private *privp = malloc(sizeof(l_private)); 
  
  if ((privp->curl = curl_easy_init()) == NULL)
    return luaL_error(L, "something went wrong and you cannot use the other curl functions");

  /* set error buffer */
  if (curl_easy_setopt(privp->curl, CURLOPT_ERRORBUFFER, privp->error) != CURLE_OK)
    return luaL_error(L, "cannot set error buffer");

  lua_newtable(L);

  /* create closures using private data as upvalue */
  lua_pushlightuserdata(L, privp); 
  luaI_openlib (L, NULL, luacurl_c, 1);

  
  for (setopts = luacurl_opt_c; setopts->name; setopts++) {
    CURLoption *optionp = &(setopts->option);
    lua_pushlightuserdata(L, privp); 
    lua_pushlightuserdata(L, optionp);
    lua_pushcclosure(L, setopts->func, 2);
    lua_setfield(L, -2, setopts->name);
  }
  /* return table */
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


/* just test if called with easy hander */ 
int l_easy_opt_long(lua_State *L) {
  CURL *curl = LUACURL_PRIVATEP_UPVALUE(L, 1)->curl;
  CURLoption *optionp = LUACURL_OPTIONP_UPVALUE(L, 2);
  long value = luaL_checklong(L,1);
  int rc = curl_easy_setopt(curl, *optionp, value);
  lua_pushboolean(L, rc == CURLE_OK);
  return 1;
}

l_easy_opt_string(lua_State *L) {
  CURL *curl = LUACURL_PRIVATEP_UPVALUE(L, 1)->curl;
  CURLoption *optionp = LUACURL_OPTIONP_UPVALUE(L, 2);  
  const char *value = luaL_checkstring(L,1);
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

