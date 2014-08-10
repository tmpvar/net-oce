#include <uv.h>
#include <stdio.h>
#include <stdlib.h>

#include "test.pb.h"

#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>

using namespace google::protobuf;

uv_loop_t * loop;
uv_pipe_t stdin_pipe, stdout_pipe;

/* returns a buffer instance for storing incoming stdin lines */
uv_buf_t alloc_buffer(uv_handle_t *handle, size_t size) {
  return uv_buf_init((char *) malloc(size), size);
}

uint32_t packet_size = 0;

void read_stdin(uv_stream_t *stream, ssize_t nread, uv_buf_t buffer) {
  // TODO: handle splits in buffer
  //       see: http://stackoverflow.com/questions/9496101/protocol-buffer-over-socket-in-c

  if (nread > -1) {
    if (packet_size == 0) {
      memcpy(&packet_size, buffer.base, 4);

    } else {

      Person p;

      io::ArrayInputStream arr(buffer.base, nread);
      io::CodedInputStream input(&arr);

      p.ParseFromCodedStream(&input);

      printf("result: \n%s", p.DebugString().c_str());

    }

    // TODO: only free after expending the entire buffer
    if (buffer.base) {
      free(buffer.base);
    }
  } else {
    // TODO: close up shop, this process is going down
  }

}

int main() {

  GOOGLE_PROTOBUF_VERIFY_VERSION;

  // kick off the uv event loop and rig up stdio
  loop = uv_default_loop();

  uv_pipe_init(loop, &stdin_pipe, 0);
  uv_pipe_open(&stdin_pipe, 0);

  uv_pipe_init(loop, &stdout_pipe, 0);
  uv_pipe_open(&stdout_pipe, 1);

  // read from stdin
  uv_read_start((uv_stream_t*) &stdin_pipe, alloc_buffer, read_stdin);

  uv_run(loop, UV_RUN_DEFAULT);
}
