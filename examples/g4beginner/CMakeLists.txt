#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.8...3.18)
if(${CMAKE_VERSION} VERSION_LESS 3.12)
  cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()
project(B1)

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
add_executable(exampleB1 exampleB1.cc ${sources} ${headers})
target_link_libraries(exampleB1 ${Geant4_LIBRARIES} ${G4CMP_LIBRARIES})

#----------------------------------------------------------------------------
# Copy all scripts to the build directory, i.e. the directory in which we
# build B1. This is so that we can run the executable directly because it
# relies on these scripts being in the current working directory.
#
set(EXAMPLEB1_MACROS
  g4cmp_electrons.mac
  g4cmp_electrons_100um.mac
  g4cmp_electrons_300um.mac
  g4cmp_phonons.mac
  g4cmp_phonons_100um.mac
  g4cmp_phonons_300um.mac
  init_vis.mac
)
set(EXAMPLEB1_SCRIPTS
  g4cmp_electrons_interface_hits.C
  g4cmp_phonons_interface_hits.C
)

foreach(_script ${EXAMPLEB1_SCRIPTS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/scripts/${_script}
    ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
    )
endforeach()

foreach(_macro ${EXAMPLEB1_MACROS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/G4Macros/${_macro}
    ${PROJECT_BINARY_DIR}/${_macro}
    COPYONLY
    )
endforeach()

#----------------------------------------------------------------------------
# For internal Geant4 use - but has no effect if you build this
# example standalone
#
add_custom_target(B1 DEPENDS exampleB1)

#----------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
#
install(TARGETS exampleB1 DESTINATION bin)
