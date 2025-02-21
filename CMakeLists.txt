# ------------------------------------------------------------------------------
# Specify the C and C++ compilers to be used.
# Clang is explicitly set as the compiler for both C and C++.
# ------------------------------------------------------------------------------
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER /usr/bin/clang++)

# ------------------------------------------------------------------------------
# CMake minimum version requirement
# ------------------------------------------------------------------------------
# We specify a minimum required version of CMake to ensure that the features 
# we use are available.
cmake_minimum_required(VERSION 3.13)

# ------------------------------------------------------------------------------
# Define the project name
# ------------------------------------------------------------------------------
project(LoAT)

# ------------------------------------------------------------------------------
# Set the C++ standard to C++20
# ------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD 20)

# ------------------------------------------------------------------------------
# Define build options
# ------------------------------------------------------------------------------
# BUILD_SHARED_LIBS: Build shared lib (default ON)
# BUILD_STATIC_LIBS: Build static lib (default ON)
# BUILD_EXECUTABLE: Build the executable (default ON)
# STATIC: Whether to build a static binary - if we build an executable (default ON).
# SANITIZE: Whether to enable sanitizers for debugging (default OFF).
# ------------------------------------------------------------------------------
option(BUILD_SHARED_LIBRARY "Build shared libraries" ON)
option(BUILD_STATIC_LIBRARY "Build static libraries" ON)
option(BUILD_EXECUTABLE "Build the executable" OFF)
option(STATIC "static" ON)
option(SANITIZE "sanitize" OFF)

# ------------------------------------------------------------------------------
# Set library search paths (for the executable)
# ------------------------------------------------------------------------------
# CMake will look for static libraries (.a) by default instead of shared (.so).
# Additionally, the system's LD_LIBRARY_PATH environment variable is used
# to locate additional libraries.
# ------------------------------------------------------------------------------
set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
set(CMAKE_LIBRARY_PATH $ENV{LD_LIBRARY_PATH})

# ------------------------------------------------------------------------------
# Include the external dependencies configuration file.
# This ensures that all required libraries are found before linking.
# ------------------------------------------------------------------------------
include(cmake/Dependencies.cmake)

# ------------------------------------------------------------------------------
# Add link options for multi-threading support.
# ------------------------------------------------------------------------------
add_link_options(-pthread)

# ------------------------------------------------------------------------------
# Enable sanitizers if the SANITIZE option is ON.
# Sanitizers help detect memory errors, undefined behavior, and address issues.
# ------------------------------------------------------------------------------
if(${SANITIZE})
    message(STATUS "Sanitizer enabled")

    # run "ulimit -n 2048", otherwise the resulting binary will hang

    add_link_options(-fsanitize=undefined,address -fsanitize-recover=undefined,address)
    add_compile_options(-fsanitize=undefined,address -fsanitize-recover=undefined,address)

    # MemorySanitizer is currently useless due to false positives
    # add_link_options(-fsanitize=memory -fsanitize-recover=memory -fsanitize-memory-track-origins)
    # add_compile_options(-fsanitize=memory -fsanitize-recover=memory -fsanitize-memory-track-origins)
    # add_compile_options(-fPIE -pie -fno-omit-frame-pointer)
endif()

# ------------------------------------------------------------------------------
# Enable compiler warnings to enforce good coding practices.
# -Wall: Enables all warnings.
# -Wextra: Enables extra warnings.
# -pedantic: Ensures compliance with the standard.
# -Wno-unused-parameter: Suppresses warnings about unused function parameters.
# ------------------------------------------------------------------------------
add_compile_options(-Wall -Wextra -pedantic -Wno-unused-parameter) # for GCC: -fconcepts-diagnostics-depth=100

# ------------------------------------------------------------------------------
# Set compiler flags for different build types (Debug, Release, etc.).
# ------------------------------------------------------------------------------
set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE} -g")

# ------------------------------------------------------------------------------
# Print build configurations for debugging purposes.
# ------------------------------------------------------------------------------
message(STATUS "Build type: " ${CMAKE_BUILD_TYPE})
message(STATUS "Compiler flags:" ${CMAKE_CXX_COMPILE_FLAGS})
message(STATUS "Compiler cxx debug flags:" ${CMAKE_CXX_FLAGS_DEBUG})
message(STATUS "Compiler cxx release flags:" ${CMAKE_CXX_FLAGS_RELEASE})
message(STATUS "Compiler cxx relwithdebinfo flags:" ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
message(STATUS "Compiler cxx min size flags:" ${CMAKE_CXX_FLAGS_MINSIZEREL})
message(STATUS "Compiler cxx flags:" ${CMAKE_CXX_FLAGS})

file(GLOB_RECURSE SRC_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE HEADER_FILES ${CMAKE_SOURCE_DIR}/src/*.hpp)

# 🔍 API-spezifische Quellen extrahieren
file(GLOB_RECURSE API_SRC_FILES ${CMAKE_SOURCE_DIR}/src/api/*.cpp)
file(GLOB_RECURSE API_HEADER_FILES ${CMAKE_SOURCE_DIR}/src/api/*.cpp)

# 🔍 Alle anderen Quellen für die Executable
set(EXECUTABLE_SRC_FILES ${SRC_FILES})
list(REMOVE_ITEM EXECUTABLE_SRC_FILES ${API_SRC_FILES})


# ------------------------------------------------------------------------------
# Include executable setup if we are building an executable.
# ------------------------------------------------------------------------------
include(cmake/ExecutableSetup.cmake)

# ------------------------------------------------------------------------------
# Library setup (only if shared or static libraries are built)
# ------------------------------------------------------------------------------
if(BUILD_SHARED_LIBRARY OR BUILD_STATIC_LIBRARY)

    # ------------------------------------------------------------------------------
    # Define library names
    # ------------------------------------------------------------------------------
    set(LIBRARY_NAME LoAT_API)
    set(STATIC_LIBRARY_NAME LoAT_API_Static)

    # ------------------------------------------------------------------------------
    # Define the API directory where the public headers are located
    # ------------------------------------------------------------------------------
    set(API_DIR ${CMAKE_SOURCE_DIR}/src/api)

    # ------------------------------------------------------------------------------
    # Automatically find all source and header files
    # ------------------------------------------------------------------------------
    file(GLOB_RECURSE SOURCES ${API_DIR}/*.cpp)
    file(GLOB_RECURSE HEADERS ${API_DIR}/*.hpp)

    message(STATUS "SOURCES: ${SOURCES}") 
    message(STATUS "HEADERS: ${SOURCES}") 

    # ------------------------------------------------------------------------------
    # Create shared library (if enabled)
    # ------------------------------------------------------------------------------
    if(BUILD_SHARED_LIBRARY)
        add_library(${LIBRARY_NAME} SHARED ${SOURCES})
        target_include_directories(${LIBRARY_NAME} PUBLIC
            $<BUILD_INTERFACE:${API_DIR}>
            $<INSTALL_INTERFACE:include>
        )
    endif()

    # ------------------------------------------------------------------------------
    # Create static library (if enabled)
    # ------------------------------------------------------------------------------
    if(BUILD_STATIC_LIBRARY)
        add_library(${STATIC_LIBRARY_NAME} STATIC ${SOURCES})
        target_include_directories(${STATIC_LIBRARY_NAME} PUBLIC
            $<BUILD_INTERFACE:${API_DIR}>
            $<INSTALL_INTERFACE:include>
        )
    endif()

    # ------------------------------------------------------------------------------
    # Installation rules (only if at least one library is built)
    # ------------------------------------------------------------------------------
    include(GNUInstallDirs)

    install(TARGETS ${LIBRARY_NAME} ${STATIC_LIBRARY_NAME}
        EXPORT ${LIBRARY_NAME}Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    # ------------------------------------------------------------------------------
    # Export the library for `find_package()`
    # ------------------------------------------------------------------------------
    install(EXPORT ${LIBRARY_NAME}Targets
        FILE ${LIBRARY_NAME}Targets.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${LIBRARY_NAME}
    )

    # ------------------------------------------------------------------------------
    # Install header files
    # ------------------------------------------------------------------------------
    install(DIRECTORY ${API_DIR}/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.hpp"
    )

    # ------------------------------------------------------------------------------
    # Generate package configuration files for CMake
    # ------------------------------------------------------------------------------
    # This section generates CMake package configuration files, allowing
    # other projects to find and use this library via find_package().
    include(CMakePackageConfigHelpers)

    # # ------------------------------------------------------------------------------
    # # Generate a version file for compatibility checking
    # # ------------------------------------------------------------------------------
    # # This file ensures that when a project requests LoAT,
    # # it gets a version that matches the major version it was built for.
    # write_basic_package_version_file(
    #     "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}ConfigVersion.cmake"
    #     VERSION ${PROJECT_VERSION}
    #     COMPATIBILITY SameMajorVersion
    # )

    # ------------------------------------------------------------------------------
    # Generate the main package configuration file
    # ------------------------------------------------------------------------------
    # This generates `LoATConfig.cmake`, which helps CMake-based projects
    # automatically locate the LoAT library when they use `find_package(LoAT)`.
    configure_package_config_file(
        "${CMAKE_SOURCE_DIR}/cmake/${LIBRARY_NAME}Config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}Config.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${LIBRARY_NAME}
    )

    # ------------------------------------------------------------------------------
    # Install generated CMake configuration files
    # ------------------------------------------------------------------------------
    # Install the generated config files for package discovery.
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}Config.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${LIBRARY_NAME}
    )

endif() # End of library construction
