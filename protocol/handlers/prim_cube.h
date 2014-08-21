#include "handler.h"

#include <BRepPrimAPI_MakeBox.hxx>

HANDLER(prim_cube, "double") {

  if (req->argument_size() != 3) {
    HANDLER_ERROR("r argument required: radius")
    return true;
  }

  double r = req->argument(0).double_value()/2;

  TopoDS_Solid cube = BRepPrimAPI_MakeBox(gp_Pnt(-r, -r, -r), gp_Pnt(r, r, r));

  NetOCE_Value *val = res->add_value();
  val->set_type(NetOCE_Value::SHAPE_HANDLE);
  val->set_uint32_value(editor->addShape(cube));

  return true;
}
