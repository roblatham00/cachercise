# set source files
set (server-src-files
     provider.c)

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
set (cachercize-pkg "share/cmake/cachercize")

# set version fo shared library
set (cachercize-vers "${CACHERCIZE_VERSION_MAJOR}.${CACHERCIZE_VERSION_MINOR}")

# server library
add_library (cachercize-server ${server-src-files} ${dummy-src-files})
target_link_libraries (cachercize-server
    PkgConfig::MARGO
    PkgConfig::ABTIO
    PkgConfig::UUID
    PkgConfig::JSONC)
target_include_directories (cachercize-server PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories (cachercize-server BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)
target_include_directories (cachercize-server BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
set_target_properties (cachercize-server
    PROPERTIES VERSION ${CACHERCIZE_VERSION}
    SOVERSION ${CACHERCIZE_VERSION_MAJOR})

# client library
add_library (cachercize-client ${client-src-files})
target_link_libraries (cachercize-client PkgConfig::MARGO PkgConfig::UUID)
target_include_directories (cachercize-client PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories (cachercize-client BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)
target_include_directories (cachercize-client BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
set_target_properties (cachercize-client
    PROPERTIES VERSION ${CACHERCIZE_VERSION}
    SOVERSION ${CACHERCIZE_VERSION_MAJOR})

# admin library
add_library (cachercize-admin ${admin-src-files})
target_link_libraries (cachercize-admin PkgConfig::MARGO PkgConfig::UUID)
target_include_directories (cachercize-admin PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories (cachercize-admin BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)
target_include_directories (cachercize-admin BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
set_target_properties (cachercize-admin
    PROPERTIES VERSION ${CACHERCIZE_VERSION}
    SOVERSION ${CACHERCIZE_VERSION_MAJOR})

if (${ENABLE_BEDROCK})
# bedrock-module library
add_library (cachercize-bedrock-module ${bedrock-module-src-files})
target_link_libraries (cachercize-bedrock-module cachercize-server cachercize-admin cachercize-client bedrock-server)
target_include_directories (cachercize-bedrock-module PUBLIC $<INSTALL_INTERFACE:include>)
target_include_directories (cachercize-bedrock-module BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>)
target_include_directories (cachercize-bedrock-module BEFORE PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)
set_target_properties (cachercize-bedrock-module
    PROPERTIES VERSION ${CACHERCIZE_VERSION}
    SOVERSION ${CACHERCIZE_VERSION_MAJOR})
endif ()

# installation stuff (packaging and install commands)
write_basic_package_version_file(
    "cachercize-config-version.cmake"
    VERSION ${CACHERCIZE_VERSION}
    COMPATIBILITY AnyNewerVersion)

# generate our config file for find_package()
set(INCLUDE_INSTALL_DIR include/)
set(LIB_INSTALL_DIR lib/)
configure_package_config_file(cachercize-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/cachercize-config.cmake
  INSTALL_DESTINATION ${LIB_INSTALL_DIR}/cachercize/cmake
  PATH_VARS INCLUDE_INSTALL_DIR)

# some bits for the pkg-config file
set (DEST_DIR "${CMAKE_INSTALL_PREFIX}")
set (SERVER_PRIVATE_LIBS "-lcachercize-server")
set (CLIENT_PRIVATE_LIBS "-lcachercize-client")
set (ADMIN_PRIVATE_LIBS  "-lcachercize-admin")
configure_file ("cachercize-server.pc.in" "cachercize-server.pc" @ONLY)
configure_file ("cachercize-client.pc.in" "cachercize-client.pc" @ONLY)
configure_file ("cachercize-admin.pc.in"  "cachercize-admin.pc"  @ONLY)

# configure config.h
configure_file ("config.h.in" "config.h" @ONLY)

# "make install" rules
install (TARGETS cachercize-server cachercize-client cachercize-admin
         EXPORT cachercize-targets
         ARCHIVE DESTINATION lib
         LIBRARY DESTINATION lib)
if (${ENABLE_BEDROCK})
    install (TARGETS cachercize-bedrock-module
             ARCHIVE DESTINATION lib
             LIBRARY DESTINATION lib)
endif ()
install (EXPORT cachercize-targets
         DESTINATION ${cachercize-pkg}
         FILE "cachercize-targets.cmake")
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/cachercize-config.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/cachercize-config-version.cmake"
         DESTINATION ${cachercize-pkg} )
install (DIRECTORY ../include/cachercize
         DESTINATION include
         FILES_MATCHING PATTERN "*.h")
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/cachercize-server.pc"
               "${CMAKE_CURRENT_BINARY_DIR}/cachercize-client.pc"
               "${CMAKE_CURRENT_BINARY_DIR}/cachercize-admin.pc"
         DESTINATION "lib/pkgconfig/")
