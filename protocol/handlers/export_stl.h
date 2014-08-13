#include "handler.h"

#include <StlAPI_Writer.hxx>

HANDLER(export_stl) {

  if (req->argument_size() < 2) {
    HANDLER_ERROR("2 arguments required: <shape> 'filename.stl'")
    return true;
  }

  uint32_t handle = req->argument(0).uint32_value();
  const char *filename = req->argument(1).string_value().c_str();

  // TODO: coefficient argument
  // TODO: stl filename argument

  if (editor->shapes->size()) {

    StlAPI_Writer writer;

    TopoDS_Shape shape = editor->getShape(handle);
    if (shape.IsNull()) {
      HANDLER_ERROR("specified shape does not exist")
      return true;
    }

    // lower numbers cause more surface subdivision
    writer.SetCoefficient(0.01);
    writer.Write(shape, filename, true);

    // return a boolean true
    NetOCE_Value *value = res->add_value();
    value->set_type(NetOCE_Value::BOOL);
    value->set_bool_value(1);
  } else {
    // return a boolean true
    NetOCE_Value *value = res->add_value();
    value->set_type(NetOCE_Value::BOOL);
    value->set_bool_value(0);
  }

  return true;
}
