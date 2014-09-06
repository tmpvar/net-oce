# net-oce

a wrapper around oce that exposes it to stdio (for now)

## install

You'll need a few things to get started..

### ubuntu

```
sudo apt-get install build-essential git clang libgl-dev libfreetype6-dev libglu-dev libprotobuf-dev protobuf-compiler

curl http://www.cmake.org/files/v3.0/cmake-3.0.1.tar.gz | tar xz
cd cmake-3.0.1
./configure && make && sudo make install
```

### osx

`brew install libuv protobuf`

```
git clone https://github.com/tpaviot/oce.git
cd oce
cmake . && make && sudo make install
```

## use

This is a bit rough right now, but I've been doing

`cmake . && make && node test/run.js | ./out/bin/net-oce`

to test changes
