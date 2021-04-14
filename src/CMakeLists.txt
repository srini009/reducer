# set source files
set (server-src-files
     provider.cc)

set (client-src-files
     client.c)

set (admin-src-files
     admin.c)

set (dummy-src-files
     dummy/dummy-backend.c)

set (bedrock-module-src-files
     bedrock-module.c)

# load package helper for generating cmake CONFIG packages
include (CMakePackageConfigHelpers)

# where to install files for "find_package"
set (reducer-pkg "share/cmake/reducer")

# set version fo shared library
set (reducer-vers "${REDUCER_VERSION_MAJOR}.${REDUCER_VERSION_MINOR}")

if(${ENABLE_AGGREGATOR})
  add_definitions("-DUSE_AGGREGATOR")
endif(${ENABLE_AGGREGATOR})

if(${ENABLE_SYMBIOMON})
  add_definitions("-DUSE_SYMBIOMON")
endif(${ENABLE_SYMBIOMON})

# server library
add_library (reducer-server ${server-src-files} ${dummy-src-files})
target_link_libraries (reducer-server
    PkgConfig::MARGO
    PkgConfig::ABTIO
    PkgConfig::UUID)
#    PkgConfig::JSONC)
target_include_directories (reducer-server PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories (reducer-server BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)
target_include_directories (reducer-server BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
set_target_properties (reducer-server
    PROPERTIES VERSION ${REDUCER_VERSION}
    SOVERSION ${REDUCER_VERSION_MAJOR})

# client library
add_library (reducer-client ${client-src-files})
target_link_libraries (reducer-client PkgConfig::MARGO PkgConfig::UUID)
target_include_directories (reducer-client PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories (reducer-client BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)
target_include_directories (reducer-client BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
set_target_properties (reducer-client
    PROPERTIES VERSION ${REDUCER_VERSION}
    SOVERSION ${REDUCER_VERSION_MAJOR})

if(${ENABLE_AGGREGATOR})
  target_include_directories (reducer-server PUBLIC ${SDSKV_INCLUDE_DIR})
  target_link_libraries (reducer-server PkgConfig::SDSKV)
endif(${ENABLE_AGGREGATOR})

if(${ENABLE_SYMBIOMON})
  target_include_directories (reducer-server PUBLIC ${SYMBIOMON_INCLUDE_DIR})
  target_link_libraries (reducer-server PkgConfig::SYMBIOMON)
endif(${ENABLE_SYMBIOMON})

# admin library
add_library (reducer-admin ${admin-src-files})
target_link_libraries (reducer-admin PkgConfig::MARGO PkgConfig::UUID)
target_include_directories (reducer-admin PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories (reducer-admin BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)
target_include_directories (reducer-admin BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
set_target_properties (reducer-admin
    PROPERTIES VERSION ${REDUCER_VERSION}
    SOVERSION ${REDUCER_VERSION_MAJOR})

if (${ENABLE_BEDROCK})
# bedrock-module library
add_library (reducer-bedrock-module ${bedrock-module-src-files})
target_link_libraries (reducer-bedrock-module reducer-server reducer-admin reducer-client)
target_include_directories (reducer-bedrock-module PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories (reducer-bedrock-module BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)
target_include_directories (reducer-bedrock-module BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
set_target_properties (reducer-bedrock-module
    PROPERTIES VERSION ${REDUCER_VERSION}
    SOVERSION ${REDUCER_VERSION_MAJOR})
endif ()

# installation stuff (packaging and install commands)
write_basic_package_version_file(
    "reducer-config-version.cmake"
    VERSION ${REDUCER_VERSION}
    COMPATIBILITY AnyNewerVersion)

# generate our config file for find_package()
set(INCLUDE_INSTALL_DIR include/)
set(LIB_INSTALL_DIR lib/)
configure_package_config_file(reducer-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/reducer-config.cmake
  INSTALL_DESTINATION ${LIB_INSTALL_DIR}/reducer/cmake
  PATH_VARS INCLUDE_INSTALL_DIR)

# some bits for the pkg-config file
set (DEST_DIR "${CMAKE_INSTALL_PREFIX}")
set (SERVER_PRIVATE_LIBS "-lreducer-server")
set (CLIENT_PRIVATE_LIBS "-lreducer-client")
set (ADMIN_PRIVATE_LIBS  "-lreducer-admin")
configure_file ("reducer-server.pc.in" "reducer-server.pc" @ONLY)
configure_file ("reducer-client.pc.in" "reducer-client.pc" @ONLY)
configure_file ("reducer-admin.pc.in"  "reducer-admin.pc"  @ONLY)

# configure config.h
configure_file ("config.h.in" "config.h" @ONLY)

# "make install" rules
install (TARGETS reducer-server reducer-client reducer-admin
         EXPORT reducer-targets
         ARCHIVE DESTINATION lib
         LIBRARY DESTINATION lib)
if (${ENABLE_BEDROCK})
    install (TARGETS reducer-bedrock-module
             ARCHIVE DESTINATION lib
             LIBRARY DESTINATION lib)
endif ()
install (EXPORT reducer-targets
         DESTINATION ${reducer-pkg}
         FILE "reducer-targets.cmake")
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/reducer-config.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/reducer-config-version.cmake"
         DESTINATION ${reducer-pkg} )
install (DIRECTORY ../include/reducer
         DESTINATION include
         FILES_MATCHING PATTERN "*.h")
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/reducer-server.pc"
               "${CMAKE_CURRENT_BINARY_DIR}/reducer-client.pc"
               "${CMAKE_CURRENT_BINARY_DIR}/reducer-admin.pc"
         DESTINATION "lib/pkgconfig/")
