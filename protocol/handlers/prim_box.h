#include "handler.h"

#include <BRepPrimAPI_MakeBox.hxx>

HANDLER(prim_box, "double, double, double") {

  if (req->argument_size() != 3) {
    HANDLER_ERROR("3 arguments required: width(x) depth(y) height(z)")
    return true;
  }

  double w = req->argument(0).double_value()/2;
  double h = req->argument(1).double_value()/2;
  double d = req->argument(2).double_value()/2;


  TopoDS_Solid cube = BRepPrimAPI_MakeBox(gp_Pnt(-w, -h, -d), gp_Pnt(w, h, d));

  NetOCE_Value *val = res->add_value();
  val->set_type(NetOCE_Value::SHAPE_HANDLE);
  val->set_uint32_value(editor->addShape(cube));

  return true;
}
