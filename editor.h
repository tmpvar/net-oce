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

    uint32_t addShape(NetOCE_Request *request, TopoDS_Shape shape) {
      if (request->has_shape_id()) {
        this->shape_index = request->shape_id();
      } else {
        this->shape_index++;
      }

      if (this->shapes->count(this->shape_index)) {
        TopoDS_Shape shape = this->shapes->at(this->shape_index);
        shape.Nullify();
        this->shapes->erase(this->shape_index);
      }

      this->shapes->emplace(this->shape_index, shape);

      return this->shape_index;
    }

    TopoDS_Shape getShape(uint32_t handle) {
      unordered_map<uint32_t, TopoDS_Shape>::iterator it = this->shapes->find(handle);
      if (it == this->shapes->end()) {
        TopoDS_Shape ret;
        ret.Nullify();
        return ret;
      } else {
        return it->second;
      }
    }

    unordered_map<uint32_t, TopoDS_Shape> *shapes;
  protected:
    uint32_t shape_index;
};



#endif
