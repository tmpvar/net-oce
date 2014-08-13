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

    uint32_t addShape(TopoDS_Shape shape) {
      this->shape_index++;

      this->shapes->insert(
        make_pair<uint32_t, TopoDS_Shape>(this->shape_index, shape)
      );
      return this->shape_index;
    }

    unordered_map<uint32_t, TopoDS_Shape> *shapes;
  protected:
    uint32_t shape_index;
};



#endif
