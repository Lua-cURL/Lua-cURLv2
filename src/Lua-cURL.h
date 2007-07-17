#ifndef LUACURL_H
#define LUACURL_H

/* lua specific */
#include <lua.h>
#include <lauxlib.h>

/* curl specific */
#include <curl/curl.h>
#include <curl/easy.h>

/* custom metatables */
#define LUACURL_EASYMETATABLE "CURL.easy"
#define LUACURL_OPTTABLE "option" 

/* custom macros */
#define LUACURL_CHECKEASY(L) (CURL *) luaL_checkudata(L, 1, LUACURL_EASYMETATABLE)
#define LUACURL_UPVALUE(L) (CURL *) lua_touserdata(L, lua_upvalueindex(1));

/* Lua closures (CURL* upvalue) */
int l_tostring (lua_State *L);
int l_easy_dummy (lua_State *L);
int l_easy_escape (lua_State *L);
int l_easy_init (lua_State *L);
int l_easy_unescape (lua_State *L);

/* Lua module functions */
int l_easy_init (lua_State *L);
int l_unescape (lua_State *L);
int l_version (lua_State *L);
int l_version_info (lua_State *L);

/* Lua metatable functions */
int l_tostring (lua_State *L);
int l_easy_gc (lua_State *L);
int l_setopt(lua_State *L);
int l_getopt(lua_State *L);

#endif
