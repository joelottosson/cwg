cmake_minimum_required(VERSION 2.8)

project(tank_game_gui C CXX) #FindQt needs C

INCLUDE("$ENV{SAFIR_SDK}/data/build/safir_sdk_core_config.cmake")

if (NOT MSVC)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()

foreach(flag ${MYFLAGS})
  message("Adding define for ${flag}")
  add_definitions(-D${flag})
endforeach()

FILE(GLOB qrc *.qrc)
FILE(GLOB ui *.ui)
FILE(GLOB sources *.cpp)
FILE(GLOB headers [!ui_]*.h)
#FILE(GLOB sounds ../sounds/*.mp3)
#message(" sounds ${sounds}")

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# We default to Qt5 if we can find it and fall back to Qt4

if (CMAKE_VERSION VERSION_GREATER "2.8.10") #need cmake 2.8.11 or greater for qt5
  find_package(Qt5Widgets QUIET)
  find_package(Qt5Multimedia QUIET)
endif()

if (Qt5Widgets_FOUND)
  message("-- Building tank_game with Qt5")
  set(CMAKE_AUTOMOC ON)   
  QT5_WRAP_UI(ui_headers ${ui})
  QT5_ADD_RESOURCES(qrc_src ${qrc})
else() 
  # Try to use Qt4 instead
  SET(QT_USE_QTMAIN TRUE)
  find_package(Qt4 REQUIRED)

  message("-- Building tank_game with Qt4")

  include(${QT_USE_FILE})

  QT4_ADD_RESOURCES(qrc_src ${qrc})

  QT4_WRAP_UI(ui_headers ${ui})
   
  #qt 4.7 and 4.8 have a bug where moc will fail to parse some stuff in boost
  #so we have a test.h file which we try moc on, and if it fails we add a 
  #workaround. See https://bugreports.qt-project.org/browse/QTBUG-22829
  execute_process(COMMAND moc -o moc_test.cxx test.h
    OUTPUT_VARIABLE moctest_output
    ERROR_VARIABLE moctest_output
    RESULT_VARIABLE moctest_result
    WORKING_DIRECTORY moctest)
  
  if (moctest_result) #if test failed
    set(QT4_WRAP_CPP_OPTIONS OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED)
  endif()
  
  QT4_WRAP_CPP(moc_sources ${headers} ${QT4_WRAP_CPP_OPTIONS})

endif()

ADD_EXECUTABLE(tank_game_gui WIN32 ${sources} ${ui_headers} ${moc_sources} ${qrc_src} ${headers})

if (Qt5Widgets_FOUND)
  qt5_use_modules(tank_game_gui Widgets Multimedia)
  QT5_WRAP_CPP(moc_sources ${headers} ${QT5_WRAP_CPP_OPTIONS})
else()
  TARGET_LINK_LIBRARIES(tank_game_gui ${QT_LIBRARIES})
endif()

FIND_PACKAGE(Boost COMPONENTS filesystem system)

IF (NOT MSVC)
  TARGET_LINK_LIBRARIES(tank_game_gui libconfig.so dots_cpp dots_generated-Safir-cpp dots_generated-cpp dots_generated-Consoden-cpp dose_internal dose_cpp lluf_utils lluf_crash_reporter ${Boost_LIBRARIES})
ENDIF ()

INSTALL(TARGETS tank_game_gui
  RUNTIME DESTINATION ${SAFIR_RUNTIME}/bin
  LIBRARY DESTINATION ${SAFIR_RUNTIME}/lib
  ARCHIVE DESTINATION ${SAFIR_SDK}/lib)
INSTALL_DEBUG_INFO(tank_game_gui)
INSTALL(DIRECTORY ../sounds DESTINATION ${SAFIR_RUNTIME}/data/tank_game)
