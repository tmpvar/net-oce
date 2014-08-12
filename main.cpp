#include <uv.h>
#include <stdio.h>
#include <stdlib.h>

#include "oce.pb.h"
#include "editor.h"

#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>

using namespace google::protobuf;

uv_loop_t * loop;
uv_pipe_t stdin_pipe, stdout_pipe;
typedef struct {
  uv_write_t req;
  uv_buf_t buf;
} write_req_t;

NetOCE_Editor editor;

/* returns a buffer instance for storing incoming stdin lines */
uv_buf_t alloc_buffer(uv_handle_t *handle, size_t size) {
  return uv_buf_init((char *) malloc(size), size);
}

void write_cb(uv_fs_t *req) {
  uv_fs_req_cleanup(req);
}

void read_stdin(uv_stream_t *stream, ssize_t nread, uv_buf_t buffer) {
  // TODO: handle splits in buffer
  //       see: http://stackoverflow.com/questions/9496101/protocol-buffer-over-socket-in-c
  if (nread > -1) {
    if (buffer.base) {

      NetOCE_Request req;
      NetOCE_Response res;

      io::ArrayInputStream arr(buffer.base, nread);
      io::CodedInputStream input(&arr);

      req.ParseFromCodedStream(&input);

      if (editor.handleRequest(&req, &res)) {
        int size = res.ByteSize();
        char *bytes = (char *)malloc(size);
        bool result = res.SerializeToArray((void *)bytes, size);

        uv_fs_t write_req;

        uv_fs_write(
          uv_default_loop(),
          &write_req,
          1,
          bytes,
          size,
          -1 /*offset*/,
          write_cb
        );
      }

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

  // read from stdin
  uv_read_start((uv_stream_t*) &stdin_pipe, alloc_buffer, read_stdin);

  uv_run(loop, UV_RUN_DEFAULT);

  ShutdownProtobufLibrary();

  return 0;
}
