cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 20)

project(loat_safetyproblem CXX)

add_library(${PROJECT_NAME} OBJECT
    safetyproblem.hpp
    safetyproblem.cpp
    safetycex.hpp
    safetycex.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${sexpresso_INCLUDE_DIRS}
  ${loat_util_INCLUDE_DIRS}
  ${loat_container_INCLUDE_DIRS}
  ${loat_expr_INCLUDE_DIRS}
)

target_include_directories(${PROJECT_NAME} INTERFACE ".")

# Expose public includes to other
# subprojects through cache variable.
set(${PROJECT_NAME}_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)
