cmake_minimum_required(VERSION 2.8)

set(proj_name tank_player_grandmother)

project(${proj_name} CXX)
include($ENV{SAFIR_SDK}/data/build/safir_sdk_core_config.cmake)

if (NOT MSVC)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -std=c++0x")
endif()

file(GLOB headers *.h)
file(GLOB sources *.cpp)
add_executable(${proj_name} ${sources} ${headers})

find_package(Boost COMPONENTS system chrono)

if (NOT MSVC)
    target_link_libraries(${proj_name} swre_interface_cpp dose_cpp dots_generated-Safir-cpp dots_generated-Consoden-cpp dots_cpp logging_cpp ${Boost_LIBRARIES})
endif()

install(TARGETS ${proj_name}
        RUNTIME DESTINATION ${SAFIR_RUNTIME}/bin
        LIBRARY DESTINATION ${SAFIR_RUNTIME}/lib
        ARCHIVE DESTINATION ${SAFIR_SDK}/lib)

INSTALL_DEBUG_INFO(${proj_name})

