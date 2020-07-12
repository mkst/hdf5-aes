# TBD
cmake_minimum_required(VERSION 3.10)

project(hdf5_aes VERSION 0.0.1 DESCRIPTION "HDF5 AES Filter")

include(GNUInstallDirs)

#------------------------------------------------------------------------------
# Add the filter plugin
#------------------------------------------------------------------------------
add_library(hdf5_aes SHARED
    hdf5_aes.c
)

#------------------------------------------------------------------------------
# Set a default build type if none was specified
#------------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release)
endif()

#------------------------------------------------------------------------------
# Find HDF5
#------------------------------------------------------------------------------
# You probably only need 1.8 for this to work...
find_package(HDF5 NO_MODULE NAMES hdf5 COMPONENTS C shared)
if(HDF5_FOUND)
    set(HDF5_C_SHARED_LIBRARY hdf5-shared)
    if(NOT TARGET ${HDF5_C_SHARED_LIBRARY})
        message(FATAL_ERROR "Could not find hdf5 shared target, please make "
        "sure that HDF5 has ben compiled with shared libraries enabled.")
    endif()
    set(AES_FILTER_EXT_PKG_DEPENDENCIES
        ${AES_FILTER_EXT_PKG_DEPENDENCIES}
        ${HDF5_C_SHARED_LIBRARY})
else()
    # Allow for HDF5 autotools builds
    # NOTE: I have not gotten this to work...
    find_package(HDF5 MODULE REQUIRED)
    if(HDF5_FOUND)
        set(AES_FILTER_EXT_INCLUDE_DEPENDENCIES
            ${AES_FILTER_EXT_INCLUDE_DEPENDENCIES}
            ${HDF5_INCLUDE_DIRS})
        set(AES_FILTER_EXT_LIB_DEPENDENCIES
            ${AES_FILTER_EXT_LIB_DEPENDENCIES}
            ${HDF5_LIBRARIES})
    else()
        message(FATAL_ERROR "Could not find HDF5, please check HDF5_DIR.")
    endif()
endif()

# find openssl
find_package(OpenSSL)

#------------------------------------------------------------------------------
# Some minimum target properties
#------------------------------------------------------------------------------
set_target_properties(hdf5_aes PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER hdf5_aes.h
)

#------------------------------------------------------------------------------
# Set external include directories and libraries
#------------------------------------------------------------------------------
target_include_directories(hdf5_aes
    SYSTEM PUBLIC ${AES_FILTER_EXT_INCLUDE_DEPENDENCIES}
)
target_link_libraries(hdf5_aes
    ${AES_FILTER_EXT_LIB_DEPENDENCIES}
    ${AES_FILTER_EXT_PKG_DEPENDENCIES}
    OpenSSL::Crypto
)

#------------------------------------------------------------------------------
# Install
#------------------------------------------------------------------------------
install(TARGETS hdf5_aes
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
