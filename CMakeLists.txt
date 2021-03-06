# Defines ChainBase library target.
project( ChainBase )
cmake_minimum_required( VERSION 3.5 )

#list( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules" )

set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")

SET( Boost_USE_STATIC_LIBS ON CACHE STRING "ON or OFF" )

include( GNUInstallDirs )

IF( WIN32 )
  SET(BOOST_ROOT $ENV{BOOST_ROOT})
  set(Boost_USE_MULTITHREADED ON)
  set(BOOST_ALL_DYN_LINK OFF) # force dynamic linking for all libraries
ENDIF(WIN32)

FIND_PACKAGE(Boost 1.70 REQUIRED COMPONENTS filesystem program_options unit_test_framework)

SET(PLATFORM_LIBRARIES)
FIND_LIBRARY(ROCKSDB_LIBRARY rocksdb HINTS /usr/local/lib/)

if(CMAKE_CXX_STANDARD EQUAL 98)
   message(FATAL_ERROR "chainbase requires c++14 or newer")
elseif(NOT CMAKE_CXX_STANDARD)
   set(CMAKE_CXX_STANDARD 17)
   set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

if( APPLE )
  # Apple Specific Options Here
  message( STATUS "Configuring ChainBase on OS X" )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-conversion" )
else( APPLE )
  # Linux Specific Options Here
  message( STATUS "Configuring ChainBase on Linux" )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall" )
  if ( FULL_STATIC_BUILD )
    set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc")
  endif ( FULL_STATIC_BUILD )
  LIST( APPEND PLATFORM_LIBRARIES pthread )
endif( APPLE )

if( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-builtin-memcmp" )
endif()

if( "${CMAKE_GENERATOR}" STREQUAL "Ninja" )
  if( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )
      set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics" )
  endif()
endif()

# based on http://www.delorie.com/gnu/docs/gdb/gdb_70.html
# uncomment this line to tell GDB about macros (slows compile times)
# set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -gdwarf-2 -g3" )

set(ENABLE_COVERAGE_TESTING FALSE CACHE BOOL "Build ChainBase for code coverage analysis")

if(ENABLE_COVERAGE_TESTING)
    SET(CMAKE_CXX_FLAGS "--coverage ${CMAKE_CXX_FLAGS}")
endif()


file( GLOB HEADERS "include/chainbase/*.hpp" "include/chainrocks/*.hpp" )

add_library( chainbase src/chainbase.cpp src/pinnable_mapped_file.cpp ${HEADERS} )
target_link_libraries( chainbase Boost::filesystem ${PLATFORM_LIBRARIES} )
target_include_directories( chainbase PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

add_library( chainrocks src/database.cpp src/rocksdb_backend.cpp src/rocksdb_datum.cpp src/rocksdb_options.cpp src/undo_state.cpp ${HEADERS} )
target_link_libraries( chainrocks Boost::filesystem ${ROCKSDB_LIBRARY} ${PLATFORM_LIBRARIES} )
target_include_directories( chainrocks PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/aux/plot.gp" "${CMAKE_CURRENT_BINARY_DIR}/bench/plot.gp" COPYONLY)

if(WIN32)
   target_link_libraries( chainbase ws2_32 mswsock )
endif()

# add_subdirectory( bench )
# add_subdirectory( test )
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/chainbase DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR})

install(TARGETS chainbase
   LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR} 
ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})

