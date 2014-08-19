#include "handler.h"

#include <BRepPrimAPI_MakeSphere.hxx>

HANDLER(sphere, "double, double, double, double") {

  if (req->argument_size() != 4) {
    HANDLER_ERROR("4 arguments required: cx cy cz radius")
    return true;
  }

  double x = req->argument(0).double_value();
  double y = req->argument(1).double_value();
  double z = req->argument(2).double_value();
  double r = req->argument(3).double_value();


  TopoDS_Solid sphere = BRepPrimAPI_MakeSphere(gp_Pnt(x, y, z), r);

  NetOCE_Value *val = res->add_value();
  val->set_type(NetOCE_Value::SHAPE_HANDLE);
  val->set_uint32_value(editor->addShape(sphere));

  return true;
}
