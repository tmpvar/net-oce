var _test = require('tape');
var fs = require('fs');
var path = require('path');
var pjoin = path.join;
var spawn = require('child_process').spawn;
var oce = require('net-oce-protocol');

var request = oce.objects.NetOCE_Request;
var response = oce.objects.NetOCE_Response;

var tdir = __dirname;
var tmpdir = pjoin(tdir, 'tmp') + '/';
var exe = pjoin(tdir, '..', 'out/bin/net-oce');

var stl = require('stl');

var ENUM = oce.NetOCE_Value.type;

function argumentHintParser(hint, args) {
  var hintMap = {
    double : function(val) {
      return { type : ENUM('DOUBLE'), double_value: val };
    },
    handle: function (val) {
      return { type : ENUM('SHAPE_HANDLE'), uint32_value: val.id };
    },
    string: function(val) {
      return { type : ENUM('STRING'), string_value: val }
    }
  }

  var parts = hint.replace(/ /g, '').split(',');

  var lastHint = null;
  var repeating = false;

  return args.map(function (arg, i) {

    var hint = parts[i];
    if (typeof parts[i] === 'undefined') {
      if (repeating) {
        hint = lastHint;
      }
    } else {

      if (hint.indexOf('..') > -1) {
        repeating = true;
        hint = hint.replace('..', '');
      }

      lastHint = hint;
    }

    return hintMap[hint](args[i]);

  });
}

var Response = oce.NetOCE_Response;
var type_mapping = [
  '',
  'double_value',
  'float_value',
  'int32_value',
  'int64_value',
  'uint32_value',
  'uint64_value',
  'sint32_value',
  'sint64_value',
  'fixed32_value',
  'fixed64_value',
  'sfixed32_value',
  'sfixed64_value',
  'bool_value',
  'string_value',
  'bytes_value'
];

function getResponseArray(obj, fn) {
  var ret = [];

  var l = obj.value.length;
  for (var i=0; i<l; i++) {
    var value = obj.value[i];

    if (value.type < 16) {
      ret.push(value[type_mapping[value.type]]);
    } else {
      switch (value.type) {
        case 16: // OPERATION
        break;

        case 17: // ERROR
          return fn(new Error(value.string_value));
        break;

        case 18: // SHAPE_HANDLE
          ret.push({ id: value.uint32_value });
        break;
      }
    }
  }

  fn(null, (ret.length > 1) ? ret : ret[0]);
}


function setup(fn) {
  var child = spawn(exe, [], {
    stdio : 'pipe'
  });

  child.stdout.once('data', function(first) {
    var r = response.decode(first);

    var queue = [];
    child.stdout.on('data', function(data) {
      var fn = queue.shift();
      getResponseArray(response.decode(data), fn);
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
        }

        var obj = {
          method : id,
          seq: seq++,
          argument: argumentHintParser(op.operation.arguments, args)
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
    t.ok(e);
    t.notOk(r);
    t.end();
  });
});

test('stl export - invalid SHAPE_HANDLE', function(child, t) {
  child.cube(0, 0, 0, 10, 10, 10, function(e, cube) {
    child.export_stl('blah.stl', 0, function(e, r) {
      t.ok(e)
      t.end();
    });
  });
});

test('stl export - no shapes', function(child, t) {
  child.export_stl('no shapes', 123, function(e) {
    t.ok(e);
    t.end();
  });
});

test('stl export - cube', function(child, t) {
  child.cube(0, 0, 0, 10, 10, 10, function(e, cube) {

    t.equal(cube.id, 1);

    var out = tmpdir + 'cube.stl';

    child.export_stl(out, cube, function(e, result) {
      t.equal(result, true);

      var obj = stl.toObject(fs.readFileSync(out).toString());
      t.equal(obj.facets.length, 12);

      t.end();
    });
  });
});

test('stl export - 2 cubes', function(child, t) {
  child.cube(0, 0, 0, 10, 10, 10, function(e, cube1) {
    t.equal(cube1.id, 1);

    child.cube(20, 0, 0, 10, 10, 10, function(e, cube2) {
      t.equal(cube2.id, 2);

      var out = tmpdir + 'two-cubes.stl';

      child.export_stl(out, cube1, cube2, function(e, result) {
        t.equal(result, true);

        var obj = stl.toObject(fs.readFileSync(out).toString());
        t.equal(obj.facets.length, 24);

        t.end();
      });
    });
  });
});

test('cube - no args', function(child, t) {
  child.cube(function(e) {
    t.ok(e)
    t.end();
  });
});

test('cube', function(child, t) {
  child.cube(0, 0, 0, 10, 10, 10, function(e, cube) {
    t.equal(cube.id, 1);
    t.end();
  });
});

test('op_union - 2 cubes', function(child, t) {
  child.cube(0, 0, 0, 10, 10, 10, function(e, cube1) {
    child.cube(5, 5, 5, 10, 10, 10, function(e, cube2) {
      child.op_union(cube1, cube2, function(e, unioned) {
        t.equal(unioned.id, 3)

        var out = tmpdir + 'op_union.stl'

        child.export_stl(out, unioned, function(e, result) {
          t.equal(result, true)

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
        child.op_union(cube1, cube2, cube3, function(e, unioned) {

          var out = tmpdir + 'op_union.3cubes.stl';

          child.export_stl(out, unioned, function(e, result) {
            t.equal(result, true);

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
      child.op_union(cube1, cube2, { id: 0 }, function(e, r) {
        t.ok(e)
        t.end();
      });
    });
  });
});

test('op_union - invalid handle in loop', function(child, t) {
  child.cube(0, 0, 0, 10, 10, 10, function(e, cube1) {
    child.cube(5, 5, 5, 10, 10, 10, function(e, cube2) {
      child.op_union(cube1, cube2, 0, function(e, r) {
        t.ok(e);
        t.end();
      });
    });
  });
});

test('op_cut - two cubes', function(child, t) {
  child.cube(0, 0, 0, 20, 20, 10, function(e, cube1) {
    child.cube(10, 10, 0, 5, 5, 20, function(e, cube2) {
      child.op_cut(cube1, cube2, function(e, cut) {
        t.ok(cut.id);

        var out = tmpdir + 'op_cut.2cubes.stl';

        child.export_stl(out, cut, function(e, result) {
          t.equal(result, true);

          var obj = stl.toObject(fs.readFileSync(out).toString());
          t.equal(obj.facets.length, 32);
          t.end();
        });
      });
    });
  });
});
