cmake_minimum_required(VERSION 2.8)
project(tank_engine CXX)

INCLUDE($ENV{SAFIR_SDK}/data/build/safir_sdk_core_config.cmake)

foreach(flag ${MYFLAGS})
  message("Adding define for ${flag}")
  add_definitions(-D${flag})
endforeach()

FILE(GLOB sources *.cpp)
FILE(GLOB headers *.h)

ADD_EXECUTABLE(tank_engine ${sources} ${headers})

FIND_PACKAGE(Boost COMPONENTS system)
TARGET_LINK_LIBRARIES(tank_engine ${Boost_LIBRARIES})
  
if (NOT MSVC)
  TARGET_LINK_LIBRARIES(tank_engine swre_interface_cpp dose_cpp dots_generated-Consoden-cpp dots_generated-Safir-cpp dots_cpp logging_cpp lluf_utils)
endif()

INSTALL(TARGETS tank_engine
  RUNTIME DESTINATION ${SAFIR_RUNTIME}/bin
  LIBRARY DESTINATION ${SAFIR_RUNTIME}/lib
  ARCHIVE DESTINATION ${SAFIR_SDK}/lib)

INSTALL_DEBUG_INFO(tank_engine)
