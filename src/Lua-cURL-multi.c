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

#include <stdlib.h>		/* malloc */
#include <sys/select.h>		/* select */
#include <string.h>		/* strerror */
#include <errno.h>

#include "Lua-cURL.h"
#include "Lua-utility.h"

/* REGISTRYINDEX[MULTIREGISTRY_KEY] = {
   { 1=type, 2=data, 3=EASY_HANDLE
   } 
   { 1=type, 2=data, 3=EASY_HANDLE
   }
}
 */

typedef struct l_multi_callbackdata {
  lua_State* L;
  l_easy_private *easyp;		/* corresponding easy handler */
  char *name;			/* type: header/write */
} l_multi_callbackdata;

typedef struct l_multi_private {
  CURLM *curlm;
  int last_remain;			/* remaining easy sockets */
  int n_easy;				/* number of easy handles */
} l_multi_private;


int l_multi_init(lua_State *L) {
  l_multi_private *privp;
  
  /* create userdata and assign metatable */
  privp = (l_multi_private *) lua_newuserdata(L, sizeof(l_multi_private));
  
  luaL_getmetatable(L, LUACURL_MULTIMETATABLE);
  lua_setmetatable(L, -2);

  privp->n_easy = 0;
  privp->last_remain = 1;		/* dummy: not null */
  if ((privp->curlm = curl_multi_init()) == NULL)
    return luaL_error(L, "something went wrong and you cannot use the other curl functions");

  /* return userdata */
  return 1;			
}

static int l_multi_internalcallback(void *ptr, size_t size, size_t nmemb, void *stream) {
  l_multi_callbackdata *callbackdata = (l_multi_callbackdata*) stream;
  /* append data */
  lua_State *L = callbackdata->L;

  /* table.insert(myregistrytable, {callbackdata}) */
  lua_getglobal(L, "table"); 
  lua_getfield(L, -1, "insert");
  /* remove table reference */
  lua_remove(L, -2);		
  lua_getfield(L, LUA_REGISTRYINDEX, MULTIREGISTRY_KEY); 

  /* create new table containing callbackdata */
  lua_newtable(L);		
  /* insert table entries */
  lua_pushlstring(L, ptr, size * nmemb);
  lua_rawseti(L, -2 , 1);	
  lua_pushstring(L, callbackdata->name);
  lua_rawseti(L, -2 , 2);
  
  lua_call(L, 2, 0);
  return nmemb*size;
}

l_multi_callbackdata* l_multi_create_callbackdata(lua_State *L, char *name, l_easy_private *easyp) {
  l_multi_callbackdata *callbackdata;

  lua_getfield(L, LUA_REGISTRYINDEX, MULTIREGISTRY_KEY);
  
  lua_newtable(L);		/* table containing callback-data easyhandle  */
  lua_rawseti(L, -2, (int) easyp);

  /* TODO: sanity check */
  /*   luaL_error(L, "callbackdata exists: %d, %s", easyp, name); */
  
  /* shrug! we need to garbage-collect this */
  callbackdata = (l_multi_callbackdata*) malloc(sizeof(l_multi_callbackdata));
  if (callbackdata == NULL)
    luaL_error(L, "can't malloc callbackdata");
  
  /* initialize */
  callbackdata->L = L;
  callbackdata->name = name;
  callbackdata->easyp = easyp;

  /* add to list of callbackdata */
  printf("Added to list off callbackdata");
  return callbackdata;
}

int l_multi_add_handle (lua_State *L) {
  l_multi_private *privatep = luaL_checkudata(L, 1, LUACURL_MULTIMETATABLE);  
  CURLM *curlm = privatep->curlm;
  CURLMcode rc;
  l_multi_callbackdata *data_callbackdata, *header_callbackdata;

  /* get easy userdata */
  l_easy_private *easyp = luaL_checkudata(L, 2, LUACURL_EASYMETATABLE);

  if ((rc = curl_multi_add_handle(curlm, easyp->curl)) != CURLM_OK)
    luaL_error(L, "cannot add handle: %s", curl_multi_strerror(rc));
  
  privatep->n_easy++;
  data_callbackdata = l_multi_create_callbackdata(L, "data", easyp);
  /* setup internal callback */
  if (curl_easy_setopt(easyp->curl, CURLOPT_WRITEDATA , data_callbackdata) != CURLE_OK)
    luaL_error(L, "%s", easyp->error);
  if (curl_easy_setopt(easyp->curl, CURLOPT_WRITEFUNCTION, l_multi_internalcallback) != CURLE_OK)
    luaL_error(L, "%s", easyp->error);

  /* shrug! we need to garbage-collect this */
  header_callbackdata = l_multi_create_callbackdata(L, "header", easyp);
  if (curl_easy_setopt(easyp->curl, CURLOPT_WRITEHEADER , header_callbackdata) != CURLE_OK)
    luaL_error(L, "%s", easyp->error);
  if (curl_easy_setopt(easyp->curl, CURLOPT_WRITEFUNCTION, l_multi_internalcallback) != CURLE_OK)
    luaL_error(L, "%s", easyp->error);
  return 0;
}

/* try to get data from internall callbackbuffer */
static int l_multi_perform_ingernal_getfrombuffer(lua_State *L) {
  /* table.remove(myregistrytable, 1) */
  lua_getglobal(L, "table");
  lua_getfield(L, -1, "remove");
  /* remove table reference */
  lua_remove(L, -2);		
  lua_getfield(L, LUA_REGISTRYINDEX, MULTIREGISTRY_KEY);
  lua_pushinteger(L, 1);
  lua_call(L, 2, 1);
  return 1;
}

static int l_multi_perform_internal (lua_State *L) {
  l_multi_private *privatep = LUACURL_PRIVATE_MULTIP_UPVALUE(L, 1);
  CURLM *curlm = privatep->curlm;
  CURLMcode rc;
  int remain;
  int n;

  l_multi_perform_ingernal_getfrombuffer(L);
  /* no data in buffer: try another perform */
  while (lua_isnil(L, -1)) {	
    lua_pop(L, -1);
    if (privatep->last_remain == 0) 
      return 0;			/* returns nil*/

   while ((rc = curl_multi_perform(curlm, &remain)) == CURLM_CALL_MULTI_PERFORM); /* loop */
    if (rc != CURLM_OK)
      luaL_error(L, "cannot perform: %s", curl_multi_strerror(rc));
    privatep->last_remain = remain;

    /* got data ? */
    l_multi_perform_ingernal_getfrombuffer(L);
    /* block for more data */
    if (lua_isnil(L, -1) && remain) {
      fd_set fdread;
      fd_set fdwrite;
      fd_set fdexcep;
      int maxfd;

      FD_ZERO(&fdread);
      FD_ZERO(&fdwrite);
      FD_ZERO(&fdexcep);
      
      if ((rc = curl_multi_fdset(curlm, &fdread, &fdwrite, &fdexcep, &maxfd)) != CURLM_OK)
	luaL_error(L, "curl_multi_fdset: %s", curl_multi_strerror(rc));
      
      
      if ((n = select(maxfd+1, &fdread, &fdwrite, &fdexcep, NULL)) < 0)
	luaL_error(L, "select: %s", strerror(errno));
      else 
	printf("Number of fds :%d\n", n);
    }
  }
  /* unpack table */
  
  n = lua_gettop(L);
  lua_rawgeti(L, n, 1);		/* data */
  lua_rawgeti(L, n, 2);		/* type */
  lua_remove(L, n);
  return 2;
}
/* return closure */
int l_multi_perform (lua_State *L) {
  luaL_checkudata(L, 1, LUACURL_MULTIMETATABLE);  
  lua_pushcclosure(L, l_multi_perform_internal, 1);
  return 1;
}

int l_multi_gc (lua_State *L) {
  l_multi_private *privatep = luaL_checkudata(L, 1, LUACURL_MULTIMETATABLE);  
  printf("Have to cleanup easyhandles: %d\n", privatep->n_easy);
  return 0;
}
