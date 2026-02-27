// config.h.in
#pragma once

// These will be replaced by CMake
#define PROJECT_NAME "minimal_libtorch"
#define PROJECT_VERSION "0.1.0"

#define LIBTORCH_VERSION "2.5.0"
#define SCRIPTS_MODLES_PATH "/home/lsmon/Documents/libtorch_cpp/scripts/models"
#define MODEL_RESNET18 "/home/lsmon/Documents/libtorch_cpp/models/resnet18_scriptmodule.pt"
#define MODEL_SAVE_PATH "/home/lsmon/Documents/libtorch_cpp/models/output/model.pt"


// You can even pass paths
#define TORCH_INSTALL_PREFIX "/home/lsmon/Documents/libtorch_cpp/libtorch"

#define OCID_DSET_PATH "/home/lsmon/Documents/libtorch_cpp/data/ocid"

#define CUDA_ENABLED TRUE

#ifdef CUDA_ENABLED


#define CUDA_CHECK(err)                                                    \
    do {                                                                   \
        cudaError_t _e = (err);                                            \
        if (_e != cudaSuccess) {                                           \
            std::cerr << "CUDA error " << _e << " at " << __FILE__     \
                      << ":" << __LINE__ << " -> "                    \
                      << cudaGetErrorString(_e) << std::endl;             \
            std::exit(EXIT_FAILURE);                                       \
        }                                                                  \
    } while (0)

#define CUBLAS_CHECK(err)                                                  \
    do {                                                                   \
        cublasStatus_t _e = (err);                                         \
        if (_e != CUBLAS_STATUS_SUCCESS) {                                 \
            std::cerr << "cuBLAS error " << _e << " at " << __FILE__   \
                      << ":" << __LINE__ << std::endl;                 \
            std::exit(EXIT_FAILURE);                                       \
        }                                                                  \
    } while (0)

#define CUSOLVER_CHECK(err)                                                \
    do {                                                                   \
        cusolverStatus_t _e = (err);                                       \
        if (_e != CUSOLVER_STATUS_SUCCESS) {                               \
            std::cerr << "cuSolver error " << _e << " at " << __FILE__ \
                      << ":" << __LINE__ << std::endl;                 \
            std::exit(EXIT_FAILURE);                                       \
        }                                                                  \
    } while (0)

#endif
