#include "editor.h"
#include "handler.h"

NetOCE_Editor::NetOCE_Editor() {
  this->shapes = new vector<TopoDS_Shape>;
}

NetOCE_Editor::~NetOCE_Editor() {
  delete this->shapes;
}

void NetOCE_Editor::reset() {
  vector<TopoDS_Shape>::iterator it = this->shapes->begin();
  for (; it != this->shapes->end(); ++it) {
    printf("clean\n");
  }

  this->shapes->clear();
}

bool NetOCE_Editor::handleRequest(NetOCE_Request *request, NetOCE_Response *response) {
  // TODO: typechecking on the request

  bool r = net_oce_handlers[request->method()](this, request, response);

  if (r) {
    response->set_seq(request->seq());
  }
  return r;
}
