#include <uv.h>
#include <stdio.h>
#include <stdlib.h>

#include "transport/tcp.h"

/**
 * Our tcp server object.
 */
uv_tcp_t server;

/**
 * Shared reference to our event loop.
 */
uv_loop_t * loop;

int main() {
  loop = uv_default_loop();

  /* convert a humanreadable ip address to a c struct */
  struct sockaddr_in addr = uv_ip4_addr("127.0.0.1", 3000);

  /* initialize the server */
  uv_tcp_init(loop, &server);
  /* bind the server to the address above */
  uv_tcp_bind(&server, addr);

  /* let the server listen on the address for new connections */
  int r = uv_listen((uv_stream_t *) &server, 128, connection_cb);

  if (r) {
    return fprintf(stderr, "Error on listening: %s.\n",
        uv_strerror(uv_last_error(loop)));
  }

  /* execute all tasks in queue */
  return uv_run(loop, UV_RUN_DEFAULT);
}
