cmake_minimum_required(VERSION 2.8)
project(config CXX)

INCLUDE($ENV{SAFIR_SDK}/data/build/safir_sdk_core_config.cmake)

foreach(flag ${MYFLAGS})
  message("Adding define for ${flag}")
  add_definitions(-D${flag})
endforeach()

if (NOT MSVC)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wno-write-strings")
endif()
FILE(GLOB sources *.cpp)
FILE(GLOB headers *.h)


ADD_LIBRARY(SHARED ${sources} ${headers})


FIND_PACKAGE(Boost COMPONENTS system)
#TARGET_LINK_LIBRARIES(${Boost_LIBRARIES})
  
if (NOT MSVC)
  TARGET_LINK_LIBRARIES(swre_interface_cpp dose_cpp dots_generated-Consoden-cpp dots_generated-Safir-cpp dots_cpp logging_cpp lluf_utils)
endif()

INSTALL(TARGETS config
        RUNTIME DESTINATION ${SAFIR_RUNTIME}/bin
        LIBRARY DESTINATION ${SAFIR_RUNTIME}/lib
        ARCHIVE DESTINATION ${SAFIR_SDK}/lib)

INSTALL(FILES rules.cfg DESTINATION ${SAFIR_RUNTIME}/data/tank_game)

