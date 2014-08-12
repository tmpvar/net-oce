#include "handler.h"
#include "editor.h"

#include <StlAPI_Writer.hxx>

HANDLER(export_stl) {

  // TODO: coefficient argument
  // TODO: stl filename argument

  if (editor->shapes->size()) {

    StlAPI_Writer writer;

    // lower numbers cause more surface subdivision
    writer.SetCoefficient(0.01);
    writer.Write(editor->shapes->at(0), "out.stl", true);
  }

  // TODO: return a handle
  return false;
}
