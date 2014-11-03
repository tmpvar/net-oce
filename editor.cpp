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

  for ( auto it = this->shapes->begin(); it != this->shapes->end(); ++it ) {
    it->second.Nullify();
  }

  this->shape_index = 0;
  this->shapes->clear();
}

bool NetOCE_Editor::handleRequest(NetOCE_Request *request, NetOCE_Response *response) {
  // TODO: typechecking on the request
  bool r = true;
  try {
    r = net_oce_handlers[request->method()](this, request, response);
  } catch (Standard_Failure e) {
    response->Clear();
    NetOCE_Value *val = response->add_value();
    val->set_type(NetOCE_Value::ERROR);
    val->set_string_value(e.GetMessageString());
  }

  // only forward the sequence if the callee has not already done so
  if (r && !response->has_seq()) {
    response->set_seq(request->seq());
  }
  return r;
}
