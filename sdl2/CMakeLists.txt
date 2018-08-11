include_directories(${DEVKITPRO}/portlibs/switch/include/SDL2)
set(BASE_SRC pdcclip.c
        pdcdisp.c
        pdcgetsc.c
        pdckbd.c
        pdcscrn.c
        pdcsetsc.c
        pdcutil.c
        vkeyboard/int10font14.c
        vkeyboard/vkeykb.cpp
        )

add_library(pdcurseSDL STATIC ${BASE_SRC})

add_executable(sdltest sdltest.c)
target_link_libraries(sdltest pdbase)