add_executable (server ${CMAKE_CURRENT_SOURCE_DIR}/server.c)
target_link_libraries (server reducer-server)

if(${ENABLE_SYMBIOMON})
  add_definitions("-DUSE_SYMBIOMON")
  target_include_directories (server PUBLIC ${SYMBIOMON_INCLUDE_DIR})
  target_link_libraries (server symbiomon-server)
endif(${ENABLE_SYMBIOMON})

install (TARGETS server DESTINATION bin)
