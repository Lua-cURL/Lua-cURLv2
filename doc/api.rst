==================
Lua-cURL 
==================

:Author: Jürgen Hötzel
:Contact: http://www.hoetzel.info/
:Date: $Date: 2007/08/11 20:49:09 $
:Copyright: This document has been placed in the public domain.

.. contents::

Introduction
------------------

This project is not a fork of LuaCURL_ which is an direct mapping of parts of the libcurl-easy_ interface.

.. _libcurl-easy: http://curl.haxx.se/libcurl/c/libcurl-easy.html
.. _LuaCURL: http://luaforge.net/projects/luacurl/

The goal of **Lua-cURL** to make the :

* Easy Interface
* Multi Interface
* Shared Interface 

available while, doing it the Lua way (for example using iterators instead of callbacks) when possible.

Instead of a 1:1 mapping of all cURL functions, this implementation adapt the api to the functionality of Lua (like LuaExpat)

Installation
------------------

If you don't have pkgconfig configured, you have to specify compiler/linker flags::

    LUA_CFLAGS="-I/usr/local/include" LUA_LIBS="-L/usr/local/lib -lm -llua" ./configure  --with-cmoddir=/usr/lib/lua/5.1

Easy interface
------------------


Example 1: Fetch the example.com homepage
.........................................

..  include:: ../examples/file.lua  
    :literal:


Example 2: "On the fly" Fileupload
...................................

Register a read callback:

..  include:: ../examples/ftpupload.lua
    :literal:

Example 3: "Posting" data
.........................

.. include:: ../examples/post.lua
   :literal:



Differences
------------------

Differences in easy setopt
..........................
`setopt_proxytype` requires a string argument::

  c:setopt_proxytype("SOCKS4")

curl_version_info
..........................

Returns a table, containing version info and features/protocols subtable

..  include:: ../examples/version_info.lua
    :literal:


Multi interface
------------------


