#ifndef __TRANSPORT_TCP
#define __TRANSPORT_TCP

  #include <uv.h>
  #include <stdio.h>
  #include <stdlib.h>

  /**
   * Function declarations.
   */
  uv_buf_t alloc_buffer(uv_handle_t * handle, size_t size);
  void connection_cb(uv_stream_t * server, int status);
  void read_cb(uv_stream_t * stream, ssize_t nread, uv_buf_t buf);

#endif
