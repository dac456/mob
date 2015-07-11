cmake_minimum_required(VERSION 2.8)
project(mob)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules")

add_definitions( -DMOBAPI_EXPORT )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Boost
unset(Boost_INCLUDE_DIR CACHE)
unset(Boost_LIBRARY_DIRS CACHE)

find_package(Boost REQUIRED system filesystem thread serialization)
link_directories(${Boost_LIBRARY_DIRS})
include_directories(${Boost_INCLUDE_DIR})

# libmob
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libmob/include)

file(GLOB_RECURSE LIB_HDR RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} FOLLOW_SYMLINKS libmob/include/*.h)
file(GLOB_RECURSE LIB_SRC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} FOLLOW_SYMLINKS libmob/src/*.cpp)

add_library(mob SHARED ${LIB_SRC} ${LIB_HDR})

target_link_libraries(mob
   ${Boost_LIBRARIES}
   rt #TODO: only if linux
)

# mobnode
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/mobnode/include)

file(GLOB_RECURSE NODE_HDR RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} FOLLOW_SYMLINKS mobnode/include/*.h)
file(GLOB_RECURSE NODE_SRC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} FOLLOW_SYMLINKS mobnode/src/*.cpp)

add_executable(mobnode ${NODE_SRC} ${NODE_HDR})

target_link_libraries(mobnode
   ${Boost_LIBRARIES}
   mob
)

# mobtest
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/mobtest/include)

file(GLOB_RECURSE TEST_HDR RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} FOLLOW_SYMLINKS mobtest/include/*.h)
file(GLOB_RECURSE TEST_SRC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} FOLLOW_SYMLINKS mobtest/src/*.cpp)

add_executable(mobtest ${TEST_SRC} ${TEST_HDR})

target_link_libraries(mobtest
   ${Boost_LIBRARIES}
   mob
)