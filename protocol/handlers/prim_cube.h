#include "handler.h"

#include <BRepPrimAPI_MakeBox.hxx>

HANDLER(prim_cube, "double") {

  if (req->argument_size() != 1) {
    HANDLER_ERROR("1 argument required: radius")
    return true;
  }

  double d = fabs(req->argument(0).double_value());

  if (d == 0) {
    HANDLER_ERROR("radius cannot be 0")
    return true;
  }

  double r = d/2;

  TopoDS_Solid shape = BRepPrimAPI_MakeBox(gp_Pnt(-r, -r, -r), gp_Pnt(r, r, r));
  shape_id_t shape_id = editor->addShape(req, shape);

  bool has_shape_id = req->has_shape_id();

  if (!has_shape_id) {
    NetOCE_Value *val = res->add_value();
    val->set_type(NetOCE_Value::SHAPE_HANDLE);
    val->set_uint32_value(shape_id);
    return true;
  }

  return false;
}
