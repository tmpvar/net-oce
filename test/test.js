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

    methods.export_stl('cube.stl', cube, function(e, result) {
      t.ok(result);

      var obj = stl.toObject(result);
      t.equal(obj.description, 'cube.stl');
      t.equal(obj.facets.length, 12);

      var fixture = stl.toObject(
        fs.readFileSync(pjoin(__dirname, 'fixtures', 'cube.stl'))
      );

      console.log(obj.facets.map(function(f){ return f.normal; }));
      t.deepEqual(obj, fixture);

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

        methods.export_stl('two-cubes.stl', cube1, cube2, function(e, result) {
          t.ok(result);

          var obj = stl.toObject(result);
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

        methods.export_stl('op_union.stl', unioned, function(e, result) {
          t.ok(result)

          var obj = stl.toObject(result);
          t.equal(obj.facets.length, 12);
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

          methods.export_stl('op_union.3cubes.stl', unioned, function(e, result) {
            t.ok(result);

            var obj = stl.toObject(result);
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

        methods.export_stl('op_cut.2cubes.stl', cut, function(e, result) {
          t.ok(result);

          var obj = stl.toObject(result);
          t.equal(obj.facets.length, 32);
          t.end();
        });
      });
    });
  });
});

test('op_cut - two boxes from one', function(methods, t) {
  methods.prim_box(100, 100, 10, function(e, cube1) {
    methods.prim_box(5, 5, 10, function(e, cube2) {
      methods.op_translate(cube2, 40, 40, 0, function(e, cube2a) {

        methods.prim_box(20, 20, 10, function(e, cube3) {

          methods.op_cut(cube1, cube2a, cube3, function(e, cut) {
            t.ok(cut.id);

            var out = tmpdir + 'op_cut.3boxes.stl';

            methods.export_stl('op_cut.3boxes.stl', cut, function(e, result) {
              fs.writeFileSync(out, result);
              t.ok(result);

              var obj = stl.toObject(result);
              t.equal(obj.facets.length, 52);
              t.end();
            });
          });
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

test('display verts', function(methods, t) {
  methods.prim_cube(10, function(e, cube) {
    t.ok(!e)
    t.ok(cube);
    methods.shape_display(cube, function(e, r) {
      t.equal(r.positions.length/9, 12); // verts
      t.equal(r.normals.length/9, 12); // normals
      t.equal(r.features.length, 12); // features
      t.equal(r.bounds.length, 6); // bounding box
      t.end();
    });
  });
});

test('display verts (multiple)', function(methods, t) {
  methods.prim_cube(10, function(e, cube) {
    t.ok(!e)
    t.ok(cube);

    methods.prim_cube(10, function(e, cube2) {
      t.ok(!e);
      t.ok(cube2);
      methods.shape_display(cube, cube2, function(e, r) {
        t.equal(r.length, 2);

        t.equal(r[0].positions.length/9, 12); // verts
        t.equal(r[0].normals.length/9, 12); // normals
        t.equal(r[0].features.length, 12); // features
        t.equal(r[0].bounds.length, 6); // bounding box

        t.equal(r[1].positions.length/9, 12); // verts
        t.equal(r[1].normals.length/9, 12); // normals
        t.equal(r[1].features.length, 12); // features
        t.equal(r[1].bounds.length, 6); // bounding box
        t.end();
      });
    });
  });
});

test('invalid box dimensions', function(methods, t) {
  methods.prim_box(0, 1, 1, function(e) {
    t.ok(e);
    methods.prim_box(1, 0, 1, function(e) {
      t.ok(e);

      methods.prim_box(1, 1, 0, function(e) {
        t.ok(e);
        t.end();
      });
    });
  });
});

test('invalid cube dimensions', function(methods, t) {
  methods.prim_box(0, function(e) {
    t.ok(e);
    t.end();
  });
});

test('invalid cylinder dimensions', function(methods, t) {
  methods.prim_cylinder(0, 1, function(e) {
    t.ok(e);
    methods.prim_cylinder(0, 1, function(e) {
      t.ok(e);
      t.end();
    });
  });
});

test('invalid sphere dimensions', function(methods, t) {
  methods.prim_sphere(0, function(e) {
    t.ok(e);
    t.end();
  });
});

