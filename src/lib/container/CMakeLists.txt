cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 20)

project(loat_container CXX)

add_library(${PROJECT_NAME} INTERFACE
  variantset.hpp
  linkedhashset.hpp
  linkedhashmap.hpp
)

# target_include_directories(${PROJECT_NAME} PRIVATE
#   ${loat_util_INCLUDE_DIRS}
# )

target_include_directories(${PROJECT_NAME} INTERFACE
  "."
)

# Expose public includes to other
# subprojects through cache variable.
set(${PROJECT_NAME}_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)
