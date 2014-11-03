#ifndef _NET_OCE_EDITOR_
#define _NET_OCE_EDITOR_

#include <unordered_map>

#include <TopoDS_Shape.hxx>
#include "oce.pb.h"

typedef uint32_t shape_id_t;

using namespace std;

class NetOCE_Editor {
  public:
    NetOCE_Editor();
    ~NetOCE_Editor();

    void reset();

    bool handleRequest(NetOCE_Request *request, NetOCE_Response *response);

    shape_id_t addShape(NetOCE_Request *request, TopoDS_Shape shape) {
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

    TopoDS_Shape getShape(shape_id_t handle) {
      if (this->shapes->count(handle)) {
        return this->shapes->at(handle);

      // signal a not-found back to the caller
      } else {
        TopoDS_Shape null;
        null.Nullify();
        return null;
      }
    }

    unordered_map<shape_id_t, TopoDS_Shape> *shapes;
  protected:
    shape_id_t shape_index;
};



#endif
