#include "handler.h"

#include <BRepPrimAPI_MakeCylinder.hxx>
#include <gp_Dir.hxx>

HANDLER(prim_cylinder, "double, double") {

  if (req->argument_size() != 2) {
    HANDLER_ERROR("2 arguments required: radius(y) height(z)")
    return true;
  }

  double r = fabs(req->argument(0).double_value());
  double h = fabs(req->argument(1).double_value());

  if (r == 0) {
    HANDLER_ERROR("radius cannot be 0")
    return true;
  }

  if (h == 0) {
    HANDLER_ERROR("height cannot be 0")
    return true;
  }

  TopoDS_Solid cylinder = BRepPrimAPI_MakeCylinder(
    gp_Ax2(gp_Pnt(0, -h/2, 0), gp_Dir(0, 1, 0), gp_Dir(1, 0, 0)), r, h
  );

  shape_id_t shape_id = editor->addShape(req, cylinder);

  NetOCE_Value *val = res->add_value();
  val->set_type(NetOCE_Value::SHAPE_HANDLE);
  val->set_uint32_value(shape_id);
  return true;
}

