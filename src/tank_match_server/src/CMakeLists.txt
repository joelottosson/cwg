cmake_minimum_required(VERSION 2.8)

set(proj_name tank_match_server)

project(${proj_name} CXX)
include($ENV{SAFIR_SDK}/data/build/safir_sdk_core_config.cmake)

file(GLOB headers *.h)
file(GLOB sources *.cpp)

add_executable(${proj_name} ${sources} ${headers} ${hpp})

find_package(Boost COMPONENTS system chrono filesystem)

if (NOT MSVC)
    #enable c++11
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    target_link_libraries(${proj_name}
                            swre_interface_cpp 
                            dose_cpp 
                            dots_generated-Consoden-cpp 
                            dots_generated-Safir-cpp dots_cpp 
                            logging_cpp lluf_utils 
                            ${Boost_LIBRARIES})
endif()

install(TARGETS ${proj_name}
        RUNTIME DESTINATION ${SAFIR_RUNTIME}/bin
        LIBRARY DESTINATION ${SAFIR_RUNTIME}/lib
        ARCHIVE DESTINATION ${SAFIR_SDK}/lib)

INSTALL_DEBUG_INFO(${proj_name})

