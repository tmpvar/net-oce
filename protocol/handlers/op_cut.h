#include "handler.h"

#include <BRepAlgoAPI_Cut.hxx>

HANDLER(op_cut, "handle, handle..") {

  int argc = req->argument_size();

  if (argc < 2) {
    HANDLER_ERROR("2 arguments required: [cut] object [with] object")
    return true;
  }

  TopoDS_Shape result, b;

  result = editor->getShape(req->argument(0).uint32_value());
  if (result.IsNull()) {
    HANDLER_ERROR("invalid shape specified")
    return true;
  }

  for (int i=1; i<argc; i++) {
    b = editor->getShape(req->argument(i).uint32_value());
    if (b.IsNull()) {
      HANDLER_ERROR("invalid shape specified")
      return true;
    }

    result = BRepAlgoAPI_Cut(result, b);
  }

  NetOCE_Value *val = res->add_value();
  val->set_type(NetOCE_Value::SHAPE_HANDLE);
  val->set_uint32_value(editor->addShape(req, result));

  return true;
}
