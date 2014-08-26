#include "handler.h"

#include <TopExp_Explorer.hxx>
#include <TopoDS_Edge.hxx>
#include <Geom2d_Curve.hxx>
#include <BRepTools.hxx>
#include <BRepTools_ShapeSet.hxx>
#include <iostream>

#include <BRep_CurveRepresentation.hxx>
#include <BRep_ListIteratorOfListOfCurveRepresentation.hxx>
#include <BRep_GCurve.hxx>
#include <BRep_TEdge.hxx>

HANDLER(extract_edges, "handle") {

  int argc = req->argument_size();

  if (argc < 1) {
    HANDLER_ERROR("1+ arguments required: object, object..")
    return true;
  }



  BRepTools_ShapeSet SS;

  SS.Add(editor->getShape(req->argument(0).uint32_value()));
  SS.Dump(std::cerr);

  // return true;


  TopoDS_Shape shape;

  for (uint32_t i=0; i<argc; i++) {
    shape = editor->getShape(req->argument(i).uint32_value());
    if (shape.IsNull()) {
      HANDLER_ERROR("invalid shape specified")
      return true;
    }

    Handle(BRep_TEdge) TE = Handle(BRep_TEdge)::DownCast(shape.TShape());
    BRep_ListIteratorOfListOfCurveRepresentation itrc = TE->Curves();
    // Standard_Real first, last;
    // while (itrc.More()) {
    //   cerr << "HERE" << endl;
    //   const Handle(BRep_CurveRepresentation)& CR = itrc.Value();
    //   if (CR->IsCurve3D()) {
    //     // Handle(BRep_GCurve) GC = Handle(BRep_GCurve)::DownCast(itrc.Value());
    //   //   GC->Range(first, last);
    //   //   if (!CR->Curve3D().IsNull()) {
    //   //     // cerr << "    - Curve 3D : "<<myCurves.Index(CR->Curve3D());
    //   //     // if (!CR->Location().IsIdentity())
    //   //     //   cerr << " location "<<Locations().Index(CR->Location());
    //   //     cerr <<", range : " << first << " " << last <<"\n";
    //   //   }
    //   }
    // }

    // TopExp_Explorer explorer(shape, TopAbs_EDGE);

    // for (; explorer.More(); explorer.Next()) {
    //   const TopoDS_Edge& edge = TopoDS::Edge (explorer.Current());
    //   Standard_Real aF, aL;
    //   Handle(Geom2d_Curve) curve = BRep_Tool::CurveOnSurface (anEdge, theFace, aF, aL);

    //   NetOCE_Value *val = res->add_value();
    //   val->set_type(NetOCE_Value::SHAPE_HANDLE);
    //   val->set_uint32_value(editor->addShape(result));

    // }
  }


  return true;
}
