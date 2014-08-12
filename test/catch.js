var fs = require('fs');
var path = require('path');
var protobuf = require('protocol-buffers/require');

var req = protobuf('../protocol/oce.proto').NetOCE_Response;

var b = new Buffer([]);

process.stdin.on('data', function(d) {
  b = Buffer.concat([b, d]);
});

process.stdin.on('end', function() {
  console.log(JSON.stringify(req.decode(b), null, '  '));
});

