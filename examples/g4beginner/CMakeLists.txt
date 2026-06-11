#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.10)
if(${CMAKE_VERSION} VERSION_LESS 3.12)
  cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()
set(PROJ_NAME g4beginner)
project(g4beginner)

if (NOT DEFINED MACRO_DIR)
  set(MACRO_DIR ${PROJECT_SOURCE_DIR}/G4Macros)
endif()
#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# You can set WITH_GEANT4_UIVIS to OFF via the command line or ccmake/cmake-gui
# to build a batch mode only executable
#
option(WITH_GEANT4_UIVIS "Build example with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 REQUIRED)
endif()
find_package(G4CMP REQUIRED)

# Geant4 11+ requires C++17 or greater
if (Geant4_VERSION_MAJOR GREATER_EQUAL 11)
  if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
  endif()
  if (CMAKE_CXX_STANDARD LESS 17)
    set(CMAKE_CXX_STANDARD 17)
  endif()
endif()

#----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
# Setup include directory for this project
#
include(${Geant4_USE_FILE})
include(${G4CMP_USE_FILE})
include_directories(${PROJECT_SOURCE_DIR}/include)

#----------------------------------------------------------------------------
# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
#
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
#
add_executable(${PROJ_NAME} ${PROJ_NAME}.cc ${sources} ${headers})
target_link_libraries(${PROJ_NAME} ${Geant4_LIBRARIES} ${G4CMP_LIBRARIES})
# Setting this variable allows us to set a base directory path for macro files
target_compile_definitions(${PROJ_NAME} PUBLIC MACRO_DIR="${MACRO_DIR}")

#----------------------------------------------------------------------------
# Copy all scripts to the build directory, i.e. the directory in which we
# build g4beginner. This is so that we can run the executable directly because it
# relies on these scripts being in the current working directory.
set(G4BEGINNER_SCRIPTS
  g4cmp_electrons_interface_hits.C
  g4cmp_phonons_interface_hits.C
  g4cmp_phonons_pairbreaking_summary.C
)
if (NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR})
endif()
# check to see if CMAKE_INSTALL_PREFIX is writable
set(TEST_PATH "${CMAKE_INSTALL_PREFIX}/.cmake_write_test")
execute_process(COMMAND ${CMAKE_COMMAND} -E touch "${TEST_PATH}" RESULT_VARIABLE WRITE_RESULT ERROR_QUIET)
if (WRITE_RESULT EQUAL 0)
  file(REMOVE "${TEST_PATH}")
endif()
if (NOT EXISTS ${CMAKE_INSTALL_PREFIX}/bin)
  if (WRITE_RESULT EQUAL 0)
    file(MAKE_DIRECTORY "${CMAKE_INSTALL_PREFIX}/bin")
  endif()
endif()

foreach(_script ${G4BEGINNER_SCRIPTS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/scripts/${_script}
    ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
    )
  if (WRITE_RESULT EQUAL 0)
    configure_file(
      ${PROJECT_SOURCE_DIR}/scripts/${_script}
      ${CMAKE_INSTALL_PREFIX}/bin/${_script}
      COPYONLY
      )
  endif()
endforeach()

# foreach(_macro ${G4BEGINNER_MACROS})
#   configure_file(
#     ${PROJECT_SOURCE_DIR}/G4Macros/${_macro}
#     ${MACRO_DIR}/${_macro}
#     COPYONLY
#     )
# endforeach()

#----------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
#
install(TARGETS ${PROJ_NAME} DESTINATION bin)
