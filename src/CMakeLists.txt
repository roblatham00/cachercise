# set source files
set (server-src-files
     provider.c
     hoard.cc)

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
set (cachercise-pkg "share/cmake/cachercise")

# set version fo shared library
set (cachercise-vers "${CACHERCISE_VERSION_MAJOR}.${CACHERCISE_VERSION_MINOR}")

# server library
add_library (cachercise-server ${server-src-files} ${dummy-src-files})
target_link_libraries (cachercise-server
    PkgConfig::MARGO
    PkgConfig::ABTIO
    PkgConfig::UUID
    PkgConfig::JSONC)
target_include_directories (cachercise-server PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories (cachercise-server BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)
target_include_directories (cachercise-server BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
set_target_properties (cachercise-server
    PROPERTIES VERSION ${CACHERCISE_VERSION}
    SOVERSION ${CACHERCISE_VERSION_MAJOR})

# client library
add_library (cachercise-client ${client-src-files})
target_link_libraries (cachercise-client PkgConfig::MARGO PkgConfig::UUID)
target_include_directories (cachercise-client PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories (cachercise-client BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)
target_include_directories (cachercise-client BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
set_target_properties (cachercise-client
    PROPERTIES VERSION ${CACHERCISE_VERSION}
    SOVERSION ${CACHERCISE_VERSION_MAJOR})

# admin library
add_library (cachercise-admin ${admin-src-files})
target_link_libraries (cachercise-admin PkgConfig::MARGO PkgConfig::UUID)
target_include_directories (cachercise-admin PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories (cachercise-admin BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)
target_include_directories (cachercise-admin BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
set_target_properties (cachercise-admin
    PROPERTIES VERSION ${CACHERCISE_VERSION}
    SOVERSION ${CACHERCISE_VERSION_MAJOR})

if (${ENABLE_BEDROCK})
# bedrock-module library
add_library (cachercise-bedrock-module ${bedrock-module-src-files})
target_link_libraries (cachercise-bedrock-module cachercise-server cachercise-admin cachercise-client bedrock-server)
target_include_directories (cachercise-bedrock-module PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories (cachercise-bedrock-module BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)
target_include_directories (cachercise-bedrock-module BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
set_target_properties (cachercise-bedrock-module
    PROPERTIES VERSION ${CACHERCISE_VERSION}
    SOVERSION ${CACHERCISE_VERSION_MAJOR})
endif ()

# installation stuff (packaging and install commands)
write_basic_package_version_file(
    "cachercise-config-version.cmake"
    VERSION ${CACHERCISE_VERSION}
    COMPATIBILITY AnyNewerVersion)

# generate our config file for find_package()
set(INCLUDE_INSTALL_DIR include/)
set(LIB_INSTALL_DIR lib/)
configure_package_config_file(cachercise-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/cachercise-config.cmake
  INSTALL_DESTINATION ${LIB_INSTALL_DIR}/cachercise/cmake
  PATH_VARS INCLUDE_INSTALL_DIR)

# some bits for the pkg-config file
set (DEST_DIR "${CMAKE_INSTALL_PREFIX}")
set (SERVER_PRIVATE_LIBS "-lcachercise-server")
set (CLIENT_PRIVATE_LIBS "-lcachercise-client")
set (ADMIN_PRIVATE_LIBS  "-lcachercise-admin")
configure_file ("cachercise-server.pc.in" "cachercise-server.pc" @ONLY)
configure_file ("cachercise-client.pc.in" "cachercise-client.pc" @ONLY)
configure_file ("cachercise-admin.pc.in"  "cachercise-admin.pc"  @ONLY)

# configure config.h
configure_file ("config.h.in" "config.h" @ONLY)

# "make install" rules
install (TARGETS cachercise-server cachercise-client cachercise-admin
         EXPORT cachercise-targets
         ARCHIVE DESTINATION lib
         LIBRARY DESTINATION lib)
if (${ENABLE_BEDROCK})
    install (TARGETS cachercise-bedrock-module
             ARCHIVE DESTINATION lib
             LIBRARY DESTINATION lib)
endif ()
install (EXPORT cachercise-targets
         DESTINATION ${cachercise-pkg}
         FILE "cachercise-targets.cmake")
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/cachercise-config.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/cachercise-config-version.cmake"
         DESTINATION ${cachercise-pkg} )
install (DIRECTORY ../include/cachercise
         DESTINATION include
         FILES_MATCHING PATTERN "*.h")
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/cachercise-server.pc"
               "${CMAKE_CURRENT_BINARY_DIR}/cachercise-client.pc"
               "${CMAKE_CURRENT_BINARY_DIR}/cachercise-admin.pc"
         DESTINATION "lib/pkgconfig/")
