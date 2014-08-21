#include "handler.h"

#include <BRepBuilderAPI_Transform.hxx>

HANDLER(op_translate, "double, double, double, handle") {

  if (req->argument_size() != 4) {
    HANDLER_ERROR("4 arguments required: x y z handle")
    return true;
  }

  double x = req->argument(0).double_value();
  double y = req->argument(1).double_value();
  double z = req->argument(2).double_value();
  uint32_t handle = req->argument(3).uint32_value();

  TopoDS_Shape shape = editor->getShape(handle);
  if (shape.IsNull()) {
    HANDLER_ERROR("specified object does not exist")
    return true;
  }

  gp_Trsf trsf;
  trsf.SetTranslation(gp_Vec(x, y, z));
  BRepBuilderAPI_Transform brep(shape, trsf, Standard_False);
  brep.Build();

  editor->setShape(handle, brep.Shape());

  NetOCE_Value *val = res->add_value();
  val->set_type(NetOCE_Value::SHAPE_HANDLE);
  val->set_uint32_value(handle);

  return true;
}
