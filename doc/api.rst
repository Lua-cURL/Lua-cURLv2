==================
Lua-cURL 
==================

:Author: Jürgen Hötzel
:Contact: http://www.hoetzel.info/
:Date: $Date: 2007/08/08 22:15:49 $
:Copyright: This document has been placed in the public domain.

.. contents::

Introduction
------------------
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


Differencesf
------------------

Differences in easy setopt

`setopt_proxytype` requires a string argument::

  c:setopt_proxytype("SOCKS4")



Multi interface
------------------

