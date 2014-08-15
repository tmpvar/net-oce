var fs = require('fs');
var path = require('path');
var res = require('net-oce-protocol').objects.NetOCE_Response;
var b = new Buffer([]);

process.stdin.on('data', function(d) {
  b = Buffer.concat([b, d]);
});

process.stdin.on('end', function() {
  console.log(JSON.stringify(res.decode(b), null, '  '));
});

