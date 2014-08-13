var test = require('tape');
var fs = require('fs');
var path = require('path');
var protobuf = require('protocol-buffers/require');
var spawn = require('child_process').spawn;
var oce = protobuf('../protocol/oce.proto');
var request = oce.NetOCE_Request;
var response = oce.NetOCE_Response;

var exe = path.join(__dirname, '..', 'out/bin/net-oce');

function setup(fn) {
  var child = spawn(exe, [], {
    stdio : 'pipe'
  });

  child.stdout.once('data', function(first) {
    var r = response.decode(first);

    var queue = [];
    child.stdout.on('data', function(data) {
      var fn = queue.shift();
      fn(null, response.decode(data));
    });

    var methods = {
      _process : child
    };

    r.value.forEach(function(op) {
      var id = op.operation.id;

      methods[op.operation.name] = function() {

        var args = [];
        Array.prototype.push.apply(args, arguments);
        var fn = args.pop();

        var obj = {
          method : id,
          seq: 123123,
          arguments: args.map(function(arg) {

            // TODO: do proper typing
            return {
              type: 1,
              double_value: arg
            }
          })
        };
        queue.push(fn);

        child.stdin.write(request.encode(obj));
      };
    });

    fn(null, methods);
  });

  child.stdin.write(request.encode({ method: 0, seq: 0 }));
}

test('stl export - no shapes', function(t) {
  setup(function(e, child) {
    child.export_stl(function(e, result) {
      t.equal(result.value[0].type, 13);
      t.equal(result.value[0].bool_value, false);

      child._process.kill();
      t.end();
    });
  })
});

test('stl export - cube', function(t) {
  setup(function(e, child) {
    child.cube(0, 0, 0, 10, 10, 10, function(e, result) {
      child.export_stl(function(e, result) {
        t.equal(result.value[0].type, 13);
        t.equal(result.value[0].bool_value, true);

        child._process.kill();
        t.end();
      });
    });
  })
})
