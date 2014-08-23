#include "handler.h"

#include <BRepBuilderAPI_Transform.hxx>

HANDLER(op_translate, "handle, double, double, double") {

  if (req->argument_size() != 4) {
    HANDLER_ERROR("4 arguments required: handle x y z")
    return true;
  }

  uint32_t handle = req->argument(0).uint32_value();
  double x = req->argument(1).double_value();
  double y = req->argument(2).double_value();
  double z = req->argument(3).double_value();


  TopoDS_Shape shape = editor->getShape(handle);
  if (shape.IsNull()) {
    HANDLER_ERROR("specified object does not exist")
    return true;
  }

  gp_Trsf trsf;
  trsf.SetTranslation(gp_Vec(x, y, z));
  BRepBuilderAPI_Transform brep(shape, trsf, Standard_True);
  brep.Build();

  NetOCE_Value *val = res->add_value();
  val->set_type(NetOCE_Value::SHAPE_HANDLE);
  val->set_uint32_value(editor->addShape(brep.Shape()));

  return true;
}
