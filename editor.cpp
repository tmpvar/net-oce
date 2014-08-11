#include "editor.h"

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

  // if ()


  // no reply required
  return false;
}
