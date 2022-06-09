# Cachercise:

exploring scalable ways to manage concurrency in mochi.  The provider maintains
a simple in-memory data structure.  The client api sets and retrieves values
from this structure.

## Requirements
- bedrock

## Building


Pick your `INSTALL_PREFIX` (e.g. ${HOME}/soft/cachercise) and `BUILD_TYPE` (e.g
Debug, Release, etc)

```
    mkdir build
    cd build
    cmake .. -DENABLE_TESTS=ON -DENABLE_EXAMPLES=ON -DENABLE_BEDROCK=ON \
        -DCMAKE_INSTALL_PREFIX=... -DCMAKE_BUILD_TYPE=...
```

## Running Provider

This is a bedrock-based service.  There are some json files in `examples` to get you started.  One way to start the provider side of this benchmark:

```
    bedrock -c examples/cachercise-4x-server.json na+sm &
```

Once the provider is running the client can read the SSG group file to find the
provider.  Client tunables are in a separate JSON file.  The
`cachercise-client.json` file in `examples` is a good starting point.

### Running with jx9

bedrock will let you start the serivce with a json-like configuration language,
instead of straight json.  See examples/cachercise-server.jx9, which allows us
to vary the number of pools and execution streams like so, instead of creating
a JSON file for each configuration:

```
   bedrock na+sm -v trace --jx9 -c examples/cachercise-server.jx9 --jx9-context 'num_extra_pools=4,num_extra_xstreams=16' &

```

## Clients

The client is an MPI program.  You can find some job scripts in the `examples` directory.
