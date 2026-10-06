# Text shaping is a UI dependency; the domain targets never link it.
if(CMAKE_VERSION VERSION_LESS 3.24)
    message(FATAL_ERROR "The text UI requires CMake 3.24 or newer")
endif()
enable_language(CXX) # HarfBuzz only; all project-owned code remains C.
include(FetchContent)
FetchContent_Declare(sb_freetype URL https://codeload.github.com/libsdl-org/freetype/tar.gz/9973564cfa63763a3e4ac67c09147899539b1e07
    URL_HASH SHA256=026a05a49d114a1235d2926f4c03a9330e4b1a6efe7c217ec9607904c32907d4)
FetchContent_Declare(sb_harfbuzz URL https://codeload.github.com/libsdl-org/harfbuzz/tar.gz/564bf9818a18709776856533829c0c04950773d6
    URL_HASH SHA256=a448dd6c22d8e1e1cf39438c662251c1f97f810b8780eed4a6d6ada948c99ddc)
FetchContent_Declare(sb_sdl_ttf URL https://github.com/libsdl-org/SDL_ttf/releases/download/release-3.2.2/SDL3_ttf-3.2.2.tar.gz
    URL_HASH SHA256=63547d58d0185c833213885b635a2c0548201cc8f301e6587c0be1a67e1e045d)
# Populate first: SDL_ttf expects its pinned dependencies inside external/.
foreach(SB_TEXT_DEP IN ITEMS sb_freetype sb_harfbuzz sb_sdl_ttf)
    FetchContent_GetProperties(${SB_TEXT_DEP})
    if(NOT ${SB_TEXT_DEP}_POPULATED)
        FetchContent_Populate(${SB_TEXT_DEP})
    endif()
endforeach()
foreach(SB_TEXT_DEP IN ITEMS freetype harfbuzz)
    if(NOT EXISTS "${sb_sdl_ttf_SOURCE_DIR}/external/${SB_TEXT_DEP}/CMakeLists.txt")
        file(COPY "${sb_${SB_TEXT_DEP}_SOURCE_DIR}/" DESTINATION "${sb_sdl_ttf_SOURCE_DIR}/external/${SB_TEXT_DEP}")
    endif()
endforeach()
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(SDLTTF_VENDORED ON CACHE BOOL "" FORCE)
set(SDLTTF_STRICT ON CACHE BOOL "" FORCE)
set(SDLTTF_HARFBUZZ ON CACHE BOOL "" FORCE)
set(SDLTTF_PLUTOSVG OFF CACHE BOOL "" FORCE)
set(SDLTTF_INSTALL OFF CACHE BOOL "" FORCE)
set(SDLTTF_SAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory("${sb_sdl_ttf_SOURCE_DIR}" "${sb_sdl_ttf_BINARY_DIR}" EXCLUDE_FROM_ALL)
