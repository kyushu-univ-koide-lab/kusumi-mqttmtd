cmake_minimum_required(VERSION 3.5)

set(EXTRA_COMPONENT_DIRS "../components")

# Set the components to include the tests for.
# This can be overriden from CMake cache:
# - when invoking CMake directly: cmake -D TEST_COMPONENTS="xxxxx" ..
# - when using idf.py: idf.py -T xxxxx build
set(TEST_COMPONENTS "tokenmgr" CACHE STRING "List of components to test")
add_compile_options(-DCOMPILEROPT_INCLUDE_TIME_LOG)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(test)