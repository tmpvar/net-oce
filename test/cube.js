var request = require('net-oce-protocol').request;

process.stdout.write(request.encode({
  method: 1,
  seq: 1,
  argument : [
    { type : 1, double_value: 10 },
    { type : 1, double_value: 10 },
    { type : 1, double_value: 10 }
  ]
}))
