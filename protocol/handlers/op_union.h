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

  TopoDS_Shape result;


  result = BRepAlgoAPI_Fuse(
    editor->getShape(req->argument(0).uint32_value()),
    editor->getShape(req->argument(1).uint32_value())
  );

  // for (int i=1; i<argc; i++) {

  // }


  // BRepAlgoAPI_Fuse
  NetOCE_Value *val = res->add_value();
  val->set_type(NetOCE_Value::SHAPE_HANDLE);
  val->set_uint32_value(editor->addShape(result));

  return true;
}
