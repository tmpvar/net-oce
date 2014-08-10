#include "tcp.h"


/**
 * Callback which is executed on each new connection.
 */
void connection_cb(uv_stream_t * server, int status) {
  /* dynamically allocate a new client stream object on conn */
  uv_tcp_t * client = malloc(sizeof(uv_tcp_t));

  /* if status not zero there was an error */
  if (status == -1) {
    fprintf(stderr, "Error on listening: %s.\n",
      uv_strerror(uv_last_error(loop)));
  }

  /* initialize the new client */
  uv_tcp_init(loop, client);

  /* now let bind the client to the server to be used for incomings */
  if (uv_accept(server, (uv_stream_t *) client) == 0) {
    /* start reading from stream */
    int r = uv_read_start((uv_stream_t *) client, alloc_buffer, read_cb);

    if (r) {
      fprintf(stderr, "Error on reading client stream: %s.\n",
          uv_strerror(uv_last_error(loop)));
    }
  } else {
    /* close client stream on error */
    uv_close((uv_handle_t *) client, NULL);
  }
}

/**
 * Callback which is executed on each readable state.
 */
void read_cb(uv_stream_t * stream, ssize_t nread, uv_buf_t buf) {
  /* dynamically allocate memory for a new write task */
  uv_write_t * req = (uv_write_t *) malloc(sizeof(uv_write_t));

  /* if read bytes counter -1 there is an error or EOF */
  if (nread == -1) {
    if (uv_last_error(loop).code != UV_EOF) {
      fprintf(stderr, "Error on reading client stream: %s.\n",
          uv_strerror(uv_last_error(loop)));
    }

    uv_close((uv_handle_t *) stream, NULL);
  }

  /* write sync the incoming buffer to the socket */
  int r = uv_write(req, stream, &buf, 1, NULL);

  if (r) {
    fprintf(stderr, "Error on writing client stream: %s.\n",
        uv_strerror(uv_last_error(loop)));
  }

  /* free the remaining memory */
  free(buf.base);
}

/**
 * Allocates a buffer which we can use for reading.
 */
uv_buf_t alloc_buffer(uv_handle_t * handle, size_t size) {
    return uv_buf_init((char *) malloc(size), size);
}
