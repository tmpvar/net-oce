#include "handler.h"

#include <BRepBndLib.hxx>
#include <StlTransfer.hxx>
#include <Bnd_Box.hxx>
#include <StlMesh_Mesh.hxx>
#include <StlMesh_MeshExplorer.hxx>

#define MAX2(X, Y)      (  Abs(X) > Abs(Y)? Abs(X) : Abs(Y) )
#define MAX3(X, Y, Z)   ( MAX2 ( MAX2(X,Y) , Z) )

HANDLER(extract_stl, "handle, handle..") {

  int argc = req->argument_size();

  if (argc < 1) {
    HANDLER_ERROR("1 arguments required: handle, handle..")
    return true;
  }

  if (editor->shapes->size()) {

    TopoDS_Compound compoundShape;
    BRep_Builder builder;
    builder.MakeCompound(compoundShape);

    uint32_t handle;
    for (int i=0; i<argc; i++) {
      handle = req->argument(i).uint32_value();

      TopoDS_Shape shape = editor->getShape(handle);
      if (shape.IsNull()) {
        HANDLER_ERROR("specified object does not exist")
        return true;
      }

      builder.Add(compoundShape, shape);
    }

    Handle_StlMesh_Mesh mesh = new StlMesh_Mesh;
    Standard_Real aXmin, aYmin, aZmin, aXmax, aYmax, aZmax, theDeflection;
    Bnd_Box total;

    BRepBndLib::Add(compoundShape, total);

    total.Get(aXmin, aYmin, aZmin, aXmax, aYmax, aZmax);

    theDeflection = MAX3(aXmax-aXmin , aYmax-aYmin , aZmax-aZmin) * 0.001;
    StlTransfer::BuildIncrementalMesh(compoundShape, .001, true, mesh);


    Standard_Real x1, y1, z1;
    Standard_Real x2, y2, z2;
    Standard_Real x3, y3, z3;

    uint32_t total_size = mesh->NbTriangles() * 9, where = 0;;
    Standard_Integer domainCount = mesh->NbDomains();

    // we lose some precision here...
    float *buf = (float *)malloc(total_size * sizeof(float));
    StlMesh_MeshExplorer explorer(mesh);

    // create progress sentry for domains

    for (uint32_t nbd = 1; nbd <= domainCount; nbd++) {

      for (explorer.InitTriangle(nbd); explorer.MoreTriangle(); explorer.NextTriangle()) {
        explorer.TriangleVertices (x1,y1,z1,x2,y2,z2,x3,y3,z3);

        buf[where++] = x1;
        buf[where++] = y1;
        buf[where++] = z1;

        buf[where++] = x2;
        buf[where++] = y2;
        buf[where++] = z2;

        buf[where++] = x3;
        buf[where++] = y3;
        buf[where++] = z3;
      }
    }

    NetOCE_Value *val = res->add_value();
    val->set_type(NetOCE_Value::BYTES);
    val->set_bytes_value((void *)buf, total_size);
  } else {
    assert(0);
  }



  return true;
}
