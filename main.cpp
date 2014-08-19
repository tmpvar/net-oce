#include <uv.h>
#include <stdio.h>
#include <stdlib.h>

#include "oce.pb.h"
#include "editor.h"

#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>

#include <Message.hxx>
#include <Message_Messenger.hxx>
#include <Standard_Macro.hxx>
#include <Message_PrinterOStream.hxx>

using namespace google;

uv_loop_t * loop;
uv_pipe_t stdin_pipe, stdout_pipe;
typedef struct {
  uv_write_t req;
  uv_buf_t buf;
} write_req_t;

NetOCE_Editor editor;

/* returns a buffer instance for storing incoming stdin lines */
void alloc_buffer(uv_handle_t *handle, size_t size, uv_buf_t* buf) {
  buf->base = (char *)malloc(size);
  buf->len = size;
}

void write_cb(uv_fs_t *req) {
  uv_fs_req_cleanup(req);
}

void read_stdin(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buffer) {
  // TODO: handle splits in buffer
  //       see: http://stackoverflow.com/questions/9496101/protocol-buffer-over-socket-in-c
  if (nread > -1) {
    if (buffer->base) {

      NetOCE_Request req;
      NetOCE_Response res;

      protobuf::io::ArrayInputStream arr(buffer->base, nread);
      protobuf::io::CodedInputStream input(&arr);

      req.ParseFromCodedStream(&input);

      if (editor.handleRequest(&req, &res)) {
        int size = res.ByteSize();
        char *bytes = (char *)malloc(size);
        bool result = res.SerializeToArray((void *)bytes, size);

        uv_fs_t write_req;

        uv_buf_t buf = uv_buf_init(bytes, size);

        uv_fs_write(
          uv_default_loop(),
          &write_req,
          1,
          &buf,
          1,
          -1 /*offset*/,
          write_cb
        );
      }

      free(buffer->base);
    }
  } else {
    // TODO: close up shop, this process is going down
  }
}

int main() {

  GOOGLE_PROTOBUF_VERIFY_VERSION;

  // turn off OCE's stdout printer
  Message::DefaultMessenger()->RemovePrinters(STANDARD_TYPE(Message_PrinterOStream));


  // kick off the uv event loop and rig up stdio
  loop = uv_default_loop();

  uv_pipe_init(loop, &stdin_pipe, 0);
  uv_pipe_open(&stdin_pipe, 0);

  // read from stdin
  uv_read_start((uv_stream_t*) &stdin_pipe, alloc_buffer, read_stdin);

  uv_run(loop, UV_RUN_DEFAULT);

  protobuf::ShutdownProtobufLibrary();

  return 0;
}
