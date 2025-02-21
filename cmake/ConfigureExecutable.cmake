# ------------------------------------------------------------------------------
# This file configures and builds the executable target.
# ------------------------------------------------------------------------------

# ------------------------------------------------------------------------------
# Include only if BUILD_EXECUTABLE is ON.
# ------------------------------------------------------------------------------
if(NOT BUILD_EXECUTABLE)
    return()
endif()

# ------------------------------------------------------------------------------
# Configure build type based on the STATIC option.
# If STATIC is ON, a static build is configured with static linking enabled.
# Otherwise, a shared build is configured with shared objects.
# ------------------------------------------------------------------------------
if(STATIC)
    message(STATUS "Configuring static build")
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")  
    set(BUILD_SHARED_LIBS OFF)
    add_link_options(-ldl -static) 
else()
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".so" ".a")
  message(STATUS "Configuring non-static build")
endif()

# ------------------------------------------------------------------------------
# Include Git revision tracking to embed version info into the binary. (for the executable)
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
  ${EXTERNAL_LIBS}
)

# ------------------------------------------------------------------------------
# Print a status message confirming the executable setup.
# ------------------------------------------------------------------------------
message(STATUS "Executable '${EXECUTABLE}' has been set up successfully.")