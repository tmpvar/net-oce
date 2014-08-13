#ifndef _NET_OCE_EDITOR_
#define _NET_OCE_EDITOR_

#include <unordered_map>

#include <TopoDS_Shape.hxx>
#include "oce.pb.h"

using namespace std;

class NetOCE_Editor {
  public:
    NetOCE_Editor();
    ~NetOCE_Editor();

    void reset();
    bool handleRequest(NetOCE_Request *request, NetOCE_Response *response);

    void addShape(uint32_t id, TopoDS_Shape shape) {
      this->shapes->insert(make_pair<uint32_t, TopoDS_Shape>(id, shape));
    }

    unordered_map<uint32_t, TopoDS_Shape> *shapes;
};



#endif
