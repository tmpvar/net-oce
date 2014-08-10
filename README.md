# net-oce

a wrapper around oce that exposes it to stdio (for now)

## install

You'll need a few things to get started..

```
git clone https://github.com/tpaviot/oce.git
cd oce
cmake -G Ninja && ninja # or another build system..
sudo ninja install
```

### osx

`brew install libuv protobuf`

## use

This is a bit rough right now, but I've been doing

`cmake . && make && node test/run.js | ./out/bin/net-oce`

to test changes
