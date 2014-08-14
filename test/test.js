var _test = require('tape');
var fs = require('fs');
var path = require('path');
var pjoin = path.join;
var protobuf = require('protocol-buffers/require');
var spawn = require('child_process').spawn;
var oce = require('net-oce-protocol');

var request = oce.objects.NetOCE_Request;
var response = oce.objects.NetOCE_Response;

var tdir = __dirname;
var tmpdir = pjoin(tdir, 'tmp') + '/';
var exe = pjoin(tdir, '..', 'out/bin/net-oce');

var stl = require('stl');

// var schema = require('../schema');
var ENUM = oce.NetOCE_Value.type;

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

    var seq = 1;
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
          seq: seq++,
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
    { type : ENUM('STRING'), string_value: 'no shapes' },
    { type : ENUM('SHAPE_HANDLE'), uint32_value: 123 }
  ], function(e, result) {
    t.equal(result.value[0].type, ENUM('BOOL'));
    t.equal(result.value[0].bool_value, false);

    t.end();
  });
});

test('stl export - cube', function(child, t) {
  child.cube(0, 0, 0, 10, 10, 10, function(e, cube) {

    t.equal(cube.value[0].type, ENUM('SHAPE_HANDLE'));

    var out = tmpdir + 'cube.stl';

    child.export_stl([
      { type : ENUM('STRING'), string_value: out },
      { type : ENUM('SHAPE_HANDLE'), uint32_value: cube.value[0].uint32_value }
    ], function(e, result) {
      t.equal(result.value[0].type, ENUM('BOOL'));
      t.equal(result.value[0].bool_value, true);

      var obj = stl.toObject(fs.readFileSync(out).toString());
      t.equal(obj.facets.length, 12);

      t.end();
    });
  });
});

test('stl export - 2 cubes', function(child, t) {
  child.cube(0, 0, 0, 10, 10, 10, function(e, cube1) {
    t.equal(cube1.value[0].type, ENUM('SHAPE_HANDLE'));

    child.cube(20, 0, 0, 10, 10, 10, function(e, cube2) {
      t.equal(cube2.value[0].type, ENUM('SHAPE_HANDLE'));

      var out = tmpdir + 'two-cubes.stl';

      child.export_stl([
        { type : ENUM('STRING'), string_value: out },
        { type : ENUM('SHAPE_HANDLE'), uint32_value: cube1.value[0].uint32_value },
        { type : ENUM('SHAPE_HANDLE'), uint32_value: cube2.value[0].uint32_value }
      ], function(e, result) {
        t.equal(result.value[0].type, ENUM('BOOL'));
        t.equal(result.value[0].bool_value, true);

        var obj = stl.toObject(fs.readFileSync(out).toString());
        t.equal(obj.facets.length, 24);

        t.end();
      });
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

test('op_union - 2 cubes', function(child, t) {
  child.cube(0, 0, 0, 10, 10, 10, function(e, cube1) {
    child.cube(5, 5, 5, 10, 10, 10, function(e, cube2) {
      child.op_union([
        { type : ENUM('SHAPE_HANDLE'), uint32_value: cube1.value[0].uint32_value },
        { type : ENUM('SHAPE_HANDLE'), uint32_value: cube2.value[0].uint32_value }
      ], function(e, unioned) {
        t.equal(unioned.value[0].type, ENUM('SHAPE_HANDLE'));
        t.ok(unioned.value[0].uint32_value !== 0);

        var out = tmpdir + 'op_union.stl'

        child.export_stl([
          { type : ENUM('STRING'), string_value: out },
          { type : ENUM('SHAPE_HANDLE'), uint32_value: unioned.value[0].uint32_value },
        ], function(e, result) {
          t.equal(result.value[0].type, ENUM('BOOL'));
          t.equal(result.value[0].bool_value, true);

          var obj = stl.toObject(fs.readFileSync(out).toString());
          t.equal(obj.facets.length, 36);
          t.end();
        });
      });
    });
  });
});

test('op_union - 3 cubes', function(child, t) {
  child.cube(0, 0, 0, 10, 10, 10, function(e, cube1) {
    child.cube(5, 5, 5, 10, 10, 10, function(e, cube2) {
      child.cube(-5, -5, -5, 10, 10, 5, function(e, cube3) {
        child.op_union([
          { type : ENUM('SHAPE_HANDLE'), uint32_value: cube1.value[0].uint32_value },
          { type : ENUM('SHAPE_HANDLE'), uint32_value: cube2.value[0].uint32_value },
          { type : ENUM('SHAPE_HANDLE'), uint32_value: cube3.value[0].uint32_value },
        ], function(e, unioned) {
          t.equal(unioned.value[0].type, ENUM('SHAPE_HANDLE'));
          t.ok(unioned.value[0].uint32_value !== 0);

          var out = tmpdir + 'op_union.3cubes.stl';

          child.export_stl([
            { type : ENUM('STRING'), string_value: out },
            { type : ENUM('SHAPE_HANDLE'), uint32_value: unioned.value[0].uint32_value },
          ], function(e, result) {
            t.equal(result.value[0].type, ENUM('BOOL'));
            t.equal(result.value[0].bool_value, true);

            var obj = stl.toObject(fs.readFileSync(out).toString());
            t.equal(obj.facets.length, 56);
            t.end();
          });
        });
      });
    });
  });
});

test('op_union - invalid handle in loop', function(child, t) {
  child.cube(0, 0, 0, 10, 10, 10, function(e, cube1) {
    child.cube(5, 5, 5, 10, 10, 10, function(e, cube2) {
      child.op_union([
        { type : ENUM('SHAPE_HANDLE'), uint32_value: cube1.value[0].uint32_value },
        { type : ENUM('SHAPE_HANDLE'), uint32_value: cube2.value[0].uint32_value },
        { type : ENUM('SHAPE_HANDLE'), uint32_value: 0 },
      ], function(e, r) {
        t.equal(r.value[0].type, ENUM('ERROR'));
        t.end();
      });
    });
  });
});

test('op_union - invalid handle in loop', function(child, t) {
  child.cube(0, 0, 0, 10, 10, 10, function(e, cube1) {
    child.cube(5, 5, 5, 10, 10, 10, function(e, cube2) {
      child.op_union([
        { type : ENUM('SHAPE_HANDLE'), uint32_value: cube1.value[0].uint32_value },
        { type : ENUM('SHAPE_HANDLE'), uint32_value: cube2.value[0].uint32_value },
        { type : ENUM('SHAPE_HANDLE'), uint32_value: 0 },
      ], function(e, r) {
        t.equal(r.value[0].type, ENUM('ERROR'));
        t.end();
      });
    });
  });
});

test('op_cut - two cubes', function(child, t) {
  child.cube(0, 0, 0, 20, 20, 10, function(e, cube1) {
    child.cube(10, 10, 0, 5, 5, 20, function(e, cube2) {
      child.op_cut([
        { type : ENUM('SHAPE_HANDLE'), uint32_value: cube1.value[0].uint32_value },
        { type : ENUM('SHAPE_HANDLE'), uint32_value: cube2.value[0].uint32_value }
      ], function(e, cut) {
        t.equal(cut.value[0].type, ENUM('SHAPE_HANDLE'));
        t.ok(cut.value[0].uint32_value !== 0);

        var out = tmpdir + 'op_cut.2cubes.stl';

        child.export_stl([
          { type : ENUM('STRING'), string_value: out },
          { type : ENUM('SHAPE_HANDLE'), uint32_value: cut.value[0].uint32_value },
        ], function(e, result) {
          t.equal(result.value[0].type, ENUM('BOOL'));
          t.equal(result.value[0].bool_value, true);

          var obj = stl.toObject(fs.readFileSync(out).toString());
          t.equal(obj.facets.length, 32);
          t.end();
        });
      });
    });
  });
});
