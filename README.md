# YaDNS

A dns relay server that can be used as a local DNS over HTTPS (DoH) proxy.

## Features

- Supports relaying DNS queries with DNS over HTTPS to secure the transmisson. (UDP-based relay is also supported)

- Supports loading local resolving rules (`hosts.txt`).

- Implements an efficient cache with Trie that will cache A-typed resource records.

## Build

```shell
mkdir -p build/default
conan install ../..
cmake -GNinja ../..
ninja
```
