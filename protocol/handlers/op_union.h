#include "handler.h"

#include <BRepAlgoAPI_Fuse.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>


HANDLER(op_union, "handle, handle..") {

  int argc = req->argument_size();

  if (argc < 2) {
    HANDLER_ERROR("atleast 2 arguments required: object object ...")
    return true;
  }

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

    result = BRepAlgoAPI_Fuse(result, c).Shape();
  }

  ShapeUpgrade_UnifySameDomain unify(result);
  unify.Build();
  shape_id_t shape_id = editor->addShape(req, unify.Shape());

  if (!req->has_shape_id()) {
    NetOCE_Value *val = res->add_value();
    val->set_type(NetOCE_Value::SHAPE_HANDLE);
    val->set_uint32_value(shape_id);
    return true;
  }

  return false;
}
