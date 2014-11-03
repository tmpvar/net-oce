#include "handler.h"

#include <BRepBuilderAPI_Copy.hxx>

HANDLER(op_copy, "handle") {

  if (req->argument_size() != 1) {
    HANDLER_ERROR("1 argument required: handle")
    return true;
  }

  uint32_t handle = req->argument(0).uint32_value();

  TopoDS_Shape shape = editor->getShape(handle);
  if (shape.IsNull()) {
    HANDLER_ERROR("specified object does not exist")
    return true;
  }

  BRepBuilderAPI_Copy builder;
  builder.Perform(shape);

  NetOCE_Value *val = res->add_value();
  val->set_type(NetOCE_Value::SHAPE_HANDLE);
  val->set_uint32_value(editor->addShape(req, builder.Shape()));

  return true;
}
