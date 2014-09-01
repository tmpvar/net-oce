#include "handler.h"

#include <StlAPI_Writer.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Builder.hxx>


HANDLER(export_stl, "string, handle..") {
  int argc = req->argument_size();

  if (argc < 2) {
    HANDLER_ERROR("2 arguments required: 'filename.stl' object ...")
    return true;
  }

  const char *filename = req->argument(0).string_value().c_str();

  // TODO: coefficient argument

  if (editor->shapes->size()) {

    StlAPI_Writer writer;

    TopoDS_Compound compoundShape;
    TopoDS_Builder builder;
    builder.MakeCompound(compoundShape);

    uint32_t handle;
    for (int i=1; i<argc; i++) {
      handle = req->argument(i).uint32_value();

      TopoDS_Shape shape = editor->getShape(handle);
      if (shape.IsNull()) {
        HANDLER_ERROR("specified object does not exist")
        return true;
      }

      builder.Add(compoundShape, shape);
    }

    // lower numbers cause more surface subdivision
    writer.SetCoefficient(0.01);
    writer.Write(compoundShape, filename, true);

    compoundShape.Nullify();

    // return a boolean true
    NetOCE_Value *value = res->add_value();
    value->set_type(NetOCE_Value::BOOL);
    value->set_bool_value(1);
  } else {
    HANDLER_ERROR("scene has no objects")
  }

  return true;
}
