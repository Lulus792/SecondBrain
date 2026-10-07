include(FetchContent)
include(ExternalProject)
find_program(SB_CARGO cargo REQUIRED)
FetchContent_Declare(sb_accesskit_c_source
    URL https://codeload.github.com/AccessKit/accesskit-c/tar.gz/8b6ed37c20ed4c59390e253407983333053662ba
    URL_HASH SHA256=f15581c841eed0f2f6cec6a6f9b7fd4ca9d34a654546efa22ae29351efa06568
    SOURCE_SUBDIR source-only)
FetchContent_Declare(sb_accesskit_windows_source
    URL https://static.crates.io/crates/accesskit_windows/accesskit_windows-0.35.1.crate
    URL_HASH SHA256=ce63f35d6bdcf59f26b76b3379063f738e6412cef46999cc772d46aa3de35adb
    SOURCE_SUBDIR source-only)
FetchContent_MakeAvailable(sb_accesskit_c_source sb_accesskit_windows_source)
set(SB_C_SOURCE "${sb_accesskit_c_source_SOURCE_DIR}")
set(SB_WINDOWS_SOURCE "${sb_accesskit_windows_source_SOURCE_DIR}")
include("${CMAKE_CURRENT_LIST_DIR}/PrepareAccessKitWindows.cmake")
set(SB_ACCESSKIT_CARGO_TARGET "${CMAKE_CURRENT_BINARY_DIR}/accesskit-windows-build")
set(SB_ACCESSKIT_CARGO_ARGS "")
set(SB_ACCESSKIT_OUTPUT "${SB_ACCESSKIT_CARGO_TARGET}/release")
if(MINGW)
    set(SB_ACCESSKIT_CARGO_ARGS --target x86_64-pc-windows-gnu)
    set(SB_ACCESSKIT_OUTPUT "${SB_ACCESSKIT_CARGO_TARGET}/x86_64-pc-windows-gnu/release")
    set(SB_ACCESSKIT_IMPLIB "${SB_ACCESSKIT_OUTPUT}/libaccesskit.dll.a")
else()
    set(SB_ACCESSKIT_IMPLIB "${SB_ACCESSKIT_OUTPUT}/accesskit.dll.lib")
endif()
ExternalProject_Add(sb_accesskit_windows_build
    SOURCE_DIR "${SB_C_SOURCE}"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND "${CMAKE_COMMAND}" -E env CARGO_PROFILE_RELEASE_DEBUG=0
        "${SB_CARGO}" build --locked --release ${SB_ACCESSKIT_CARGO_ARGS}
        --manifest-path "${SB_C_SOURCE}/Cargo.toml" --target-dir "${SB_ACCESSKIT_CARGO_TARGET}"
    INSTALL_COMMAND ""
    BUILD_BYPRODUCTS "${SB_ACCESSKIT_OUTPUT}/accesskit.dll" "${SB_ACCESSKIT_IMPLIB}"
    LOG_BUILD ON LOG_OUTPUT_ON_FAILURE ON)
add_library(sb_accesskit_windows SHARED IMPORTED GLOBAL)
set_target_properties(sb_accesskit_windows PROPERTIES
    IMPORTED_LOCATION "${SB_ACCESSKIT_OUTPUT}/accesskit.dll"
    IMPORTED_IMPLIB "${SB_ACCESSKIT_IMPLIB}"
    INTERFACE_INCLUDE_DIRECTORIES "${sb_accesskit_SOURCE_DIR}/include")
add_dependencies(sb_accesskit_windows sb_accesskit_windows_build)
set(SB_ACCESSKIT_TARGET sb_accesskit_windows)
