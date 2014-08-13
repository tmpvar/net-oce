#include "handler.h"

#include <BRepAlgoAPI_Fuse.hxx>

HANDLER(op_union) {

  int argc = req->argument_size();

  if (argc < 2) {
    HANDLER_ERROR("atleast 2 arguments required: object object ...")
    return true;
  }

  // BOPCol_ListOfShape aLS;
  // aLS.Append(shape1);
  // aLS.Append(shape2);
  // BOPAlgo_PaveFiller dsFill;
  // dsFill.SetArguments(aLS);
  // dsFill.Perform();

  TopoDS_Shape result, a, b;

  a = editor->getShape(req->argument(0).uint32_value());
  b = editor->getShape(req->argument(1).uint32_value());
  if (a.IsNull() || b.IsNull()) {
    HANDLER_ERROR("invalid shape specified")
    return true;
  }

  result = BRepAlgoAPI_Fuse(a, b);

  for (int i=2; i<argc; i++) {
    TopoDS_Shape c = editor->getShape(req->argument(i).uint32_value());

    if (c.IsNull()) {
      HANDLER_ERROR("invalid shape specified")
      return true;
    }

    result = BRepAlgoAPI_Fuse(result, c);
  }

  NetOCE_Value *val = res->add_value();
  val->set_type(NetOCE_Value::SHAPE_HANDLE);
  val->set_uint32_value(editor->addShape(result));

  return true;
}
