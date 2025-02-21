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
# STATIC: Whether to build a static binary (default ON).
# SANITIZE: Whether to enable sanitizers for debugging (default OFF).
# ------------------------------------------------------------------------------
option(STATIC "static" ON)
option(SANITIZE "sanitize" OFF)

# ------------------------------------------------------------------------------
# Set library search paths
# ------------------------------------------------------------------------------
# CMake will look for static libraries (.a) by default instead of shared (.so).
# Additionally, the system's LD_LIBRARY_PATH environment variable is used
# to locate additional libraries.
# ------------------------------------------------------------------------------
set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
set(CMAKE_LIBRARY_PATH $ENV{LD_LIBRARY_PATH})

# ------------------------------------------------------------------------------
# Add link options for multi-threading support.
# The "-pthread" flag ensures proper linking against the POSIX threading library.
# ------------------------------------------------------------------------------
add_link_options(-pthread)

# ------------------------------------------------------------------------------
# Configure build type based on the STATIC option.
# If STATIC is ON, a static build is configured with static linking enabled.
# Otherwise, a shared build is configured with shared objects.
# ------------------------------------------------------------------------------
if(${STATIC})
    message(STATUS "Configuring static build")
    # Prefer static libraries
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")  
    set(BUILD_SHARED_LIBS OFF)
    # Link dynamically-loaded libraries statically
    add_link_options(-ldl -static) 
    set(EXECUTABLE loat-static)
else()
  # Prefer shared but allow static
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".so" ".a")
  message(STATUS "Configuring non-static build")
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

# ------------------------------------------------------------------------------
# Find required external libraries.
# ANTLR4 (parser generator runtime) and FAUDES (automata library).
# ------------------------------------------------------------------------------
find_library(ANTLR4 antlr4-runtime)
message(STATUS "antlr4: ${ANTLR4}")
find_library(FAUDES faudes)
message(STATUS "faudes: ${FAUDES}")

# ------------------------------------------------------------------------------
# Include Git revision tracking to embed version info into the binary.
# ------------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC SHA)
git_local_changes(DIRTY)


# ------------------------------------------------------------------------------
# Generate a version file from a template (src/version.cpp.in).
# This embeds the git SHA and versioning information in the build.
# ------------------------------------------------------------------------------
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/version.cpp.in" "${CMAKE_CURRENT_SOURCE_DIR}/src/version.cpp" @ONLY)

# ------------------------------------------------------------------------------
# Create the executable target.
# ------------------------------------------------------------------------------
add_executable(${EXECUTABLE} "")

# ------------------------------------------------------------------------------
# Add the 'src' directory, which contains source files.
# ------------------------------------------------------------------------------
add_subdirectory(src)

# ------------------------------------------------------------------------------
# Specify include directories required by the executable.
# ------------------------------------------------------------------------------
target_include_directories(${EXECUTABLE} PRIVATE
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


# ------------------------------------------------------------------------------
# Find additional external solver libraries.
# These provide support for SMT solving, symbolic computation, and arithmetic.
# ------------------------------------------------------------------------------
message(STATUS "Searching libraries")
find_library(Z3 z3)
message(STATUS "z3: ${Z3}")
find_library(YICES yices)
message(STATUS "yices: ${YICES}")
find_library(POLY poly)
message(STATUS "poly: ${POLY}")
find_library(POLYXX polyxx)
message(STATUS "polyxx: ${POLYXX}")
find_library(CUDD cudd)
message(STATUS "cudd: ${CUDD}")
find_library(CVC5 cvc5)
message(STATUS "cvc5: ${CVC5}")
find_library(CADICAL cadical)
message(STATUS "cadical: ${CADICAL}")
find_library(SWINE swine-z3)
message(STATUS "swine: ${SWINE}")
find_library(CLN cln)
message(STATUS "cln: ${CLN}")
find_library(GMP gmp)
message(STATUS "gmp: ${GMP}")

# ------------------------------------------------------------------------------
# Link all found libraries to the executable.
# $<TARGET_OBJECTS:*> is used to link object libraries.
# ------------------------------------------------------------------------------
target_link_libraries(${EXECUTABLE}
  $<TARGET_OBJECTS:sexpresso>
  $<TARGET_OBJECTS:loat_util>
  $<TARGET_OBJECTS:loat_expr>
  $<TARGET_OBJECTS:loat_fmplex>
  $<TARGET_OBJECTS:loat_preprocessing>
  $<TARGET_OBJECTS:loat_smt>
  $<TARGET_OBJECTS:loat_recurrence>
  $<TARGET_OBJECTS:loat_acceleration>
  $<TARGET_OBJECTS:loat_trp>
  $<TARGET_OBJECTS:loat_itsproblem>
  $<TARGET_OBJECTS:loat_chcproblem>
  $<TARGET_OBJECTS:loat_safetyproblem>
  $<TARGET_OBJECTS:loat_transformations>
  ${ANTLR4}
  ${FAUDES}
  ${SWINE}
  ${Z3}
  ${CVC5}
  ${CADICAL}
  ${YICES}
  ${POLYXX}
  ${POLY}
  ${CUDD}
  ${CLN}
  ${GMP}
  ${LINKER_OPTIONS}
)
