var fs = require('fs');
var path = require('path');

var req = require('net-oce-protocol').request;

var buf = req.encode({
  method: 0,
  seq: 1
});

process.stdout.write(buf);
