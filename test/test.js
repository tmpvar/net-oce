var _test = require('tape');
var fs = require('fs');
var path = require('path');
var pjoin = path.join;
var spawn = require('child_process').spawn;
var duplex = require('duplexer');

var createClient = require('net-oce-protocol');

var tdir = __dirname;
var tmpdir = pjoin(tdir, 'tmp') + '/';
var exe = pjoin(tdir, '..', 'out/bin/net-oce');

var stl = require('stl');

function setup(fn) {
  var child = spawn(exe, [], {
    stdio : 'pipe'
  });

  child.stderr.on('data', function(d) {
    console.log('ERROR', d.toString());
  });

  createClient(duplex(child.stdin, child.stdout), fn);

  child.stdin.write(createClient.request.encode({ method: 0, seq: 0 }));

  return child;
}

function test(name, fn) {
  var child = setup(function(e, methods) {
    _test(name, function(t) {
      var end = t.end.bind(t);
      var ended = false;
      t.end = function() {
        ended = true;
        child.kill();
        end();
      };

      child.on('close', function() {
        if (!ended) {
          t.fail("net-oce crashed");
          t.end();
        }
      });

      fn(methods, t);
    });
  });
}

test('stl export - no args', function(methods, t) {
  methods.export_stl(function(e, r) {
    t.ok(e);
    t.notOk(r);
    t.end();
  });
});

test('stl export - invalid SHAPE_HANDLE', function(methods, t) {
  methods.cube(0, 0, 0, 10, 10, 10, function(e, cube) {
    methods.export_stl('blah.stl', 0, function(e, r) {
      t.ok(e)
      t.end();
    });
  });
});

test('stl export - no shapes', function(methods, t) {
  methods.export_stl('no shapes', 123, function(e) {
    t.ok(e);
    t.end();
  });
});

test('stl export - cube', function(methods, t) {
  methods.cube(0, 0, 0, 10, 10, 10, function(e, cube) {

    t.equal(cube.id, 1);

    var out = tmpdir + 'cube.stl';

    methods.export_stl(out, cube, function(e, result) {
      t.equal(result, true);

      var obj = stl.toObject(fs.readFileSync(out).toString());
      t.equal(obj.facets.length, 12);

      t.end();
    });
  });
});

test('stl export - 2 cubes', function(methods, t) {
  methods.cube(0, 0, 0, 10, 10, 10, function(e, cube1) {
    t.equal(cube1.id, 1);

    methods.cube(20, 0, 0, 10, 10, 10, function(e, cube2) {
      t.equal(cube2.id, 2);

      var out = tmpdir + 'two-cubes.stl';

      methods.export_stl(out, cube1, cube2, function(e, result) {
        t.equal(result, true);

        var obj = stl.toObject(fs.readFileSync(out).toString());
        t.equal(obj.facets.length, 24);

        t.end();
      });
    });
  });
});

test('cube - no args', function(methods, t) {
  methods.cube(function(e) {
    t.ok(e)
    t.end();
  });
});

test('cube', function(methods, t) {
  methods.cube(0, 0, 0, 10, 10, 10, function(e, cube) {
    t.equal(cube.id, 1);
    t.end();
  });
});

test('op_union - 2 cubes', function(methods, t) {
  methods.cube(0, 0, 0, 10, 10, 10, function(e, cube1) {
    methods.cube(5, 5, 5, 10, 10, 10, function(e, cube2) {
      methods.op_union(cube1, cube2, function(e, unioned) {
        t.equal(unioned.id, 3)

        var out = tmpdir + 'op_union.stl'

        methods.export_stl(out, unioned, function(e, result) {
          t.equal(result, true)

          var obj = stl.toObject(fs.readFileSync(out).toString());
          t.equal(obj.facets.length, 36);
          t.end();
        });
      });
    });
  });
});

test('op_union - 3 cubes', function(methods, t) {
  methods.cube(0, 0, 0, 10, 10, 10, function(e, cube1) {
    methods.cube(5, 5, 5, 10, 10, 10, function(e, cube2) {
      methods.cube(-5, -5, -5, 10, 10, 5, function(e, cube3) {
        methods.op_union(cube1, cube2, cube3, function(e, unioned) {

          var out = tmpdir + 'op_union.3cubes.stl';

          methods.export_stl(out, unioned, function(e, result) {
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

test('op_union - invalid handle in loop', function(methods, t) {
  methods.cube(0, 0, 0, 10, 10, 10, function(e, cube1) {
    methods.cube(5, 5, 5, 10, 10, 10, function(e, cube2) {
      methods.op_union(cube1, cube2, { id: 0 }, function(e, r) {
        t.ok(e)
        t.end();
      });
    });
  });
});

test('op_union - invalid handle in loop', function(methods, t) {
  methods.cube(0, 0, 0, 10, 10, 10, function(e, cube1) {
    methods.cube(5, 5, 5, 10, 10, 10, function(e, cube2) {
      methods.op_union(cube1, cube2, 0, function(e, r) {
        t.ok(e);
        t.end();
      });
    });
  });
});

test('op_cut - two cubes', function(methods, t) {
  methods.cube(0, 0, 0, 20, 20, 10, function(e, cube1) {
    methods.cube(10, 10, 0, 5, 5, 20, function(e, cube2) {
      methods.op_cut(cube1, cube2, function(e, cut) {
        t.ok(cut.id);

        var out = tmpdir + 'op_cut.2cubes.stl';

        methods.export_stl(out, cut, function(e, result) {
          t.equal(result, true);

          var obj = stl.toObject(fs.readFileSync(out).toString());
          t.equal(obj.facets.length, 32);
          t.end();
        });
      });
    });
  });
});
