#include "handler.h"

#include <BRepAlgoAPI_Common.hxx>

HANDLER(op_intersect, "handle, handle") {

  int argc = req->argument_size();

  if (argc != 2) {
    HANDLER_ERROR("2 arguments required: [intersect] object [with] object")
    return true;
  }

  TopoDS_Shape result, a, b;

  a = editor->getShape(req->argument(0).uint32_value());
  b = editor->getShape(req->argument(1).uint32_value());
  if (a.IsNull() || b.IsNull()) {
    HANDLER_ERROR("invalid shape specified")
    return true;
  }

  result = BRepAlgoAPI_Common(a, b);

  NetOCE_Value *val = res->add_value();
  val->set_type(NetOCE_Value::SHAPE_HANDLE);
  val->set_uint32_value(editor->addShape(req, result));

  return true;
}
