# xnode library

The `xnode` library is a part of the `xsdk` framework to create high performance video processing and management tools.

The `xnode` library provides a set of C++ classes and utilities for working with tree data structures which used in `xsdk`.
The `xnode` data format is natively compatible with JSON, enabling it to handle these data format smoothly and without issue.
The library is thread-safe and allows multiple threads to access it concurrently.
It supports importing and exporting data from/to XML and JSON formats.

## Features

- `INode`:    A base interface for creating node objects.
- `xnode`:    A namespace providing utilities for working with `INode` via smart pointers.
- `XKey`:     XKey class is a variant wrapper to store different types of keys.
- `XPath`:    A deque (double-ended queue) container for XKey objects representing an XPath
- `XValue`:   A utility class used for storing and manipulating values.
- `XTimed`:   A templated class XTimed that extends TValue and stores a timestamp.
- `XValueRT`: XValueRT is a wrapper of XValue class using UniqueClock to generate unique monotonic timestamps.

## Usage

Examples of usage you can find in the tests.

## Build and Dependencies

The xnode library can be built using a C++17 compatible compiler. There are a few external dependencies:

 - `Gtest` for execute unit tests. [Read more...](https://google.github.io/googletest/)
 - `RapidJSON` for import and export to/from JSON format. [Read more...](https://rapidjson.org/)
 - `Xerces-C++` for import and export to/from XML format. [Read more...](https://xerces.apache.org/xerces-c/)

The library can be built with following command:
 ```shell
 cmake -S . -B build
 cmake --build build
 ```

Also to automatically resolve dependencies it can be build with using Conan package manager:
```
cmake -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES="xsdk_conan.cmake" -S . -B build
cmake --build build
```

## License

The xnode library is licensed under the [GPL v3 License](LICENSE).