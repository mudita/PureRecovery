 # PureRecovery

## Description
PureRecovery is an auxiliary application which is responsible for various tasks including performing update
process, backup, etc. PureRecovery is mainly a backend providing a low-level API for LUA engine.
This way, all the business logic can be written and easily tested using the higher-level language.


## How to build

### Updater app

The application can be configured for RT1051 using a standard CMake file or the provided 
`configure.sh` script.

##### Standard CMake
```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DPRODUCT=<product> -DTARGET=rt1051 ..

where product can be BellHybrid or PurePhone
```

##### configure.sh

```
./configure.sh <product> <target> <build_type> [other cmake options]
```
Example for Pure: 
```
./configure.sh pure rt1051 Debug
```
To check available options use:
```
./configure.sh --help
```
after running the script,
_build-[product]-[target]-[build_type]_ directory will be created.

#### Building

##### Main application

```
cd <build_dir>
ninja 
```

Then updater catalog will have _PureRecovery.bin_ ready to be run on the hardware.

##### Hardware tests application

The recovery has a set of a unit tests designed to be run on the hardware.
To build it, use:

```
cd <build_dir>
ninja on-target-tests
```

`tests` catalog will contain _PureRecovery-test.bin_ ready to be run on the hardware.

##### Generating documentation
```
cd <build_dir>
ninja docs
```
Will create `docs` folder in the `<build_dir>` containing LUA API documentation in form of a webpage.