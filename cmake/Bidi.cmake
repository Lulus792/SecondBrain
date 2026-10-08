# UI paragraph and script layout. Domain targets never link this library.
include(FetchContent)
file(SHA256 "${CMAKE_CURRENT_SOURCE_DIR}/app/script_data.inc" SB_SCRIPT_TABLE_SHA)
if(NOT SB_SCRIPT_TABLE_SHA STREQUAL "3ab5503b6c2fd04ca7fc386458f71203d1995f40e41f3c556ca2c5b96d863381")
    message(FATAL_ERROR "Unreviewed Unicode 18 script table")
endif()
FetchContent_Declare(sb_sheenbidi
    URL https://codeload.github.com/Tehreer/SheenBidi/tar.gz/refs/tags/v3.0.0
    URL_HASH SHA256=86c56014034739ba39a24c23eb00323b0bf6f737354f665786015fca842af786)
FetchContent_GetProperties(sb_sheenbidi)
if(NOT sb_sheenbidi_POPULATED)
    FetchContent_Populate(sb_sheenbidi)
endif()
# Separate preparation preserves the pinned upstream sources. Only the UBA
# classification and pairing data are updated; unused script/category APIs
# still describe the upstream Unicode 17 version and are not exposed.
set(SB_BIDI_PREPARED "${CMAKE_CURRENT_BINARY_DIR}/bidi-prepared")
file(COPY "${sb_sheenbidi_SOURCE_DIR}/Source" "${sb_sheenbidi_SOURCE_DIR}/Headers"
    DESTINATION "${SB_BIDI_PREPARED}")
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/third_party/ui/bidi18/manifest.json" SB_BIDI_MANIFEST)
string(JSON SB_BIDI_LICENSE_SHA GET "${SB_BIDI_MANIFEST}" runtime_license sha256)
file(SHA256 "${CMAKE_CURRENT_SOURCE_DIR}/third_party/licenses/SheenBidi.txt" SB_BIDI_LICENSE_ACTUAL)
if(NOT SB_BIDI_LICENSE_SHA STREQUAL SB_BIDI_LICENSE_ACTUAL)
    message(FATAL_ERROR "Unreviewed SheenBidi runtime license")
endif()
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/app/script_data.inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/licenses/SheenBidi.txt"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/ui/bidi18/manifest.json"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/ui/bidi18/BidiTypeLookup.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/ui/bidi18/PairingLookup.c")
foreach(SB_BIDI_FILE IN ITEMS BidiTypeLookup.c PairingLookup.c)
    set(SB_BIDI_DATA "${CMAKE_CURRENT_SOURCE_DIR}/third_party/ui/bidi18/${SB_BIDI_FILE}")
    string(JSON SB_BIDI_EXPECTED GET "${SB_BIDI_MANIFEST}" outputs "${SB_BIDI_FILE}")
    file(SHA256 "${SB_BIDI_DATA}" SB_BIDI_ACTUAL)
    if(NOT SB_BIDI_EXPECTED STREQUAL SB_BIDI_ACTUAL)
        message(FATAL_ERROR "Unreviewed Unicode 18 bidi table: ${SB_BIDI_FILE}")
    endif()
    configure_file("${SB_BIDI_DATA}" "${SB_BIDI_PREPARED}/Source/Data/${SB_BIDI_FILE}" COPYONLY)
endforeach()
add_library(sb_bidi_engine STATIC "${SB_BIDI_PREPARED}/Source/SheenBidi.c")
target_compile_definitions(sb_bidi_engine PRIVATE SB_CONFIG_UNITY)
target_include_directories(sb_bidi_engine PRIVATE "${SB_BIDI_PREPARED}/Headers" "${SB_BIDI_PREPARED}/Source")
add_library(sb_text_layout STATIC app/bidi.c app/scripts.c)
target_include_directories(sb_text_layout PUBLIC app PRIVATE "${SB_BIDI_PREPARED}/Headers")
target_link_libraries(sb_text_layout PUBLIC sb_core PRIVATE sb_bidi_engine)
if(MSVC)
    target_compile_options(sb_text_layout PRIVATE /W4 /utf-8 /we4133)
else()
    target_compile_options(sb_text_layout PRIVATE -Wall -Wextra -Wpedantic -Werror=incompatible-pointer-types)
endif()
