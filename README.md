
# LibTorch C++

A C++ project for machine learning using PyTorch's C++ API.

## Overview

This repository is a learning playground for PyTorch's C++ API (LibTorch).
It demonstrates unsupervised algorithms (PCA and K‑means) on both CPU and
CUDA, and gradually descends to raw CUDA kernels so you can see what the
high‑level tensor calls actually do on the GPU.

The history of the exercises is:

1. `minimal` – a tiny program that verifies the LibTorch installation by
   constructing a simple tensor and printing it.
2. `pca` – an example that generates synthetic 2‑D correlated data and
   performs principal component analysis using `torch::linalg::svd`.
3. `pca_k‑means` – extends the previous example with a k‑means clustering
   step and more realistic (higher‑dimensional) simulated data.
4. CUDA variants (`pca_cuda`, `pca_kmeans_cuda`) – the same algorithms but
   executing on the GPU, with explicit device handling and calls to
   cuBLAS/cuSolver.
5. `unsupervised/cuda/*.cu` – handwritten CUDA kernels plus cuBLAS and
   cuSolver calls that replicate the PCA/​k‑means logic without any
   LibTorch dependency. These files serve as a reference to show how the
   tensor API is implemented under the hood.

A `sample_factory` helper is included to produce tunable fingerprint‑like
input data; it accepts an `n_features` argument and will expand/trim the
output accordingly.

## Features

- **C++ Integration**: direct usage of LibTorch and optional raw CUDA
  code.
- **Automatic dependency fetch**: CMake scripts download libtorch,
  Cassandra driver and MNIST dataset on first run.
- **CUDA support**: the build detects the CUDA Toolkit and builds GPU
  variants automatically; the installer script can fetch CUDA‑enabled
  libtorch binaries.
- **Testing & benchmarking**: CTest targets run every executable and
  report timings for simple benchmarks.
- **Flexible data generation**: `sample_factory` produces synthetic data
  with a user‑controlled number of features.

## Getting Started
- Ch 01. *build name:* `minimal` After building the system a simple implementation of silly `torch::Tensor` object to check if the installation was done correctly
- Ch 02. *build name:* `pca` Trying to learn the concept of PCA how to make SVD (Singular Value Decomposition) which is the standard mathematical way to do PCA.
- Ch 03. *build name:* `pca_k-means` Trtying to apply PCA to a multidementional sample to be able to simulate a more real case scenario. In addition use the results to understand and learn K-Means Clustering.

### Prerequisites

- C++17 or later
- CMake 3.10+
- LibTorch C++ libraries (the project will download these automatically using the included Python installer)
- Python 3 (required by the helper scripts under `scripts/` – e.g. the `libtorch-install.py` and Cassandra driver installer)
- Docker (used to run an Apache Cassandra container for the database exercises)
- Access to the internet to fetch:
  - libtorch binaries (CPU or CUDA)
  - pretrained Torch models (`models/` directory)
  - CSV datasets placed under `data/ocid` which are inserted into Cassandra during tests

The build system invokes several Python scripts to fetch dependencies and
prepare datasets; make sure the `python3` command is available on your
PATH.  To exercise the database utilities you should start a Cassandra
instance, for example:

```bash
# from project root
sudo docker run --name cassandra -d -p 9042:9042 cassandra:4.0
```

and then run `make db_test` or `ctest -R db_test` to verify connectivity,
read/write access and CSV import functionality.  See
`scripts/dependencies/cassandra-driver.py` for further details on the
expected setup.

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
libtorch_cpp/
├── CMakeLists.txt             # top–level build configuration
├── cmake/                     # helper macros, dependency installers
├── scripts/                   # Python helpers used by CMake
├── src/unsupervised/          # high‑level examples
│   ├── pca.cpp                # CPU PCA demo
│   ├── pca_k-means.cpp        # CPU PCA + k‑means demo
│   └── cuda/                  # GPU and raw‑CUDA implementations
│       ├── pca_cuda.cu        # GPU version of PCA using cuBLAS/cuSolver
│       ├── pca_kmeans_cuda.cu # GPU PCA+kmeans, pure CUDA
│       └── …
├── test/unsupervised/         # alternative examples used by CTest
│   ├── pca.cpp                # nearly identical to src version
│   ├── sample_factory.hpp     # data generator helper
│   └── …
└── test/db/ …                 # Cassandra-related tests and utilities
```

Each executable is added as a CMake target; CUDA builds are only added
when `find_package(CUDAToolkit)` succeeds.  The `test/` folder also
contains a simple sample‑factory for generating input without LibTorch
on the CUDA side.

## Usage

### Building the project

```bash
mkdir build && cd build
cmake ..               # or add -DCMAKE_BUILD_TYPE=Release
cmake --build . -- -j 8 # build all targets including CUDA variants
```

The CMake script will download the required libtorch binaries
(`scripts/dependencies/libtorch-install.py`), picking a CUDA or CPU
build depending on what you request and what is installed.

### Running examples

After building you can invoke any of the examples directly from
`build/`, e.g.: `./pca`, `./pca_cuda`, `./pca_k-means`.  The CUDA
executables fall back to CPU if no GPU is present.

### Running tests & benchmarks

```bash
ctest --output-on-failure          # run all smoke tests
ctest -R run_pca_cuda              # single test
```

CTest records the wall‑clock time taken by each executable; the
current benchmarks (on the author’s machine) show that the GPU PCA is
roughly 3× faster than the CPU version, and the GPU k‑means example is
about 5× faster, although note that the CPU and CUDA k‑means examples
originally operated on different data sets (see data generator
above).

## Contributing

Pull requests are welcome!  If you add new algorithms or improve the
CUDA kernels, please update the CTest targets and README accordingly.

Coding style is mostly modern C++17; the early examples deliberately
avoid heavy abstraction to keep the learning curve gentle.

## License

Open BSD. Use it at your own risks. If you fix anything make a PR and I will love to review it and add it.
