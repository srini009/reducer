add_executable (server ${CMAKE_CURRENT_SOURCE_DIR}/server.c)
target_link_libraries (server reducer-server)

install (TARGETS server DESTINATION bin)
