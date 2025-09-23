cmake_minimum_required(VERSION 3.10)
project(covscript-database)
include_directories(.)

# nanodbc specific options

option(NANODBC_DISABLE_ASYNC "Disable async features entirely" OFF)
option(NANODBC_ENABLE_UNICODE "Enable Unicode support" ON)
option(NANODBC_ENABLE_WORKAROUND_NODATA "Enable SQL_NO_DATA workaround (see Issue #33)" OFF)

# Compiler Options

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    set(CMAKE_CXX_FLAGS "/O2 /EHsc /utf-8 /w")
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
elseif (CMAKE_COMPILER_IS_GNUCXX)
    if (WIN32)
        set(CMAKE_C_FLAGS "--static -fPIC -s -O3")
        set(CMAKE_CXX_FLAGS "--static -fPIC -s -O3")
    else ()
        set(CMAKE_C_FLAGS "-fPIC -s -O3")
        set(CMAKE_CXX_FLAGS "-fPIC -s -O3")
    endif ()
else ()
    set(CMAKE_C_FLAGS "-fPIC -O3")
    set(CMAKE_CXX_FLAGS "-fPIC -O3")
endif ()

# nanodbc features

IF (NOT DEFINED NANODBC_ODBC_VERSION)
    message(STATUS "nanodbc feature: ODBC Version Override - OFF")
else ()
    message(STATUS "nanodbc feature: ODBC Version Override - ${NANODBC_ODBC_VERSION}")
    add_definitions(-DNANODBC_ODBC_VERSION=${NANODBC_ODBC_VERSION})
endif ()

if (NANODBC_DISABLE_ASYNC)
    add_definitions(-DNANODBC_DISABLE_ASYNC)
endif ()
message(STATUS "nanodbc feature: Disable async features - ${NANODBC_DISABLE_ASYNC}")

if (NANODBC_ENABLE_UNICODE)
    add_definitions(-DNANODBC_ENABLE_UNICODE)
    if (MSVC)
        # Sets "Use Unicode Character Set" property in Visual Studio projects
        add_definitions(-DUNICODE -D_UNICODE)
    endif ()
endif ()
message(STATUS "nanodbc feature: Enable Unicode - ${NANODBC_ENABLE_UNICODE}")

if (NANODBC_ENABLE_WORKAROUND_NODATA)
    add_definitions(-DNANODBC_ENABLE_WORKAROUND_NODATA)
endif ()
message(STATUS "nanodbc feature: Enable SQL_NO_DATA bug workaround - ${NANODBC_ENABLE_WORKAROUND_NODATA}")

# find CovScript SDK

if (DEFINED ENV{CS_DEV_PATH})
    message(STATUS "CovScript SDK Path: $ENV{CS_DEV_PATH}")
    include_directories($ENV{CS_DEV_PATH}/include)
    link_directories($ENV{CS_DEV_PATH}/lib)
endif ()

if (DEFINED ENV{CS_DEV_OUTPUT})
    message(STATUS "CovScript Package Install Path: $ENV{CS_DEV_OUTPUT}")
    set(LIBRARY_OUTPUT_PATH $ENV{CS_DEV_OUTPUT})
    set(EXECUTABLE_OUTPUT_PATH $ENV{CS_DEV_OUTPUT})
endif ()

# find unixODBC or iODBC config binary

if (UNIX)
    # Try to find unixODBC first via odbc_config program.
    find_program(ODBC_CONFIG odbc_config
            PATHS $ENV{ODBC_PATH}/bin /usr/bin /usr/local/bin)
    if (ODBC_CONFIG)
        message(STATUS "csbuild: ODBC on Unix - unixODBC")
        set(ODBCLIB odbc)
        execute_process(COMMAND ${ODBC_CONFIG} --include-prefix
                OUTPUT_VARIABLE ODBC_INCLUDE_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
        set(ODBC_CFLAGS "-I${ODBC_INCLUDE_DIR}")
        set(CMAKE_FLAGS "${CMAKE_FLAGS} ${ODBC_CFLAGS}")
        execute_process(COMMAND ${ODBC_CONFIG} --libs
                OUTPUT_VARIABLE ODBC_LINK_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif ()

    # Fallback to finding unixODBC via install paths
    if (NOT ODBC_CONFIG)
        find_path(UnixODBC_INCLUDE_DIR uodbc_stats.h
                /usr/include
                /usr/local/include
                /usr/include/odbc
                /usr/local/include/odbc
                /usr/include/libodbc
                /usr/local/include/libodbc)
        if (UnixODBC_INCLUDE_DIR)
            set(ODBC_CONFIG 1)
            message(STATUS "csbuild: ODBC on Unix - unixODBC")
            set(ODBCLIB odbc)
            set(ODBC_CFLAGS "-I${UnixODBC_INCLUDE_DIR} -DHAVE_UNISTD_H -DHAVE_PWD_H -DHAVE_SYS_TYPES_H -DHAVE_LONG_LONG -DSIZEOF_LONG_INT=8")
        endif ()
    endif ()

    # Fallback to using iODBC
    if (NOT ODBC_CONFIG)
        find_program(ODBC_CONFIG iodbc-config
                PATHS $ENV{ODBC_PATH}/bin /usr/bin /usr/local/bin)
        if (ODBC_CONFIG)
            message(STATUS "csbuild: ODBC on Unix - iODBC")
            set(ODBCLIB iodbc)
            execute_process(COMMAND ${ODBC_CONFIG} --cflags
                    OUTPUT_VARIABLE ODBC_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
            set(CMAKE_FLAGS "${CMAKE_FLAGS} ${ODBC_CFLAGS}")
            execute_process(COMMAND ${ODBC_CONFIG} --libs
                    OUTPUT_VARIABLE ODBC_LINK_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
            if (NANODBC_ENABLE_UNICODE)
                add_definitions(-DNANODBC_USE_IODBC_WIDE_STRINGS)
            endif ()
        endif ()
    endif ()

    if (NOT ODBC_CONFIG)
        message(FATAL_ERROR "can not find a suitable odbc driver manager")
    endif ()

    message(STATUS "ODBC compile flags: ${ODBC_CFLAGS}")
    message(STATUS "ODBC link flags: ${ODBC_LINK_FLAGS}")
endif ()

# patch for SQLWCHAR

include(CheckCXXSourceCompiles)

set(CMAKE_REQUIRED_FLAGS "${ODBC_CFLAGS}")

check_cxx_source_compiles("
#include <string>

#ifdef _WIN32
// needs to be included above sql.h for windows
#if !defined(__MINGW32__) && !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include <sql.h>
#include <sqlext.h>

int main() {
    using Traits = std::char_traits<SQLWCHAR>;
    (void)Traits::length((const SQLWCHAR*)0);
    return 0;
}
" HAS_SQLWCHAR_CHAR_TRAITS)

if (NOT HAS_SQLWCHAR_CHAR_TRAITS)
    message("-- Applying patch for std::char_traits<SQLWCHAR>")
    add_compile_definitions(NANODBC_SQLWCHAR_PATCH)
endif ()

# find ODBC libraries to link

if (UNIX)
    set(ODBC_LIBRARIES ${ODBCLIB})
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${ODBC_LINK_FLAGS}")
elseif (MSVC OR CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    set(ODBC_LIBRARIES odbc32.lib odbccp32.lib Ws2_32.lib)
elseif (MINGW)
    set(ODBC_LIBRARIES odbc32 odbccp32)
endif ()

# Target Settings

add_library(covscript-database SHARED database.cpp nanodbc/nanodbc.cpp)

if (APPLE AND HAS_SQLWCHAR_CHAR_TRAITS)
    target_compile_options(covscript-database PRIVATE -Wno-deprecated-declarations)
endif ()

target_link_libraries(covscript-database covscript ${ODBC_LIBRARIES})

if (UNIX)
    set_target_properties(covscript-database PROPERTIES COMPILE_FLAGS "${ODBC_CFLAGS}")
endif ()

set_target_properties(covscript-database PROPERTIES OUTPUT_NAME database)
set_target_properties(covscript-database PROPERTIES PREFIX "")
set_target_properties(covscript-database PROPERTIES SUFFIX ".cse")