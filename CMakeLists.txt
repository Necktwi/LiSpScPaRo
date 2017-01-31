cmake_minimum_required (VERSION 3.5)
include(GNUInstallDirs)
project (LiSpScPaRo)
IF (DEFINED _DEBUG)
ADD_DEFINITIONS(-D_DEBUG=${_DEBUG})
ENDIF()
set (VERSION_MAJOR 1)
set (VERSION_MINOR 0)
set (Umbrella "ferryfair")
configure_file (
    "${PROJECT_SOURCE_DIR}/config.h.in"
    "${PROJECT_BINARY_DIR}/config.h"
    )

message(${CMAKE_SYSTEM_NAME})
# macOS specific code
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
SET(macOS ON)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

# Linux specific code
IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
SET(Linux ON)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

# Windows specific code
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
SET(Windows ON)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

#file(GLOB HEADERS "*.h")
#file(GLOB SOURCES "*.cpp")
#list(FILTER HEADERS REGEX "^[^\.].+" output_variable HEADERS)
#list(FILTER SOURCES REGEX "^[^\.].+" output_variable SOURCES)
#message("HEADERS: " ${HEADERS})
#message("SOURCES: " ${SOURCES})
SET(HEADERS config.h connect.h)
SET(SOURCES LiSpScPaRo.cpp connect.cpp)
set (GCC_COVERAGE_COMPILE_FLAGS "-std=c++14 -pthread")
set (GCC_COVERAGE_LINK_FLAGS "")

IF(Linux)
set (GCC_COVERAGE_LINK_FLAGS "${GCC_COVERAGE_LINK_FLAGS} -fPIC -Wl,-unresolved-symbols=ignore-in-shared-libs -static")
set (BUILD_SHARED_LIBRARIES OFF)
ENDIF(Linux)
IF(Windows)
set (BUILD_SHARED_LIBRARIES OFF)
ENDIF(Windows)

IF (DEFINED _DEBUG)
set (GCC_COVERAGE_COMPILE_FLAGS "${GCC_COVERAGE_COMPILE_FLAGS} -g")
set (GCC_COVERAGE_LINK_FLAGS "${GCC_COVERAGE_LINK_FLAGS} -g")
ENDIF()
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")
include_directories(${PROJECT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_INSTALL_FULL_INCLUDEDIR}/${Umbrella})
IF (macOS)
include_directories("/usr/local/include")
link_directories("/usr/local/lib")
set(SERIALIZE_LIB boost_serialization)
set(SYSTEM_LIB boost_system)
set(THREAD_LIB boost_thread-mt)
set(EXCEPTION_LIB boost_exception-mt)
ENDIF (macOS)
IF (Linux)
include_directories("/usr/local/include")
link_directories("/usr/local/lib")
set(SERIALIZE_LIB boost_serialization)
set(SYSTEM_LIB boost_system)
set(THREAD_LIB boost_thread)
set(EXCEPTION_LIB boost_exception)
ENDIF (Linux)
IF (Windows)
include_directories("C:/Program Files/boost")
link_directories("C:/Program Files/boost/lib64-msvc-14.0")
find_library(SERIALIZE_LIB libboost_serialization-vc140-mt-gd-1_63.lib HINTS "C:/Program Files/boost/lib64-msvc-14.0")
find_library(SYSTEM_LIB libboost_system-vc140-mt-gd-1_63.lib HINTS "C:/Program Files/boost/lib64-msvc-14.0")
find_library(THREAD_LIB libboost_thread-vc140-mt-gd-1_63.lib HINTS "C:/Program Files/boost/lib64-msvc-14.0")
find_library(EXCEPTION_LIB libboost_exception-vc140-mt-gd-1_63.lib HINTS "C:/Program Files/boost/lib64-msvc-14.0")
find_library(TIME_LIB libboost_date_time-vc140-mt-gd-1_63.lib HINTS "C:/Program Files/boost/lib64-msvc-14.0")
find_library(REGEX_LIB libboost_regex-vc140-mt-gd-1_63.lib HINTS "C:/Program Files/boost/lib64-msvc-14.0")
message("EXCEPTION_LIB: ${SERIALIZE_LIB}")
ENDIF (Windows)
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} "${SERIALIZE_LIB}" "${SYSTEM_LIB}" "${THREAD_LIB}" "${EXCEPTION_LIB}")

#set_target_properties(${PROJECT_NAME}
#    PROPERTIES
#    VERSION ${VERSION_MAJOR}.${VERSION_MINOR}
#    SOVERSION ${VERSION_MAJOR}
#    )
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES "${PROJECT_SOURCE_DIR}/config.json"
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}
    RENAME ${PROJECT_NAME}.json)
install(FILES "${PROJECT_SOURCE_DIR}/init.conf"
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/init
    RENAME ${PROJECT_NAME}.init)
install(FILES "${PROJECT_SOURCE_DIR}/init.override"
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/init
    RENAME ${PROJECT_NAME}.override)
