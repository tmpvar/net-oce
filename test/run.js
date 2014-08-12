var fs = require('fs');
var path = require('path');
var protobuf = require('protocol-buffers/require');

var req = protobuf('../protocol/oce.proto').NetOCE_Request;

var buf = req.encode({
  method: 0,
  seq: 1
});

process.stdout.write(buf);
