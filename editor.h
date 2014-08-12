#ifndef _NET_OCE_EDITOR_
#define _NET_OCE_EDITOR_

#include <vector>
#include <TopoDS_Shape.hxx>
#include "oce.pb.h"

using namespace std;

class NetOCE_Editor {
  public:
    NetOCE_Editor();
    ~NetOCE_Editor();

    void reset();
    bool handleRequest(NetOCE_Request *request, NetOCE_Response *response);


  protected:
    vector<TopoDS_Shape> *shapes;

};



#endif
