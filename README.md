
# LibTorch C++

A C++ project for machine learning using PyTorch's C++ API.

## Overview

This project is to learn how to use LibTorch at my own pacce.
I wanted to start understaning unsupervised learning and the most comon place to start
was PCA (Principal Component Analysis) and K-Means clustering.
I will update more according to the progress I am able to make over time.

## Features

- **C++ Integration**: Direct use of LibTorch C++ API
- **Performance**: Native C++ execution for optimal speed

## Getting Started
- Ch 01. *build name:* `minimal` After building the system a simple implementation of silly `torch::Tensor` object to check if the installation was done correctly
- Ch 02. *build name:* `pca` Trying to learn the concept of PCA how to make SVD (Singular Value Decomposition) which is the standard mathematical way to do PCA.
- Ch 03. *build name:* `pca_k-means` Trtying to apply PCA to a multidementional sample to be able to simulate a more real case scenario. In addition use the results to understand and learn K-Means Clustering.

### Prerequisites

- C++17 or later
- CMake 3.10+
- LibTorch C++ libraries

### Building

#### Simplest way 
```bash
mkdir build
cd build
cmake ..
make
```

#### More complex way
```bash
# Configuration
cmake -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -DCMAKE_C_COMPILER:FILEPATH=/usr/bin/gcc -DCMAKE_CXX_COMPILER:FILEPATH=/usr/bin/g++ --no-warn-unused-cli -S /home/lsmon/Documents/libtorch_cpp -B /home/lsmon/Documents/libtorch_cpp/build -G Ninja

# Build all targets
cmake --build /home/lsmon/Documents/libtorch_cpp/build --config Debug --target all --
```

## Project Structure

```
pytorch_cpp/
├── src/           # Source files
├── cmake/         # CMake auxilary files such as helper functions or external configurations
├── scripts/       # Mostly python scripts used to install dependencies
└── CMakeLists.txt # Build configuration
```

## Usage

``TBD``

## Contributing

Contributions are welcome. Please follow standard C++ coding practices.

## License

Open BSD. Use it at your own risks. If you fix anything make a PR and I will love to review it and add it.
