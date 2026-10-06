cmake_minimum_required(VERSION 3.20)
if(NOT SB_EXECUTABLE OR NOT SB_TEST_ROOT)
    message(FATAL_ERROR "SB_EXECUTABLE and SB_TEST_ROOT are required")
endif()
string(RANDOM LENGTH 12 ALPHABET 0123456789abcdef run)
set(root "${SB_TEST_ROOT}/${run}")
file(MAKE_DIRECTORY "${root}")
set(workspace "${root}/Arbeitsordner ü")
set(config "${root}/settings.conf")
execute_process(COMMAND "${SB_EXECUTABLE}" --workspace "${workspace}"
    --settings "${config}" --snapshot "${root}/first.bmp"
    RESULT_VARIABLE result ERROR_VARIABLE error TIMEOUT 30)
if(NOT result EQUAL 0 OR NOT EXISTS "${config}")
    message(FATAL_ERROR "First process failed: ${result} ${error}")
endif()
file(READ "${config}" original)
# No explicit workspace: the second process must restore the saved location.
execute_process(COMMAND "${SB_EXECUTABLE}" --settings "${config}"
    --snapshot "${root}/second.bmp"
    RESULT_VARIABLE result ERROR_VARIABLE error TIMEOUT 30)
file(READ "${config}" restarted)
if(NOT result EQUAL 0 OR NOT original STREQUAL restarted)
    message(FATAL_ERROR "Restart lost settings: ${result} ${error}")
endif()
if(NOT EXISTS "${root}/first.bmp" OR NOT EXISTS "${root}/second.bmp")
    message(FATAL_ERROR "The two app processes did not render")
endif()
message(STATUS "Two application processes preserved their isolated settings")
