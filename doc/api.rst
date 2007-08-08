==================
Lua-cURL 
==================

:Author: Jürgen Hötzel
:Contact: http://www.hoetzel.info/
:Date: $Date: 2007/08/08 06:20:09 $
:Copyright: This document has been placed in the public domain.

.. contents::

Introduction
------------------
Instead of a 1:1 mapping of all cURL functions, this implementation adapt the api to the functionality of Lua (like LuaExpat)

Easy interface
------------------

..  include:: ../examples/file.lua  
    :literal:


A simple file "On the fly" Fileupload

..  include:: ../examples/ftpupload.lua
    :literal:


Differences
------------------

Differences in easy setopt

`setopt_proxytype` requires a string argument::

  c:setopt_proxytype("SOCKS4")



Multi interface
------------------

