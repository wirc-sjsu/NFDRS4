cmake_minimum_required (VERSION 3.13)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project (
    NFDRS4
    VERSION 1.0
    DESCRIPTION "National Fire Danger Rating System 4"
    LANGUAGES CXX)

# Only do these if this is the main project, and not if it is included through add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(CMAKE_CXX_EXTENSIONS OFF)
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

# Define SPATIAL_LIBS_DIR before adding subdirectories
set(SPATIAL_LIBS_DIR "${CMAKE_SOURCE_DIR}/extern/spatial")

add_subdirectory(lib)
add_subdirectory(app)

#install locations for apps, headers, and libs
set(app_dest "bin")
set(include_dest "include")
set(lib_dest "lib")
