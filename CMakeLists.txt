cmake_minimum_required (VERSION 3.5)
include(GNUInstallDirs)
project (LiSpScPaRo)
#set (Boost_DEBUG 1)
find_package(Boost 1.63.0)
IF(NOT Boost_FOUND)
  message ("Boost compiler: ${Boost_COMPILER}")
  message (FATAL_ERROR "Boost not found!")
ENDIF(NOT Boost_FOUND)
SET(Boost_USE_STATIC_LIBS ON)
SET(Boost_USE_MULTITHREADED ON)
SET(Boost_USE_STATIC_RUNTIME OFF)

IF (DEFINED _DEBUG)
ADD_DEFINITIONS(-D_DEBUG=${_DEBUG})
ENDIF(DEFINED _DEBUG)
set (VERSION_MAJOR 1)
set (VERSION_MINOR 0)
set (Umbrella "ferryfair")
configure_file (
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
)

message(${CMAKE_SYSTEM_NAME})
# macOS specific code
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(macOS ON)
  set(ARCHITECTURE "Darwin")
endif ()

# Linux specific code
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(Linux ON)
  set(ARCHITECTURE "Linux")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

# Windows specific code
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(Windows ON)
  set(ARCHITECTURE "Win")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
set(ARCHITECTURE "${ARCHITECTURE}${CMAKE_SYSTEM_PROCESSOR}")

#file(GLOB HEADERS "*.h")
#file(GLOB SOURCES "*.cpp")
#list(FILTER HEADERS REGEX "^[^\.].+" output_variable HEADERS)
#list(FILTER SOURCES REGEX "^[^\.].+" output_variable SOURCES)
#message("HEADERS: " ${HEADERS})
#message("SOURCES: " ${SOURCES})
set (HEADERS config.h connect.h)
set (SOURCES LiSpScPaRo.cpp connect.cpp)
list (APPEND HEADERS "config.h.in")
set (GCC_COVERAGE_COMPILE_FLAGS "-std=c++14 -pthread")
set (GCC_COVERAGE_LINK_FLAGS "")

if (Linux)
  set (GCC_COVERAGE_LINK_FLAGS "${GCC_COVERAGE_LINK_FLAGS} -fPIC -Wl,-unresolved-symbols=ignore-in-shared-libs -static")
  set (BUILD_SHARED_LIBRARIES OFF)
endif (Linux)
if (Windows)
  set (BUILD_SHARED_LIBRARIES OFF)
endif (Windows)
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set (CMAKE_CXX_LINKER_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")
include_directories(${PROJECT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_INSTALL_FULL_INCLUDEDIR}/${Umbrella}
)
link_directories("${CMAKE_INSTALL_FULL_LIBDIR}")
message("Configurations")
foreach(CONFIGURATION IN LISTS CMAKE_CONFIGURATION_TYPES)
  message(${CONFIGURATION})
endforeach()
link_directories(
  ${Boost_LIBRARY_DIRS}
  "C:/Program Files/boost_1_65_1/lib64-msvc-14.1"
)
if(NOT LINKAGE)
  set(LINKAGE "STATIC")
  message("Building static by default")
else(NOT LINKAGE)
  string(TOUPPER "${LINKAGE}" LINKAGE)
endif()

if(CMAKE_CONFIGURATION_TYPES)
  message("Linking corresponding to the Configuration Name")
  link_directories("../ferrybase/build/$(ConfigurationName)")
  link_directories("../logger/build/$(ConfigurationName)")
  link_directories("../FFJSON/build/$(ConfigurationName)")
else(CMAKE_CONFIGURATION_TYPES)
  link_directories("../ferrybase/build")
  link_directories("../logger/build")
  link_directories("../FFJSON/build")
endif()
message("ARCHITECTURE: ${ARCHITECTURE}/${CMAKE_BUILD_TYPE}")
message("${Boost_LIBRARY_DIRS}${Boost_INCLUDE_DIRS}")
#IF (macOS)
#  include_directories("/usr/local/include")
#  link_directories("/usr/local/lib")
#  set(SERIALIZE_LIB boost_serialization)
#  set(SYSTEM_LIB boost_system)
#  set(THREAD_LIB boost_thread)
#  set(EXCEPTION_LIB boost_exception)
#ELSEIF (Linux)
#  include_directories("/usr/local/include")
#  link_directories("/usr/local/lib")
#  set(SERIALIZE_LIB boost_serialization)
#  set(SYSTEM_LIB boost_system)
#  set(THREAD_LIB boost_thread)
#  set(EXCEPTION_LIB boost_exception)
#ELSEIF (Windows)
#  IF (MINGW)
#    include_directories("C:/MinGW/include")
#    link_directories("C:/MinGW/lib")
#    set(SERIALIZE_LIB boost_serialization)
#    set(SYSTEM_LIB boost_system)
#    set(THREAD_LIB boost_thread)
#    set(EXCEPTION_LIB boost_exception)
#  ELSE (MINGW)
#    include_directories("C:/Program Files/boost")
#    link_directories("C:/Program Files/boost/lib64-msvc-14.0")
#    find_library(SERIALIZE_LIB libboost_serialization-vc140-mt-gd-1_63.lib HINTS "C:/Program Files/boost/lib64-msvc-14.0")
#    find_library(SYSTEM_LIB libboost_system-vc140-mt-gd-1_63.lib HINTS "C:/Program Files/boost/lib64-msvc-14.0")
#    find_library(THREAD_LIB libboost_thread-vc140-mt-gd-1_63.lib HINTS "C:/Program Files/boost/lib64-msvc-14.0")
#    find_library(EXCEPTION_LIB libboost_exception-vc140-mt-gd-1_63.lib HINTS "C:/Program Files/boost/lib64-msvc-14.0")
#    find_library(TIME_LIB libboost_date_time-vc140-mt-gd-1_63.lib HINTS "C:/Program Files/boost/lib64-msvc-14.0")
#    find_library(REGEX_LIB libboost_regex-vc140-mt-gd-1_63.lib HINTS "C:/Program Files/boost/lib64-msvc-14.0")
#  ENDIF (MINGW)
#ENDIF (macOS)
include_directories(${Boost_INCLUDE_DIRS} ".." "../FFJSON" "../logger")
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
#target_link_libraries(${PROJECT_NAME} "${SERIALIZE_LIB}" "${SYSTEM_LIB}" "${THREAD_LIB}" "${EXCEPTION_LIB}" FFJSON logger ferrybase)
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} FFJSON logger ferrybase)
#set_target_properties(${PROJECT_NAME}
#    PROPERTIES
#    VERSION ${VERSION_MAJOR}.${VERSION_MINOR}
#    SOVERSION ${VERSION_MAJOR}
#)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES "${PROJECT_SOURCE_DIR}/config.json"
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}
    RENAME ${PROJECT_NAME}.json
)
install(FILES "${PROJECT_SOURCE_DIR}/init.conf"
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/init
    RENAME ${PROJECT_NAME}.init
)
install(FILES "${PROJECT_SOURCE_DIR}/init.override"
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/init
    RENAME ${PROJECT_NAME}.override
)
