#include "handler.h"

#include <BRepBuilderAPI_Transform.hxx>

HANDLER(op_rotate, "double, double, double, handle") {

  if (req->argument_size() != 4) {
    HANDLER_ERROR("4 arguments required (degrees): rx ry rz handle")
    return true;
  }

  double x = req->argument(0).double_value() * M_PI/180;
  double y = req->argument(1).double_value() * M_PI/180;
  double z = req->argument(2).double_value() * M_PI/180;
  uint32_t handle = req->argument(3).uint32_value();

  TopoDS_Shape shape = editor->getShape(handle);
  if (shape.IsNull()) {
    HANDLER_ERROR("specified object does not exist")
    return true;
  }

  gp_Trsf trsf, trX, trY, trZ;
  trX.SetRotation(gp::OX(), x);
  trY.SetRotation(gp::OY(), y);
  trZ.SetRotation(gp::OZ(), z);
  trsf = trX * trY * trZ;
  BRepBuilderAPI_Transform transform(shape, trsf, false);

  editor->setShape(handle, transform.Shape());

  NetOCE_Value *val = res->add_value();
  val->set_type(NetOCE_Value::SHAPE_HANDLE);
  val->set_uint32_value(handle);

  return true;
}
