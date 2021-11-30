# Postgres C++ interfacing library : pgi

pgi is a header only library built on top of [libpqxx](https://github.com/jtv/libpqxx.git) and [cpp-yaml](https://github.com/jbeder/yaml-cpp.git). It is essentially a wrapper around libpqxx that allows configuration from yaml file and higher level methods interfacing C++ with postgres.

## Requirements

1. [libpqxx](https://github.com/jtv/libpqxx.git) installed
2. [cpp-yaml](https://github.com/jbeder/yaml-cpp.git) installed


## Try it with docker-compose

A minimal working environment (for the sake of example + continuous deployment) can be found in [ci/](ci/). It sets up a minimal postgres database in one container, builds minimal example using pgi in another container and runs it.

```docker-compose up```

Will end with output :  

```
pgi_1             | 3.1 0.2     4.7
pgi_1             | 3.5 0.5     4.9
ci_pgi_1 exited with code 0
```
