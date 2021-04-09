add_executable (example-server ${CMAKE_CURRENT_SOURCE_DIR}/server.c)
target_link_libraries (example-server reducer-client reducer-server)

add_executable (example-admin ${CMAKE_CURRENT_SOURCE_DIR}/admin.c)
target_link_libraries (example-admin reducer-admin)

add_executable (example-client ${CMAKE_CURRENT_SOURCE_DIR}/client.c)
target_link_libraries (example-client reducer-server reducer-client)
