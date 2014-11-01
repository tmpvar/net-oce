# net-oce

a wrapper around oce that exposes it to stdio (for now)

## install

You'll need a few things to get started..

```
git clone git://github.com/solids/net-oce.git
cd net-oce
npm install
cmake . && make
```

### ubuntu

```
sudo apt-get install build-essential git clang libgl-dev libfreetype6-dev libglu-dev libprotobuf-dev protobuf-compiler
```

you'll need `cmake >= 3.0.1`
```
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

This library has been built as a supporting lib to [livecad](https://github.com/solids/livecad) and is generally used there by specifying the path to the `net-oce` binary when running livecad.

`node ./server.js --oce=../net-oce/out/bin/net-oce`


## testing

`npm test`

or during development

```nodemon -x "tools/monitor.sh" -e cxx,h,js,cpp```
