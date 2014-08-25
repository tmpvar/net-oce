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
    // d.toString().split('\n').forEach(function(line) {
    //   console.log('>', line);
    // });
  });

  process.nextTick(function() {
    fn(duplex(child.stdin, child.stdout))
  });

  return child;
}

function test(name, fn) {
  var child = setup(function(stream) {
    createClient(stream, function(e, methods) {
      if (e) throw e;

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
  });
}

test('test the test', function(methods, t) {
  t.ok(Object.keys(methods).length);
  t.end();
})

test('stl export - no args', function(methods, t) {
  methods.export_stl(function(e, r) {
    t.ok(e);
    t.notOk(r);
    t.end();
  });
});

test('stl export - invalid SHAPE_HANDLE', function(methods, t) {
  methods.prim_cube(10, function(e, cube) {
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
  methods.prim_cube(10, function(e, cube) {

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
  methods.prim_cube(10, function(e, cube1) {
    t.equal(cube1.id, 1);

    methods.prim_cube(10, function(e, cube2) {

      methods.op_translate(cube2, 20, 0, 0, function(e, cube3) {
        t.equal(cube3.id, 3);

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
});

test('cube - no args', function(methods, t) {
  methods.prim_cube(function(e) {
    t.ok(e)
    t.end();
  });
});

test('cube', function(methods, t) {
  methods.prim_cube(10, function(e, cube) {
    t.equal(cube.id, 1);
    t.end();
  });
});

test('box', function(methods, t) {
  methods.prim_box(10, 10, 10, function(e, box) {
    t.equal(box.id, 1);
    t.end();
  });
});

test('op_union - 2 cubes', function(methods, t) {
  methods.prim_cube(10, function(e, cube1) {
    methods.prim_box(20, 10, 10, function(e, cube2) {
      methods.op_union(cube1, cube2, function(e, unioned) {
        t.equal(unioned.id, 3)

        var out = tmpdir + 'op_union.stl'

        methods.export_stl(out, unioned, function(e, result) {
          t.equal(result, true)

          var obj = stl.toObject(fs.readFileSync(out).toString());
          t.equal(obj.facets.length, 28);
          t.end();
        });
      });
    });
  });
});

test('op_union - 3 cubes', function(methods, t) {
  methods.prim_cube(10, function(e, cube1) {
    methods.prim_box(5, 100, 5, function(e, cube2) {
      methods.prim_box(100, 5, 5, function(e, cube3) {
        methods.op_union(cube1, cube2, cube3, function(e, unioned) {

          var out = tmpdir + 'op_union.3cubes.stl';

          methods.export_stl(out, unioned, function(e, result) {
            t.equal(result, true);

            var obj = stl.toObject(fs.readFileSync(out).toString());
            t.equal(obj.facets.length, 76);
            t.end();
          });
        });
      });
    });
  });
});

test('op_union - invalid handle', function(methods, t) {
  methods.prim_box(100, 5, 10, function(e, cube1) {
    methods.prim_cube(10, function(e, cube2) {
      methods.op_union(cube1, cube2, { id: 0 }, function(e, r) {
        t.ok(e)
        t.end();
      });
    });
  });
});

test('op_union - invalid handle', function(methods, t) {
  methods.prim_cube(10, function(e, cube1) {
    methods.prim_box(100, 5, 5, function(e, cube2) {
      methods.op_union(cube1, cube2, 0, function(e, r) {
        t.ok(e);
        t.end();
      });
    });
  });
});

test('op_cut - two boxes', function(methods, t) {
  methods.prim_box(20, 20, 10, function(e, cube1) {
    methods.prim_box(5, 5, 20, function(e, cube2) {
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

test('cube - 10 in one tick', function(methods, t) {
  var total = 10;
  var seen = 0;
  function after(e, r) {
    t.ok(!e && r);
    seen++;
    if (seen === total) {
      t.end();
    }
  }

  for (var i = 0; i<total; i++) {
    methods.prim_cube(10 * (i + 1), after);
  }
});

_test('partial length message', function(t) {
  var child = setup(function(stream) {

    stream.once('data', function(d) {
      t.ok(d);
      child.kill();
      t.end();
    });

    var buffer = createClient.encodeRequest({ method: 0, seq: 0 });
    var where = 0;
    setTimeout(function write() {
      var slice = buffer.slice(where, where+1);
      stream.write(slice);
      where++;

      if (where < buffer.length) {
        setTimeout(write, 16);
      }
    }, 100);
  });
});

test('extract verts', function(methods, t) {
  methods.prim_cube(10, function(e, cube) {
    t.ok(cube);
    methods.extract_verts(cube, function(e, r) {
      t.equal((r.length/4)/9, 12);
      t.end();
    });
  })
});
