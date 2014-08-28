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

#define HEADER_SIZE 4

using namespace google;

uv_loop_t * loop;
uv_pipe_t stdin_pipe, stdout_pipe;

NetOCE_Editor editor;

/* returns a buffer instance for storing incoming stdin lines */
void alloc_buffer(uv_handle_t *handle, size_t size, uv_buf_t* buf) {
  buf->base = (char *)malloc(size);
  buf->len = size;
}

void write_cb(uv_fs_t *req) {
  free(req->ptr); // is this the buffer?
  uv_fs_req_cleanup(req);
  free(req);
}


void respond(NetOCE_Response res) {
  uv_fs_t *write_req = (uv_fs_t *)malloc(sizeof(uv_fs_t));

  uint32_t size = res.ByteSize();
  char *bytes = (char *)malloc(size+HEADER_SIZE);
  bool result = res.SerializeToArray((void *)(bytes + HEADER_SIZE), size);

  fprintf(stderr, "write %zd bytes to stdout\n", size+HEADER_SIZE);

  memcpy(bytes, &size, HEADER_SIZE);

  uv_buf_t buf = uv_buf_init(bytes, size+4);

  uv_fs_write(uv_default_loop(), write_req, 1, &buf, 1, -1, write_cb);
}


char *scratch, *current;
int parser_state = 0;

uint32_t parser_message_size = 0, parser_message_location = 0, parser_message_size_location = 0;
uint8_t header_parts[4];

bool parse(uint8_t *buf, ssize_t size) {
  ssize_t diff, to_read;

  // get buffer, if state is 0 then we need to collect the buffer;
  switch (parser_state) {
    // collect size
    case 0:
      if (parser_message_size_location < HEADER_SIZE) {
        diff = HEADER_SIZE - parser_message_size_location;
        to_read = (size < diff) ? size : diff;
        for (ssize_t i = 0; i<to_read; i++) {
          header_parts[parser_message_size_location] = buf[0];
          buf++;
          parser_message_size_location++;
          size--;
        }

        if (parser_message_size_location < HEADER_SIZE) {
          return false;
        } else {
          memcpy(&parser_message_size, header_parts, 4);
        }
      }

      current = (char *)malloc(parser_message_size);

      parser_message_location = 0;
      parser_message_size_location = 0;

      // hop to the next state
      parser_state = 1;
      if (size > 0) {
        return parse(buf, size);
      } else {
        return false;
      }
    break;

    // collect current
    case 1:

      diff = parser_message_size - parser_message_location;
      to_read = (size > diff) ? diff : size;

      memcpy(current + parser_message_location, buf, to_read);

      parser_message_location += to_read;

      assert(parser_message_location <= parser_message_size);

      if (parser_message_location == parser_message_size) {

        NetOCE_Request req;
        NetOCE_Response res;

        req.ParseFromArray(current, parser_message_size);

        editor.handleRequest(&req, &res);
        respond(res);

        parser_message_location = 0;
        parser_message_size_location = 0;
        parser_message_size = 0;
        parser_state = 0;

        free(current);

        size -= to_read;
        buf += to_read;

        if (size > 0) {
          return parse((uint8_t *)buf, size);
        } else {
          return false;
        }
      }
    break;
  }

  return false;
}


void read_stdin(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buffer) {
  if (nread > -1) {
    if (buffer->base) {
      parse((uint8_t *)buffer->base, nread);
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
