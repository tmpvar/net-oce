#include "handler.h"

#include <BRepPrimAPI_MakeBox.hxx>


//TopoDS_Shell shp1_shell = BRepTools::OuterShell(solid1);

HANDLER(cube) {

  TopoDS_Solid cube = BRepPrimAPI_MakeBox(gp_Pnt(0, 0, 0), 3, 3, 3);

  editor->shapes->push_back(cube);

  // TODO: return a handle
  return true;
}
