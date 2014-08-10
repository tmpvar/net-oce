var fs = require('fs');
var path = require('path');
var protobuf = require('protocol-buffers/require');

var messages = protobuf('../protocol/test.proto');


var buf = messages.Person.encode({
  id: 1,
  name: "Elijah",
  email: "tmpvar@gmail"
});


var prefix = new Buffer(4);
prefix.writeUInt32LE(buf.length, 0);

// TODO: message type for instantiation

process.stdout.write(prefix);
process.stdout.write(buf.toString('binary'));
