#include "handler.h"

#include <BRepPrimAPI_MakeBox.hxx>

HANDLER(prim_cube, "double") {

  if (req->argument_size() != 1) {
    HANDLER_ERROR("1 argument required: radius")
    return true;
  }

  double r = fabs(req->argument(0).double_value()/2);

  if (r == 0) {
    HANDLER_ERROR("radius cannot be 0")
    return true;
  }

  TopoDS_Solid cube = BRepPrimAPI_MakeBox(gp_Pnt(-r, -r, -r), gp_Pnt(r, r, r));
  NetOCE_Value *val = res->add_value();
  val->set_type(NetOCE_Value::SHAPE_HANDLE);
  val->set_uint32_value(editor->addShape(req, cube));

  return true;
}
