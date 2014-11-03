#include "handler.h"

#include <BRepPrimAPI_MakeSphere.hxx>

HANDLER(prim_sphere, "double") {

  if (req->argument_size() != 1) {
    HANDLER_ERROR("1 arguments required: radius")
    return true;
  }

  double r = fabs(req->argument(0).double_value());

  if (r == 0) {
    HANDLER_ERROR("radius cannot be 0")
    return true;
  }

  TopoDS_Solid sphere = BRepPrimAPI_MakeSphere(gp_Pnt(0, 0, 0), r);

  shape_id_t shape_id = editor->addShape(req, sphere);

  if (!req->has_shape_id()) {
    NetOCE_Value *val = res->add_value();
    val->set_type(NetOCE_Value::SHAPE_HANDLE);
    val->set_uint32_value(shape_id);
    return true;
  }

  return false;
}
