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

    double bounds[6];
    float theDeflection;
    Bnd_Box total;

    BRepBndLib::Add(compoundShape, total);

    total.Get(bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]);

    theDeflection = MAX3(bounds[3]-bounds[0] , bounds[4]-bounds[1] , bounds[5]-bounds[2]) * 0.01;
    StlTransfer::BuildIncrementalMesh(compoundShape, .1, true, mesh);


    Standard_Real x1, y1, z1;
    Standard_Real x2, y2, z2;
    Standard_Real x3, y3, z3;

    uint32_t total_triangles = mesh->NbTriangles();
    uint32_t position_size = total_triangles * 9 * sizeof(float);
    uint32_t normal_size = total_triangles * 9 * sizeof(float);
    uint32_t domain_size = total_triangles * sizeof(uint32_t);

    Standard_Integer domainCount = mesh->NbDomains();

    uint32_t position_where = 0, normal_where = 0, domain_where = 0;

    // we lose some precision here...
    float *position = (float *)malloc(position_size);
    float *normal = (float *)malloc(normal_size);
    uint32_t *domains = (uint32_t *)malloc(domain_size);
    StlMesh_MeshExplorer explorer(mesh);

    // create progress sentry for domains

    for (uint32_t nbd = 1; nbd <= domainCount; nbd++) {

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

        domains[domain_where++] = nbd;

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

        normal[normal_where++] = norm.X();
        normal[normal_where++] = norm.Y();
        normal[normal_where++] = norm.Z();

        normal[normal_where++] = norm.X();
        normal[normal_where++] = norm.Y();
        normal[normal_where++] = norm.Z();
      }
    }

    mesh->Clear();
    mesh.Nullify();
    compoundShape.Nullify();

    NetOCE_Value *object = res->add_value();
    object->set_type(NetOCE_Value::OBJECT);


    NetOCE_Value *positionVal = object->add_item();
    positionVal->set_type(NetOCE_Value::FLOAT_BUFFER);
    positionVal->set_string_value("positions");
    positionVal->set_bytes_value(position, position_size);
    free(position);

    NetOCE_Value *normalVal = object->add_item();
    normalVal->set_string_value("normals");
    normalVal->set_type(NetOCE_Value::FLOAT_BUFFER);
    normalVal->set_bytes_value(normal, normal_size);
    free(normal);

    NetOCE_Value *domainVal = object->add_item();
    domainVal->set_string_value("cells");
    domainVal->set_type(NetOCE_Value::UINT32_BUFFER);
    domainVal->set_bytes_value(domains, domain_size);
    free(domains);

    NetOCE_Value *boundVal = object->add_item();
    boundVal->set_string_value("bounds");
    boundVal->set_type(NetOCE_Value::DOUBLE_BUFFER);
    boundVal->set_bytes_value(bounds, sizeof(bounds));

  } else {
    HANDLER_ERROR("please make some shapes first!")
  }



  return true;
}
