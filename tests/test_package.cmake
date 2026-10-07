cmake_minimum_required(VERSION 3.20)
if(NOT SB_ARCHIVE_ROOT OR NOT SB_TEST_ROOT)
    message(FATAL_ERROR "SB_ARCHIVE_ROOT and SB_TEST_ROOT are required")
endif()
file(GLOB SB_ARCHIVES "${SB_ARCHIVE_ROOT}/SecondBrain-*.tar.gz" "${SB_ARCHIVE_ROOT}/SecondBrain-*.zip")
list(LENGTH SB_ARCHIVES SB_ARCHIVE_COUNT)
if(NOT SB_ARCHIVE_COUNT EQUAL 1)
    message(FATAL_ERROR "Expected one application archive, found ${SB_ARCHIVE_COUNT}")
endif()
list(GET SB_ARCHIVES 0 SB_ARCHIVE)
get_filename_component(SB_ARCHIVE "${SB_ARCHIVE}" ABSOLUTE)
get_filename_component(SB_TEST_ROOT "${SB_TEST_ROOT}" ABSOLUTE)
set(SB_UNPACK "${SB_TEST_ROOT}/Paket ü")
file(MAKE_DIRECTORY "${SB_UNPACK}")
execute_process(COMMAND "${CMAKE_COMMAND}" -E tar xf "${SB_ARCHIVE}"
    WORKING_DIRECTORY "${SB_UNPACK}" RESULT_VARIABLE SB_UNPACK_RESULT)
if(NOT SB_UNPACK_RESULT EQUAL 0)
    message(FATAL_ERROR "Archive could not be extracted")
endif()
file(GLOB SB_ROOTS LIST_DIRECTORIES TRUE "${SB_UNPACK}/SecondBrain-*")
list(LENGTH SB_ROOTS SB_ROOT_COUNT)
if(NOT SB_ROOT_COUNT EQUAL 1)
    message(FATAL_ERROR "Expected one package directory")
endif()
list(GET SB_ROOTS 0 SB_ROOT)
if(APPLE)
    set(SB_EXECUTABLE "${SB_ROOT}/secondbrain.app/Contents/MacOS/secondbrain")
    set(SB_ASSETS "${SB_ROOT}/secondbrain.app/Contents/Resources/assets")
elseif(WIN32)
    set(SB_EXECUTABLE "${SB_ROOT}/secondbrain.exe")
    set(SB_ASSETS "${SB_ROOT}/assets")
else()
    set(SB_EXECUTABLE "${SB_ROOT}/secondbrain")
    set(SB_ASSETS "${SB_ROOT}/assets")
endif()
foreach(SB_RESOURCE IN ITEMS "${SB_EXECUTABLE}" "${SB_ASSETS}/fonts/NotoSans-Regular.ttf"
    "${SB_ASSETS}/fonts/NotoSansMono-Regular.ttf"
    "${SB_ASSETS}/fonts/NotoSansArabic-Regular.ttf" "${SB_ASSETS}/fonts/NotoSansHebrew-Regular.ttf"
    "${SB_ASSETS}/fonts/NotoSansDevanagari-Regular.ttf" "${SB_ASSETS}/fonts/NotoSansSymbols2-Regular.ttf"
    "${SB_ASSETS}/fonts/NotoSansCJKjp-Regular.otf" "${SB_ROOT}/licenses/Noto-CJK.txt"
    "${SB_ROOT}/licenses/SDL_ttf.txt" "${SB_ROOT}/licenses/FreeType-LICENSE.txt"
    "${SB_ROOT}/licenses/FreeType-FTL.txt" "${SB_ROOT}/licenses/HarfBuzz.txt"
    "${SB_ASSETS}/licenses/Unicode.txt" "${SB_ASSETS}/licenses/LICENSE" "${SB_ASSETS}/licenses/Nuklear-LICENSE"
    "${SB_ASSETS}/licenses/SDL_ttf.txt" "${SB_ASSETS}/licenses/FreeType-FTL.txt"
    "${SB_ASSETS}/licenses/HarfBuzz-MS-USE.txt" "${SB_ASSETS}/licenses/OFL-CJK.txt" "${SB_ROOT}/licenses/SDL3.txt"
    "${SB_ROOT}/licenses/Nuklear.txt" "${SB_ROOT}/licenses/Noto.txt" "${SB_ROOT}/QUICKSTART.txt" "${SB_ROOT}/LICENSE")
    if(NOT EXISTS "${SB_RESOURCE}")
        message(FATAL_ERROR "Missing package resource: ${SB_RESOURCE}")
    endif()
endforeach()
# Identify both shipped programs before any GUI or workspace is created.
get_filename_component(SB_ARCHIVE_NAME "${SB_ARCHIVE}" NAME)
if(NOT SB_ARCHIVE_NAME MATCHES "^SecondBrain-([0-9]+\\.[0-9]+\\.[0-9]+)-")
    message(FATAL_ERROR "Package has no recognizable version")
endif()
set(SB_PACKAGED_VERSION "${CMAKE_MATCH_1}")
if(WIN32)
    set(SB_CLI "${SB_ROOT}/secondbrain-cli.exe")
else()
    set(SB_CLI "${SB_ROOT}/secondbrain-cli")
endif()
foreach(SB_PROGRAM IN ITEMS "${SB_EXECUTABLE}" "${SB_CLI}")
    execute_process(COMMAND "${SB_PROGRAM}" --version WORKING_DIRECTORY "${SB_UNPACK}"
        RESULT_VARIABLE SB_VERSION_RESULT OUTPUT_VARIABLE SB_VERSION_OUTPUT ERROR_VARIABLE SB_VERSION_ERROR TIMEOUT 15)
    string(REPLACE "\r\n" "\n" SB_VERSION_OUTPUT "${SB_VERSION_OUTPUT}")
    string(FIND "${SB_VERSION_OUTPUT}" "SecondBrain ${SB_PACKAGED_VERSION}\nBuild: " SB_VERSION_PREFIX)
    if(NOT SB_VERSION_RESULT EQUAL 0 OR NOT SB_VERSION_PREFIX EQUAL 0 OR NOT SB_VERSION_ERROR STREQUAL "")
        message(FATAL_ERROR "Packaged version mismatch: ${SB_PROGRAM}\n${SB_VERSION_OUTPUT}\n${SB_VERSION_ERROR}")
    endif()
    if(SB_PACKAGED_INFO AND NOT SB_PACKAGED_INFO STREQUAL SB_VERSION_OUTPUT)
        message(FATAL_ERROR "Desktop and CLI originate from different builds")
    endif()
    set(SB_PACKAGED_INFO "${SB_VERSION_OUTPUT}")
endforeach()
if(APPLE)
    file(READ "${SB_ROOT}/secondbrain.app/Contents/Info.plist" SB_BUNDLE_INFO)
    foreach(SB_VERSION_KEY IN ITEMS CFBundleShortVersionString CFBundleVersion)
        if(NOT SB_BUNDLE_INFO MATCHES "<key>${SB_VERSION_KEY}</key>[ \n\r\t]*<string>([^<]*)</string>" OR NOT CMAKE_MATCH_1 STREQUAL SB_PACKAGED_VERSION)
            message(FATAL_ERROR "Bundle version differs from packaged executable: ${SB_VERSION_KEY}")
        endif()
    endforeach()
endif()

# Run the extracted app with its bundled assets from a different directory.
execute_process(COMMAND "${SB_EXECUTABLE}" --self-test "${SB_TEST_ROOT}/Bedienprüfung ü"
    WORKING_DIRECTORY "${SB_UNPACK}" RESULT_VARIABLE SB_RESULT
    OUTPUT_VARIABLE SB_OUTPUT ERROR_VARIABLE SB_ERROR TIMEOUT 240)
message(STATUS "${SB_OUTPUT}")
if(NOT SB_RESULT EQUAL 0)
    message(FATAL_ERROR "Relocated application failed: ${SB_RESULT}\n${SB_ERROR}")
endif()
message(STATUS "Extracted package passed the desktop workflow: ${SB_ARCHIVE}")

execute_process(COMMAND "${SB_EXECUTABLE}" --keyboard-test "${SB_TEST_ROOT}/Tastaturprüfung ü"
    WORKING_DIRECTORY "${SB_UNPACK}" RESULT_VARIABLE SB_KEYBOARD_RESULT
    OUTPUT_VARIABLE SB_KEYBOARD_OUTPUT ERROR_VARIABLE SB_KEYBOARD_ERROR TIMEOUT 300)
message(STATUS "${SB_KEYBOARD_OUTPUT}")
if(NOT SB_KEYBOARD_RESULT EQUAL 0)
    message(FATAL_ERROR "Relocated keyboard workflow failed: ${SB_KEYBOARD_RESULT}\n${SB_KEYBOARD_ERROR}")
endif()
message(STATUS "Extracted package passed the keyboard-only workflow")

execute_process(COMMAND "${SB_EXECUTABLE}" --backup-test "${SB_TEST_ROOT}/Sicherungsprüfung ü"
    WORKING_DIRECTORY "${SB_UNPACK}" RESULT_VARIABLE SB_BACKUP_RESULT
    OUTPUT_VARIABLE SB_BACKUP_OUTPUT ERROR_VARIABLE SB_BACKUP_ERROR TIMEOUT 240)
message(STATUS "${SB_BACKUP_OUTPUT}")
if(NOT SB_BACKUP_RESULT EQUAL 0)
    message(FATAL_ERROR "Relocated backup workflow failed: ${SB_BACKUP_RESULT}\n${SB_BACKUP_ERROR}")
endif()

execute_process(COMMAND "${CMAKE_COMMAND}" "-DSB_EXECUTABLE=${SB_EXECUTABLE}"
    "-DSB_TEST_ROOT=${SB_TEST_ROOT}/Einstellungen ü"
    -P "${CMAKE_CURRENT_LIST_DIR}/test_preferences_process.cmake"
    RESULT_VARIABLE SB_PREFERENCES_RESULT OUTPUT_VARIABLE SB_PREFERENCES_OUTPUT
    ERROR_VARIABLE SB_PREFERENCES_ERROR TIMEOUT 90)
message(STATUS "${SB_PREFERENCES_OUTPUT}")
if(NOT SB_PREFERENCES_RESULT EQUAL 0)
    message(FATAL_ERROR "Relocated preferences restart failed: ${SB_PREFERENCES_ERROR}")
endif()

if(WIN32)
    set(SB_CLI "${SB_ROOT}/secondbrain-cli.exe")
else()
    set(SB_CLI "${SB_ROOT}/secondbrain-cli")
endif()
execute_process(COMMAND "${CMAKE_COMMAND}" "-DSB_CLI=${SB_CLI}"
    "-DSB_TEST_ROOT=${SB_TEST_ROOT}/Sicherungswerkzeug ü"
    -P "${CMAKE_CURRENT_LIST_DIR}/test_backup_cli.cmake"
    RESULT_VARIABLE SB_CLI_RESULT ERROR_VARIABLE SB_CLI_ERROR TIMEOUT 120)
if(NOT SB_CLI_RESULT EQUAL 0)
    message(FATAL_ERROR "Relocated backup CLI failed: ${SB_CLI_ERROR}")
endif()
