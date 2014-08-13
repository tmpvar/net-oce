#include "handler.h"

#include <BRepPrimAPI_MakeBox.hxx>

HANDLER(cube) {

  if (req->argument_size() != 6) {
    HANDLER_ERROR("6 arguments required: x y z w d h")
    return true;
  }

  double x = req->argument(0).double_value();
  double y = req->argument(1).double_value();
  double z = req->argument(2).double_value();
  double w = req->argument(3).double_value();
  double h = req->argument(4).double_value();
  double d = req->argument(5).double_value();


  TopoDS_Solid cube = BRepPrimAPI_MakeBox(gp_Pnt(x, y, z), w, h, d);

  NetOCE_Value *val = res->add_value();
  val->set_type(NetOCE_Value::SHAPE_HANDLE);
  val->set_uint32_value(editor->addShape(cube));

  return true;
}
