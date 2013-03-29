project(Lua-cURL)

cmake_minimum_required(VERSION 2.8)

include (FindCURL)
set (SRC
        src/Lua-cURL.c
        src/Lua-cURL-share.c
        src/Lua-cURL-multi.c
        src/Lua-cURL-callback.c
        src/Lua-cURL-getinfo.c
        src/Lua-cURL-post.c
        src/Lua-cURL-setopt.c
        src/Lua-utility.c
)
set(HEADER
        src/Lua-utility.h
        src/Lua-cURL-share.h
        src/Lua-cURL.h
)
add_library(cURL SHARED ${SRC})
SET_TARGET_PROPERTIES( cURL PROPERTIES PREFIX "" )
target_link_libraries(cURL ${CURL_LIBRARY})
