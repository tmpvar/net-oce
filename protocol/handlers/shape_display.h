#include "handler.h"

#include <BRepBndLib.hxx>
#include <StlTransfer.hxx>
#include <Bnd_Box.hxx>
#include <StlMesh_Mesh.hxx>
#include <StlMesh_MeshExplorer.hxx>
#include <GCPnts_UniformAbscissa.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <TopoDS.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <BRep_Tool.hxx>

#include <gp_Lin.hxx>
#include <gp_Circ.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <Geom_Line.hxx>

void discretization (const TopoDS_Edge& edge, NetOCE_Value *oce_feature_edges) {

  Standard_Real curve_start = 0, curve_end = 0;
  Handle_Geom_Curve curve = BRep_Tool::Curve (edge, curve_start, curve_end);
  assert (!curve.IsNull ());
  GeomAdaptor_Curve geometry_curve_adaptator (curve);

  GeomAbs_CurveType curve_type = geometry_curve_adaptator.GetType();

  uint8_t point_size = sizeof(double) * 3;

  NetOCE_Value *oce_edge = oce_feature_edges->add_item();
  oce_edge->set_type(NetOCE_Value::OBJECT);
  NetOCE_Value *oce_edge_type = oce_edge->add_item();
  oce_edge_type->set_type(NetOCE_Value::PRIMITIVE);
  oce_edge_type->set_string_value("type");

  if (curve_type == GeomAbs_Line) {
    gp_Lin line = geometry_curve_adaptator.Line();

    Handle(Geom_Line) geom_line = new Geom_Line(line);
    Geom_TrimmedCurve line_segment(geom_line, curve_start, curve_end, true);

    gp_Pnt line_start = line_segment.StartPoint();
    gp_Pnt line_end = line_segment.EndPoint();

    oce_edge_type->set_prim_type(NetOCE_Value::LINE);


    double line_start_d[3] = { line_start.X(), line_start.Y(), line_start.Z() };
    NetOCE_Value *oce_line_start = oce_edge->add_item();
    oce_line_start->set_type(NetOCE_Value::DOUBLE_BUFFER);
    oce_line_start->set_string_value("start");
    oce_line_start->set_bytes_value(line_start_d, point_size);

    double line_end_d[3] = { line_end.X(), line_end.Y(), line_end.Z() };
    NetOCE_Value *oce_line_end = oce_edge->add_item();
    oce_line_end->set_type(NetOCE_Value::DOUBLE_BUFFER);
    oce_line_end->set_string_value("end");
    oce_line_end->set_bytes_value(line_end_d, point_size);

  } else if (curve_type == GeomAbs_Circle) {
    gp_Circ circle = geometry_curve_adaptator.Circle();
    gp_Pnt loc = circle.Location();

    oce_edge_type->set_prim_type(NetOCE_Value::CIRCLE);

    double circle_center_d[3] = { loc.X(), loc.Y(), loc.Z() };
    NetOCE_Value *oce_circle_center = oce_edge->add_item();
    oce_circle_center->set_type(NetOCE_Value::DOUBLE_BUFFER);
    oce_circle_center->set_string_value("center");
    oce_circle_center->set_bytes_value(circle_center_d, point_size);

    NetOCE_Value *oce_circle_radius = oce_edge->add_item();
    oce_circle_radius->set_type(NetOCE_Value::DOUBLE);
    oce_circle_radius->set_string_value("radius");
    oce_circle_radius->set_double_value(circle.Radius());


  } else {
    fprintf(stderr, "UNHANDLED CURVE TYPE: %i\n", curve_type);

    BRepAdaptor_Curve curve_adaptator (edge);
    GCPnts_UniformAbscissa discretizer;
    discretizer.Initialize (curve_adaptator, 0.5);

    fprintf(stderr, "discretization (other):");

    for (uint32_t i = 1; i <= discretizer.NbPoints (); i++) {
      gp_Pnt p = curve_adaptator.Value (discretizer.Parameter (i));
      fprintf(stderr, " (%f, %f, %f)", p.X(), p.Y(), p.Z());
    }
    fprintf(stderr, "\n");
  }

  /*
    GeomAbs_Ellipse:
    GeomAbs_Hyperbola:
    GeomAbs_Parabola:
    GeomAbs_BezierCurve:
    GeomAbs_BSplineCurve:
    GeomAbs_OtherCurve:
  */
}


HANDLER(shape_display, "handle, handle..") {

  int argc = req->argument_size();

  if (argc < 1) {
    HANDLER_ERROR(">= 1 arguments required: handle, handle..")
    return true;
  }

  if (editor->shapes->size()) {
    Standard_Real x1, y1, z1;
    Standard_Real x2, y2, z2;
    Standard_Real x3, y3, z3;

    double bounds[6];
    float theDeflection;
    Bnd_Box total;
    TopoDS_Shape shape;
    Handle_StlMesh_Mesh mesh = new StlMesh_Mesh;
    uint32_t total_triangles;
    uint32_t position_size;
    uint32_t normal_size;
    uint32_t domain_size;
    Standard_Integer domainCount;

    uint32_t position_where;
    uint32_t normal_where;
    uint32_t domain_where;

    for (int i=0; i<argc; i++) {
      shape = editor->getShape(req->argument(i).uint32_value());

      if (shape.IsNull()) {
        HANDLER_ERROR("specified object does not exist")
        return true;
      }

      BRepBndLib::Add(shape, total);

      total.Get(
        bounds[0],
        bounds[1],
        bounds[2],
        bounds[3],
        bounds[4],
        bounds[5]
      );

      theDeflection = MAX3(
        bounds[3]-bounds[0],
        bounds[4]-bounds[1],
        bounds[5]-bounds[2]
      ) * 0.01;

      StlTransfer::BuildIncrementalMesh(shape, .1, true, mesh);

      total_triangles = mesh->NbTriangles();
      position_size = total_triangles * 9 * sizeof(float);
      normal_size = total_triangles * 9 * sizeof(float);
      domain_size = total_triangles * sizeof(uint32_t);

      domainCount = mesh->NbDomains();

      position_where = 0;
      normal_where = 0;
      domain_where = 0;

      // we lose some precision here...
      float *position = (float *)malloc(position_size);
      float *normal = (float *)malloc(normal_size);
      uint32_t *domains = (uint32_t *)malloc(domain_size);
      StlMesh_MeshExplorer explorer(mesh);

      NetOCE_Value *object = res->add_value();
      object->set_type(NetOCE_Value::OBJECT);


      NetOCE_Value *oce_edges = object->add_item();
      oce_edges->set_type(NetOCE_Value::ARRAY);
      oce_edges->set_string_value("feature_edges");

      TopExp_Explorer face_explorer(shape, TopAbs_FACE);
      for (uint32_t nbd = 1; nbd <= domainCount; nbd++) {

        NetOCE_Value *oce_feature_edges = oce_edges->add_item();
        oce_feature_edges->set_type(NetOCE_Value::ARRAY);

        TopExp_Explorer edge_explorer(face_explorer.Current(), TopAbs_EDGE);
        for (; edge_explorer.More(); edge_explorer.Next()) {
          const TopoDS_Edge edge = TopoDS::Edge(edge_explorer.Current());

          NetOCE_Value *oce_current_edge = oce_feature_edges->add_item();
          oce_current_edge->set_type(NetOCE_Value::ARRAY);

          discretization(edge, oce_current_edge);
        }

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

        if (face_explorer.More()) {
          face_explorer.Next();
        }
      }

      mesh->Clear();

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
      domainVal->set_string_value("features");
      domainVal->set_type(NetOCE_Value::UINT32_BUFFER);
      domainVal->set_bytes_value(domains, domain_size);
      free(domains);

      NetOCE_Value *boundVal = object->add_item();
      boundVal->set_string_value("bounds");
      boundVal->set_type(NetOCE_Value::DOUBLE_BUFFER);
      boundVal->set_bytes_value(bounds, sizeof(bounds));
    }

    mesh.Nullify();
  } else {
    HANDLER_ERROR("please make some shapes first!")
  }



  return true;
}
