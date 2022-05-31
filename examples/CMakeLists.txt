add_executable (example-server ${CMAKE_CURRENT_SOURCE_DIR}/server.c)
target_link_libraries (example-server cachercise-server)

add_executable (example-admin ${CMAKE_CURRENT_SOURCE_DIR}/admin.c)
target_link_libraries (example-admin cachercise-admin)

add_executable (example-client ${CMAKE_CURRENT_SOURCE_DIR}/client.c)
target_link_libraries (example-client cachercise-client)

#add_executable (example-cachebench ${CMAKE_CURRENT_SOURCE_DIR}/cachebench.c)
#target_link_libraries (example-cachebench cachercise-client)
