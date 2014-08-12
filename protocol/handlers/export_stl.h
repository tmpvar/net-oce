#include "handler.h"
#include "editor.h"

#include <StlAPI_Writer.hxx>

HANDLER(export_stl) {

  NetOCE_Value *value = res->add_value();
  value->set_type(NetOCE_Value::BOOL);

  // TODO: coefficient argument
  // TODO: stl filename argument

  if (editor->shapes->size()) {

    StlAPI_Writer writer;

    // lower numbers cause more surface subdivision
    writer.SetCoefficient(0.01);
    writer.Write(editor->shapes->at(0), "out.stl", true);
    value->set_bool(1);
  } else {
    value->set_bool(1);
  }

  return true;
}
