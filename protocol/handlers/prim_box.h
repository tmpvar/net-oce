#include "handler.h"

#include <BRepPrimAPI_MakeBox.hxx>

HANDLER(prim_box, "double, double, double") {

  if (req->argument_size() != 3) {
    HANDLER_ERROR("3 arguments required: width(x) depth(y) height(z)")
    return true;
  }

  double w = fabs(req->argument(0).double_value());
  double h = fabs(req->argument(1).double_value());
  double d = fabs(req->argument(2).double_value());


  TopoDS_Solid box = BRepPrimAPI_MakeBox(gp_Pnt(-w/2, -h/2, -d/2), w, h, d);

  NetOCE_Value *val = res->add_value();
  val->set_type(NetOCE_Value::SHAPE_HANDLE);
  val->set_uint32_value(editor->addShape(box));

  return true;
}
