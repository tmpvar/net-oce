#include "handler.h"

#include <BRepBndLib.hxx>
#include <StlTransfer.hxx>
#include <Bnd_Box.hxx>
#include <StlMesh_Mesh.hxx>

#define MAX2(X, Y)      (  Abs(X) > Abs(Y)? Abs(X) : Abs(Y) )
#define MAX3(X, Y, Z)   ( MAX2 ( MAX2(X,Y) , Z) )

HANDLER(extract_stl, "handle, handle..") {

  int argc = req->argument_size();

  if (argc != 2) {
    HANDLER_ERROR("1 arguments required: handle, handle..")
    return true;
  }

  if (editor->shapes->size()) {

    StlAPI_Writer writer;

    TopoDS_Compound compoundShape;
    BRep_Builder builder;
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

    StlMesh_Mesh mesh;
    Standard_Real aXmin, aYmin, aZmin, aXmax, aYmax, aZmax, theDeflection;
    Bnd_Box Total;
    BRepBndLib::Add(compoundShape, Total);
    Total.Get(aXmin, aYmin, aZmin, aXmax, aYmax, aZmax);
    theDeflection = MAX3(aXmax-aXmin , aYmax-aYmin , aZmax-aZmin) * 0.001;

    StlTransfer::BuildIncrementalMesh(compoundShape, theDeflection, true, &mesh);

  }

  // NetOCE_Value *val = res->add_value();
  // val->set_type(NetOCE_Value::SHAPE_HANDLE);
  // val->set_uint32_value(editor->addShape(result));

  return true;
}
