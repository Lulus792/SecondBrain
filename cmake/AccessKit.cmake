include(FetchContent)
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()
FetchContent_Declare(sb_accesskit
    URL https://github.com/AccessKit/accesskit-c/releases/download/0.23.1/accesskit-c-0.23.1.zip
    URL_HASH SHA256=35b7ca8a6f1e038b5da35e1e9e5a0adaed9bfcf21e1496d29598fbbadcc7043f
    SOURCE_SUBDIR prebuilt-only)
FetchContent_MakeAvailable(sb_accesskit)
include("${sb_accesskit_SOURCE_DIR}/accesskit-config.cmake")
# The released Windows static library uses a different CRT from this application.
# Keep allocators on their respective side of the C ABI and package the UI DLL.
if(WIN32)
    set(SB_ACCESSKIT_TARGET accesskit-shared)
else()
    set(SB_ACCESSKIT_TARGET accesskit-static)
endif()
