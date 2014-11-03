#include "handler.h"

#include <StlAPI_Writer.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Builder.hxx>

#include <StlMesh_Mesh.hxx>
#include <StlMesh_MeshExplorer.hxx>
#include <StlTransfer.hxx>
#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>

typedef struct {
  float nx;
  float ny;
  float nz;
  float x1;
  float y1;
  float z1;
  float x2;
  float y2;
  float z2;
  float x3;
  float y3;
  float z3;
  uint16_t attr = 0;
} stl_facet_t;

char *write_binary(const char* filename, uint8_t len, const Handle(StlMesh_Mesh)& theMesh, uint32_t *buffer_length) {
  stl_facet_t stl_facet;

  static const uint8_t stl_facet_size = sizeof(float) * 12 + sizeof(uint16_t);

  StlMesh_MeshExplorer aMexp (theMesh);

  Standard_Integer aNbDomains = theMesh->NbDomains();
  uint32_t total_triangles = theMesh->NbTriangles();

  *buffer_length = 84 + total_triangles * stl_facet_size;

  char *buffer = (char *)malloc(*buffer_length);

  memset(buffer, 0, 80);
  memcpy(buffer, filename, len>80 ? 80 : len);

  Standard_Real x1, y1, z1, x2, y2, z2, x3, y3, z3;

  uint32_t where = 80;

  memcpy(buffer+where, &total_triangles, 4);
  where+=4;

  for (Standard_Integer nbd = 1; nbd <= aNbDomains; nbd++) {
    for (aMexp.InitTriangle (nbd); aMexp.MoreTriangle(); aMexp.NextTriangle()) {
      aMexp.TriangleVertices ( x1, y1, z1, x2, y2, z2, x3, y3, z3);

      gp_XYZ V ((x2-x1), (y2-y1), (z2-z1));
      gp_XYZ W ((x3-x1), (y3-y1), (z3-z1));
      gp_XYZ normal = V ^ W;
      Standard_Real mod = normal.Modulus ();
      if (mod > gp::Resolution()) {
        normal.Divide(mod);
      } else {
        normal.SetCoord (0., 0., 0.);
      }

      stl_facet.nx = normal.X();
      stl_facet.ny = normal.Y();
      stl_facet.nz = normal.Z();

      stl_facet.x1 = x1;
      stl_facet.y1 = y1;
      stl_facet.z1 = z1;

      stl_facet.x2 = x2;
      stl_facet.y2 = y2;
      stl_facet.z2 = z2;

      stl_facet.x3 = x3;
      stl_facet.y3 = y3;
      stl_facet.z3 = z3;

      memcpy(buffer+where, (void *)&stl_facet, stl_facet_size);
      where += stl_facet_size;
    }
  }
  return buffer;
};


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

    Standard_Real aXmin, aYmin, aZmin, aXmax, aYmax, aZmax;
    Bnd_Box Total;
    Handle_StlMesh_Mesh mesh = new StlMesh_Mesh;
    BRepBndLib::Add(compoundShape, Total);
    Total.Get(aXmin, aYmin, aZmin, aXmax, aYmax, aZmax);

    float theDeflection = MAX3(
      aXmax-aXmin,
      aYmax-aYmin,
      aZmax-aZmin
    )*0.001;

    StlTransfer::BuildIncrementalMesh(
      compoundShape,
      0.01, // coefficient
      true,
      mesh
    );

    uint32_t buffer_length = 0;
    char *buffer = write_binary(
      filename,
      req->argument(0).string_value().size(),
      mesh,
      &buffer_length
    );

    compoundShape.Nullify();

    // return a boolean true
    NetOCE_Value *value = res->add_value();
    value->set_type(NetOCE_Value::BYTES);
    value->set_bytes_value(buffer, buffer_length);

    free(buffer);
  } else {
    HANDLER_ERROR("scene has no objects")
  }

  return true;
}
