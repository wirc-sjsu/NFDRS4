cmake_minimum_required(VERSION 3.13)
project(NFDRS4_spatial)

if(MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

add_executable(${PROJECT_NAME} src/main.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include  # Local include directory
)

target_link_directories(${PROJECT_NAME} PRIVATE
    ${SPATIAL_LIBS_DIR}/lib
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    NFDRS4
    
    netcdf_c++4                          # NetCDF C++ library
    netcdf                               # NetCDF C library
    hdf5_hl                              # HDF5 high-level library
    hdf5                                 # HDF5 library
    z                                    # zlib library
)