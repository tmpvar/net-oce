#include "handler.h"

#include <BRepBndLib.hxx>
#include <StlTransfer.hxx>
#include <Bnd_Box.hxx>
#include <StlMesh_Mesh.hxx>
#include <StlMesh_MeshExplorer.hxx>

#define MAX2(X, Y)      (  Abs(X) > Abs(Y)? Abs(X) : Abs(Y) )
#define MAX3(X, Y, Z)   ( MAX2 ( MAX2(X,Y) , Z) )

HANDLER(shape_display, "handle, handle..") {

  int argc = req->argument_size();

  if (argc < 1) {
    HANDLER_ERROR(">= 1 arguments required: handle, handle..")
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

    theDeflection = MAX3(aXmax-aXmin , aYmax-aYmin , aZmax-aZmin) * 0.01;
    StlTransfer::BuildIncrementalMesh(compoundShape, .1, true, mesh);


    Standard_Real x1, y1, z1;
    Standard_Real x2, y2, z2;
    Standard_Real x3, y3, z3;

    uint32_t total_triangles = mesh->NbTriangles();
    uint32_t position_size = mesh->NbTriangles() * 9 * sizeof(float);
    uint32_t normal_size = mesh->NbTriangles() * 3 * sizeof(float);
    Standard_Integer domainCount = mesh->NbDomains();

    uint32_t position_where = 0, normal_where = 0;

    // we lose some precision here...
    float *position = (float *)malloc(position_size);
    float *normal = (float *)malloc(normal_size);
    StlMesh_MeshExplorer explorer(mesh);

    // create progress sentry for domains

    for (Standard_Integer nbd = 1; nbd <= domainCount; nbd++) {

      for (explorer.InitTriangle(nbd); explorer.MoreTriangle(); explorer.NextTriangle()) {
        explorer.TriangleVertices (
          x1, y1, z1,
          x2, y2, z2,
          x3, y3, z3
        );

        gp_XYZ v12 ((x2-x1), (y2-y1), (z2-z1));
        gp_XYZ v13 ((x3-x1), (y3-y1), (z3-z1));
        gp_XYZ norm = v12 ^ v13;
        Standard_Real mod = norm.Modulus ();
        if (mod > gp::Resolution()) {
          norm.Divide(mod);
        } else {
          norm.SetCoord (0., 0., 0.);
        }

        position[position_where++] = x1;
        position[position_where++] = y1;
        position[position_where++] = z1;

        position[position_where++] = x2;
        position[position_where++] = y2;
        position[position_where++] = z2;

        position[position_where++] = x3;
        position[position_where++] = y3;
        position[position_where++] = z3;

        normal[normal_where++] = norm.X();
        normal[normal_where++] = norm.Y();
        normal[normal_where++] = norm.Z();

      }
    }

    NetOCE_Value *positionVal = res->add_value();
    positionVal->set_type(NetOCE_Value::BYTES);
    positionVal->set_bytes_value(position, position_size);

    NetOCE_Value *normalVal = res->add_value();
    normalVal->set_type(NetOCE_Value::BYTES);
    normalVal->set_bytes_value(normal, normal_size);

  } else {
    HANDLER_ERROR("please make some shapes first!")
  }



  return true;
}
