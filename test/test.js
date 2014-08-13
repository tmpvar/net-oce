var _test = require('tape');
var fs = require('fs');
var path = require('path');
var pjoin = path.join;
var protobuf = require('protocol-buffers/require');
var spawn = require('child_process').spawn;
var oce = protobuf('../protocol/oce.proto');
var request = oce.NetOCE_Request;
var response = oce.NetOCE_Response;
var tdir = __dirname;
var tmpdir = pjoin(tdir, 'tmp') + '/';
var exe = pjoin(tdir, '..', 'out/bin/net-oce');

var schema = require('../schema');
var ENUM = schema.NetOCE_Value.type;

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

    child.stderr.on('data', function(d) {
      console.log('ERROR', d.toString());
    });

    var methods = {
      _process : child
    };

    r.value.forEach(function(op) {
      var id = op.operation.id;

      methods[op.operation.name] = function(a, fn) {
        var args;

        if (Array.isArray(a)) {
          args = a;
        } else {
          args = [];
          Array.prototype.push.apply(args, arguments);
          fn = args.pop();
          args = args.map(function(arg) {
            // TODO: do proper typing
            // TODO: Handle wrapper
            return {
              type: 1,
              double_value: arg
            }
          });
        }

        var obj = {
          method : id,
          seq: 123123,
          argument: args
        };
        queue.push(fn);

        var req = request.encode(obj);
        child.stdin.write(req);
      };
    });

    fn(null, methods);
  });

  child.stdin.write(request.encode({ method: 0, seq: 0 }));
}

function test(name, fn) {
  setup(function(e, child) {
    _test(name, function(t) {
      var end = t.end.bind(t);
      var ended = false;
      t.end = function() {
        ended = true;
        child._process && child._process.kill();
        end();
      };

      child._process.on('close', function() {
        if (!ended) {
          t.fail("net-oce crashed");
          t.end();
        }
      });

      fn(child, t);
    });
  });
}

test('stl export - no args', function(child, t) {
  child.export_stl(function(e, r) {
    t.equal(r.value[0].type, ENUM('ERROR'));
    t.end();
  });
});

test('stl export - invalid SHAPE_HANDLE', function(child, t) {
  child.cube(0, 0, 0, 10, 10, 10, function(e, cube) {
    child.export_stl([
      { type : ENUM('SHAPE_HANDLE'), uint32_value: 0 },
      { type : ENUM('STRING'), string_value: 'blah.stl' }
    ], function(e, r) {
      t.equal(r.value[0].type, ENUM('ERROR'));
      t.end();
    });
  });
});

test('stl export - no shapes', function(child, t) {
  child.export_stl([
    { type : ENUM('SHAPE_HANDLE'), uint32_value: 123 },
    { type : ENUM('STRING'), string_value: 'blah.stl' }
  ], function(e, result) {
    t.equal(result.value[0].type, ENUM('BOOL'));
    t.equal(result.value[0].bool_value, false);

    t.end();
  });
});

test('stl export - cube', function(child, t) {
  child.cube(0, 0, 0, 10, 10, 10, function(e, cube) {

    t.equal(cube.value[0].type, ENUM('SHAPE_HANDLE'));

    child.export_stl([
      { type : ENUM('SHAPE_HANDLE'), uint32_value: cube.value[0].uint32_value },
      { type : ENUM('STRING'), string_value: tmpdir + 'blah.stl' }
    ], function(e, result) {
      t.equal(result.value[0].type, ENUM('BOOL'));
      t.equal(result.value[0].bool_value, true);

      t.end();
    });
  });
});

test('cube - no args', function(child, t) {
  child.cube(function(e, result) {
    t.equal(result.value[0].type, ENUM('ERROR'));
    t.end();
  });
});

test('cube', function(child, t) {
  child.cube(0, 0, 0, 10, 10, 10, function(e, cube) {
    t.equal(cube.value[0].type, ENUM('SHAPE_HANDLE'));
    t.ok(cube.value[0].uint32_value !== 0);
    t.end();
  });
});
