var fs = require('fs');
var path = require('path');
var protobuf = require('protocol-buffers/require');

var req = protobuf('../protocol/oce.proto').NetOCE_Request;

var buf = req.encode({
  method : "hello",
  seq: 1,
  arguments : [{
    type: 1,
    double_value : Math.PI
  }, {
    type : 14,
    string_value : "hello world"
  }]
});

process.stdout.write(buf);
