add_executable (
  dinner
  dinner.c
  dn_buffer.c
  dn_cgi.c
  dn_event.c
  dn_file.c
  dn_io.c
  dn_log.c
  dn_request.c
  dn_response.c
)
target_link_libraries (dinner pthread)

