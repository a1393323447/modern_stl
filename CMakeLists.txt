cmake_minimum_required(VERSION 3.14)
project(mstl VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DMSTL_DEBUG")  # Debug Mode

add_subdirectory(mini-stacktrace)

set(MSTL_ENABLE_STACKTRACE TRUE)  # enable stacktrace when panic

if ((CMAKE_SYSTEM_NAME MATCHES "Windows")
        AND NOT("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        AND ((NOT DEFINED MSTL_ENABLE_STACKTRACE_ON_WINDOWS_WITHOUT_MSVC) OR NOT ${MSTL_ENABLE_STACKTRACE_ON_WINDOWS_WITHOUT_MSVC}))
        set(MSTL_ENABLE_STACKTRACE FALSE)  # If the target OS is Windows and the compiler is not MSVC, disable stacktrace default
endif()

include(GNUInstallDirs)

add_library(mstl INTERFACE)

target_link_libraries(mstl INTERFACE mini_stacktrace)

target_compile_features(mstl INTERFACE
        cxx_std_20
        )

target_include_directories(
        mstl
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_definitions(
        mstl
        INTERFACE
        $<$<CONFIG:Debug>:MSTL_DEBUG>
        )

if (NOT ${MSTL_ENABLE_STACKTRACE})
        target_compile_definitions(mstl INTERFACE MINI_STACKTRACE_DISABLED)
endif()

install(TARGETS mstl
        EXPORT ${PROJECT_NAME}_Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

include(CMakePackageConfigHelpers)

write_basic_package_version_file("mstlConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)

configure_package_config_file(
        "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        INSTALL_DESTINATION
        ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

install(EXPORT ${PROJECT_NAME}_Targets
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/mstl DESTINATION include)

add_executable(modern_stl_test main.cpp)
target_link_libraries(modern_stl_test mstl)
enable_testing(true)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
set(CMAKE_CXX_FLAGS "-fconcepts-diagnostics-depth=10 -Wall -Wextra -Wno-attributes --coverage")
endif()

add_subdirectory(test)