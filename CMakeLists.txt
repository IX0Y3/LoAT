# ------------------------------------------------------------------------------
# Specify the C and C++ compilers to be used.
# Clang is explicitly set as the compiler for both C and C++.
# ------------------------------------------------------------------------------
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER /usr/bin/clang++)

# ------------------------------------------------------------------------------
# CMake minimum version requirement
# ------------------------------------------------------------------------------
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
# BUILD_STATIC_LIBRARY: Build static lib (default ON)
# BUILD_EXECUTABLE: Build the executable (default ON)
# STATIC: Whether to build a static binary - if we build an executable (default ON).
# SANITIZE: Whether to enable sanitizers for debugging (default OFF).
# ------------------------------------------------------------------------------
option(BUILD_STATIC_LIBRARY "Build static libraries" ON)
option(BUILD_EXECUTABLE "Build the executable" ON)
option(STATIC "static" ON)
option(SANITIZE "sanitize" OFF)

set(BUILD_STATIC_LIBRARY ON)
set(BUILD_EXECUTABLE OFF)

# ------------------------------------------------------------------------------
# Set library search paths (for the executable)
# ------------------------------------------------------------------------------
set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
set(CMAKE_LIBRARY_PATH $ENV{LD_LIBRARY_PATH})
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Install path prefix" FORCE)


# ------------------------------------------------------------------------------
# Include the external dependencies configuration file.
# ------------------------------------------------------------------------------
include(cmake/Dependencies.cmake)

# ------------------------------------------------------------------------------
# Add link options for multi-threading support.
# ------------------------------------------------------------------------------
add_link_options(-pthread)

# ------------------------------------------------------------------------------
# Define static library name
# ------------------------------------------------------------------------------
set(STATIC_LIBRARY_NAME LoAT_API_Static)

# ------------------------------------------------------------------------------
# Set the name of potential executable
# ------------------------------------------------------------------------------
if(${STATIC})
    set(EXECUTABLE loat-static)
else()
    set(EXECUTABLE loat)
endif()

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
add_compile_options(-Wall -Wextra -pedantic -Wno-unused-parameter)

# ------------------------------------------------------------------------------
# Set compiler flags for different build types (Debug, Release, etc.).
# ------------------------------------------------------------------------------
set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE} -g")

# ------------------------------------------------------------------------------
# Print build configurations for debugging purposes.
# ------------------------------------------------------------------------------
# message(STATUS "Build type: " ${CMAKE_BUILD_TYPE})
# message(STATUS "Compiler flags:" ${CMAKE_CXX_COMPILE_FLAGS})
# message(STATUS "Compiler cxx debug flags:" ${CMAKE_CXX_FLAGS_DEBUG})
# message(STATUS "Compiler cxx release flags:" ${CMAKE_CXX_FLAGS_RELEASE})
# message(STATUS "Compiler cxx relwithdebinfo flags:" ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
# message(STATUS "Compiler cxx min size flags:" ${CMAKE_CXX_FLAGS_MINSIZEREL})
# message(STATUS "Compiler cxx flags:" ${CMAKE_CXX_FLAGS})

# ------------------------------------------------------------------------------
# Create the targets executable / library / static library .
# ------------------------------------------------------------------------------
if(BUILD_EXECUTABLE)
    add_executable(${EXECUTABLE} "")
endif()

if(BUILD_STATIC_LIBRARY)
    add_library(${STATIC_LIBRARY_NAME} STATIC "")
endif()

# ------------------------------------------------------------------------------
# Call the CMakeLists.txt of the /src directory
# ------------------------------------------------------------------------------
add_subdirectory(src)

# ------------------------------------------------------------------------------
# Include executable setup if we are building an executable.
# ------------------------------------------------------------------------------
include(cmake/ConfigureExecutable.cmake)

# ------------------------------------------------------------------------------
# Library setup (static library only)
# ------------------------------------------------------------------------------
if(BUILD_STATIC_LIBRARY)
    # Define the API directory where the public headers are located
    set(API_DIR ${CMAKE_SOURCE_DIR}/src/api)

    target_include_directories(${STATIC_LIBRARY_NAME} PUBLIC
        $<BUILD_INTERFACE:${API_DIR}>
        $<INSTALL_INTERFACE:include>
    )

    set(OBJECT_LIBRARIES 
        sexpresso
        loat_util
        loat_expr
        loat_fmplex
        loat_preprocessing
        loat_smt
        loat_recurrence
        loat_acceleration
        loat_trp
        loat_itsproblem
        loat_chcproblem
        loat_safetyproblem
        loat_transformations
    )

    foreach(LIB ${OBJECT_LIBRARIES})
        target_sources(${STATIC_LIBRARY_NAME} PRIVATE $<TARGET_OBJECTS:${LIB}>)
    endforeach()

    set(INCLUDE_DIRECTORIES
        ${sexpresso_INCLUDE_DIRS}
        ${loat_util_INCLUDE_DIRS}
        ${loat_container_INCLUDE_DIRS}
        ${loat_expr_INCLUDE_DIRS}
        ${loat_fmplex_INCLUDE_DIRS}
        ${loat_preprocessing_INCLUDE_DIRS}
        ${loat_smt_INCLUDE_DIRS}
        ${loat_recurrence_INCLUDE_DIRS}
        ${loat_acceleration_INCLUDE_DIRS}
        ${loat_trp_INCLUDE_DIRS}
        ${loat_itsproblem_INCLUDE_DIRS}
        ${loat_chcproblem_INCLUDE_DIRS}
        ${loat_safetyproblem_INCLUDE_DIRS}
        ${loat_transformations_INCLUDE_DIRS}
    )

    target_include_directories(${STATIC_LIBRARY_NAME} PUBLIC
        $<BUILD_INTERFACE:${INCLUDE_DIRECTORIES}>
        $<INSTALL_INTERFACE:include>
    )

    # Finde Boost und stelle sicher, dass die benötigten Komponenten verfügbar sind
    find_library(BOOST boost)

    # Füge Boost zu den Include-Direktiven hinzu
    target_include_directories(${STATIC_LIBRARY_NAME} PUBLIC
        $<BUILD_INTERFACE:${Boost_INCLUDE_DIRS}>
        $<INSTALL_INTERFACE:include/boost>
    )

    target_link_libraries(${STATIC_LIBRARY_NAME} BOOST)

    file(GLOB BOOST_HEADERS "${Boost_INCLUDE_DIRS}/*.hpp" "${Boost_INCLUDE_DIRS}/*.h")

    install(FILES ${BOOST_HEADERS} DESTINATION include/boost)

    # target_link_libraries(${STATIC_LIBRARY_NAME} ${EXTERNAL_LIBS})

    # ------------------------------------------------------------------------------
    # Installation rules for the static library.
    # ------------------------------------------------------------------------------
    include(GNUInstallDirs)
    install(TARGETS ${STATIC_LIBRARY_NAME}
        EXPORT ${STATIC_LIBRARY_NAME}Targets
        ARCHIVE DESTINATION lib
    )
    install(EXPORT ${STATIC_LIBRARY_NAME}Targets
        FILE ${STATIC_LIBRARY_NAME}Targets.cmake
        DESTINATION lib/cmake/${STATIC_LIBRARY_NAME}
    )

    # ------------------------------------------------------------------------------
    # Install header files.
    # ------------------------------------------------------------------------------
    file(GLOB_RECURSE ALL_HEADERS "${CMAKE_SOURCE_DIR}/src/*.hpp" "${CMAKE_SOURCE_DIR}/src/*.h")
    install(FILES ${ALL_HEADERS} DESTINATION include)

    # ------------------------------------------------------------------------------
    # Generate package configuration files for CMake
    # ------------------------------------------------------------------------------
    include(CMakePackageConfigHelpers)

    configure_package_config_file(
        "${CMAKE_SOURCE_DIR}/cmake/${STATIC_LIBRARY_NAME}Config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/${STATIC_LIBRARY_NAME}Config.cmake"
        INSTALL_DESTINATION lib/cmake/${STATIC_LIBRARY_NAME}
    )

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${STATIC_LIBRARY_NAME}Config.cmake"
        DESTINATION lib/cmake/${STATIC_LIBRARY_NAME}
    )

    set_target_properties(${STATIC_LIBRARY_NAME} PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES $<INSTALL_INTERFACE:include>
    )

endif()
