include(FetchContent)
include(ExternalProject)
find_program(SB_CARGO cargo REQUIRED)
execute_process(COMMAND "${SB_CARGO}" --version RESULT_VARIABLE cargo_result OUTPUT_VARIABLE cargo_version)
if(NOT cargo_result EQUAL 0)
    message(FATAL_ERROR "Linux accessibility UI build requires a working Cargo/Rust toolchain (Rust 1.87 or newer)")
endif()
FetchContent_Declare(sb_accesskit_c_source
    URL https://codeload.github.com/AccessKit/accesskit-c/tar.gz/8b6ed37c20ed4c59390e253407983333053662ba
    URL_HASH SHA256=f15581c841eed0f2f6cec6a6f9b7fd4ca9d34a654546efa22ae29351efa06568
    SOURCE_SUBDIR source-only)
FetchContent_Declare(sb_accesskit_unix_source
    URL https://static.crates.io/crates/accesskit_unix/accesskit_unix-0.24.0.crate
    URL_HASH SHA256=202f24df034a7476d07b7f74284de84f6d62aabd858dbe7ee9cad3b7ad6f8f9d
    SOURCE_SUBDIR source-only)
FetchContent_Declare(sb_accesskit_atspi_source
    URL https://static.crates.io/crates/accesskit_atspi_common/accesskit_atspi_common-0.21.0.crate
    URL_HASH SHA256=52c182f9c282ac9c5638d876d551d15e5f7d397ec263349a0c6a2b61595dd5e4
    SOURCE_SUBDIR source-only)
FetchContent_MakeAvailable(sb_accesskit_c_source sb_accesskit_unix_source sb_accesskit_atspi_source)
set(SB_C_SOURCE "${sb_accesskit_c_source_SOURCE_DIR}")
set(SB_UNIX_SOURCE "${sb_accesskit_unix_source_SOURCE_DIR}")
set(SB_ATSPI_SOURCE "${sb_accesskit_atspi_source_SOURCE_DIR}")
include("${CMAKE_CURRENT_LIST_DIR}/PrepareAccessKitLinux.cmake")
set(SB_ACCESSKIT_CARGO_TARGET "${CMAKE_CURRENT_BINARY_DIR}/accesskit-linux-build")
ExternalProject_Add(sb_accesskit_linux_build
    SOURCE_DIR "${SB_C_SOURCE}"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND "${CMAKE_COMMAND}" -E env CARGO_PROFILE_RELEASE_DEBUG=0
        "${SB_CARGO}" build --locked --release --manifest-path "${SB_C_SOURCE}/Cargo.toml" --target-dir "${SB_ACCESSKIT_CARGO_TARGET}"
    INSTALL_COMMAND ""
    BUILD_BYPRODUCTS "${SB_ACCESSKIT_CARGO_TARGET}/release/libaccesskit.a"
    LOG_BUILD ON LOG_OUTPUT_ON_FAILURE ON)
add_library(sb_accesskit_linux STATIC IMPORTED GLOBAL)
set_target_properties(sb_accesskit_linux PROPERTIES
    IMPORTED_LOCATION "${SB_ACCESSKIT_CARGO_TARGET}/release/libaccesskit.a"
    INTERFACE_INCLUDE_DIRECTORIES "${sb_accesskit_SOURCE_DIR}/include")
find_package(Threads REQUIRED)
target_link_libraries(sb_accesskit_linux INTERFACE Threads::Threads ${CMAKE_DL_LIBS} m rt util)
add_dependencies(sb_accesskit_linux sb_accesskit_linux_build)
set(SB_ACCESSKIT_TARGET sb_accesskit_linux)
