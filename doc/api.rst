==================
Lua-cURL 
==================

:Author: Jürgen Hötzel
:Contact: http://www.hoetzel.info/
:Date: $Date: 2007/08/08 06:30:55 $
:Copyright: This document has been placed in the public domain.

.. contents::

Introduction
------------------
Instead of a 1:1 mapping of all cURL functions, this implementation adapt the api to the functionality of Lua (like LuaExpat)

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

