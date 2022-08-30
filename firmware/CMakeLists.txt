# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)
add_definitions(-DBADGE_VERSION_MAJOR=1 -DBADGE_VERSION_MINOR=0 -DBADGE_VERSION_PATCH=12)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(badge-esp32)