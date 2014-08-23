#include "editor.h"
#include "handler.h"

NetOCE_Editor::NetOCE_Editor() {
  this->shapes = new unordered_map<uint32_t, TopoDS_Shape>();
  this->shape_index = 0;
}

NetOCE_Editor::~NetOCE_Editor() {
  // TODO: cleanup shapes
  delete this->shapes;
}

void NetOCE_Editor::reset() {
  unordered_map<uint32_t, TopoDS_Shape>::iterator it = this->shapes->begin();
  for (; it != this->shapes->end(); ++it) {
    printf("clean\n");
  }


  this->shape_index = 0;
  this->shapes->clear();
}

bool NetOCE_Editor::handleRequest(NetOCE_Request *request, NetOCE_Response *response) {
  // TODO: typechecking on the request

  bool r = net_oce_handlers[request->method()](this, request, response);

  // only forward the sequence if the callee has not already done so
  if (r && !response->has_seq()) {
    response->set_seq(request->seq());
  }
  return r;
}
