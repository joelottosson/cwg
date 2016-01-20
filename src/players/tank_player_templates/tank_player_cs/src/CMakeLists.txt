cmake_minimum_required(VERSION 2.8)

set(proj_name tank_player_csharp)
project(${proj_name} CXX)

INCLUDE($ENV{SAFIR_SDK}/data/build/safir_sdk_core_config.cmake)
find_package(CSharp REQUIRED)
INCLUDE(CSharpMacros)

FILE(GLOB sources *.cs)

SET(CS_FLAGS ${COMMON_CS_FLAGS} "-r:${SAFIR_RUNTIME}/bin/Safir.Dob.Typesystem.dll,${SAFIR_RUNTIME}/bin/Safir.Dob.dll,${SAFIR_RUNTIME}/bin/dots_generated-dotnet.dll,${SAFIR_RUNTIME}/bin/Safir.SwReports.dll,${SAFIR_RUNTIME}/bin/Safir.Logging.dll" )

ADD_CS_EXECUTABLE(${proj_name} "${sources}" ALL)

#The csharp build renames the exe to XXXX.csexe, so we rename it back when we install
INSTALL(PROGRAMS ${target_EXE} 
 DESTINATION ${SAFIR_RUNTIME}/bin
  RENAME ${proj_name}.exe)

INSTALL_CS_DEBUG_INFO()

