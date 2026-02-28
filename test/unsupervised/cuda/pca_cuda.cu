#include <cuda_runtime.h>
#include <cublas_v2.h>
#include <cusolverDn.h>
#include <iostream>
#include <vector>
#include <random>
#include "sample_factory.hpp"
#include "config.hpp"

// --------------------------------------------------
// simple device kernels for mean/subtraction etc.
// --------------------------------------------------
__global__ void compute_mean(const float* data, float* mean, int n, int d)
{
    int j = blockIdx.x * blockDim.x + threadIdx.x;
    if (j >= d) return;
    float sum = 0.0f;
    for (int i = 0; i < n; ++i) {
        sum += data[i * d + j];
    }
    mean[j] = sum / n;
}

__global__ void subtract_mean(float* data, const float* mean, int n, int d)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= n * d) return;
    int j = idx % d;
    data[idx] -= mean[j];
}

int main()
{
    std::cout << "=== PCA with CUDA ===\n" << std::endl;

    // ---------- generate some synthetic 2‑D data on the host ----------
    std::cout << "[1] Generating synthetic 2D data on host...\n";
    const int n = 500;
    const int d = 32;
    std::vector<float> h_data = sample_factory::create_sample_lists(n, d);

    // std::mt19937 rng(42);
    // std::normal_distribution<float> dist(0.0f, 1.0f);
    // for (int i = 0; i < n; ++i) {
    //     float x = dist(rng) * 3.0f;
    //     float y = 2.0f * x + dist(rng) * 0.5f;
    //     h_data[i * d + 0] = x;
    //     h_data[i * d + 1] = y;
    // }
    std::cout << "    Generated " << n << " samples with " << d << " features\n\n";

    // ------------------------------------------------------------------
    // allocate device buffers and copy data over
    // ------------------------------------------------------------------
    std::cout << "[2] Allocating device memory and copying data...\n";
    float* d_data = nullptr;
    float* d_mean = nullptr;
    CUDA_CHECK(cudaMalloc(&d_data, n * d * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&d_mean, d * sizeof(float)));
    CUDA_CHECK(cudaMemcpy(d_data, h_data.data(), n * d * sizeof(float),
                          cudaMemcpyHostToDevice));
    std::cout << "    Data copied to GPU\n\n";

    // compute mean and subtract it
    std::cout << "[3] Computing mean and centering data on GPU...\n";
    int threads = 256;
    int blocks_d = (d + threads - 1) / threads;
    int blocks_nd = ((n * d) + threads - 1) / threads;
    compute_mean<<<blocks_d, threads>>>(d_data, d_mean, n, d);
    CUDA_CHECK(cudaGetLastError());
    subtract_mean<<<blocks_nd, threads>>>(d_data, d_mean, n, d);
    CUDA_CHECK(cudaGetLastError());
    std::cout << "    Data centered (mean subtracted)\n\n";

    // compute covariance matrix: cov = (1/(n-1)) * centered^T * centered
    std::cout << "[4] Computing covariance matrix with cuBLAS...\n";
    cublasHandle_t cublas;
    CUBLAS_CHECK(cublasCreate(&cublas));
    const float alpha = 1.0f / (n - 1);
    const float beta  = 0.0f;
    float* d_cov = nullptr;
    CUDA_CHECK(cudaMalloc(&d_cov, d * d * sizeof(float)));

    // (d x n) times (n x d) gives (d x d)
    // note: cuBLAS is column-major, so we swap operands and transpose flags
    CUBLAS_CHECK(
        cublasSgemm(cublas,
                    CUBLAS_OP_N, CUBLAS_OP_T,
                    d, d, n,
                    &alpha,
                    d_data, d,          // treat data as n-by-d then transpose
                    d_data, d,
                    &beta,
                    d_cov, d));
    std::cout << "    Covariance matrix computed\n\n";

    // compute eigen decomposition of covariance matrix using cuSolver
    std::cout << "[5] Computing eigendecomposition with cuSolver...\n";
    cusolverDnHandle_t solver;
    CUSOLVER_CHECK(cusolverDnCreate(&solver));
    int lwork = 0;
    CUSOLVER_CHECK(cusolverDnSsyevd_bufferSize(
        solver, CUSOLVER_EIG_MODE_VECTOR, CUBLAS_FILL_MODE_UPPER,
        d, d_cov, d, nullptr, &lwork));
    float* d_work = nullptr;
    CUDA_CHECK(cudaMalloc(&d_work, lwork * sizeof(float)));
    float* d_w = nullptr;  // eigenvalues
    CUDA_CHECK(cudaMalloc(&d_w, d * sizeof(float)));
    int* devInfo = nullptr;
    CUDA_CHECK(cudaMalloc(&devInfo, sizeof(int)));

    CUSOLVER_CHECK(cusolverDnSsyevd(
        solver, CUSOLVER_EIG_MODE_VECTOR, CUBLAS_FILL_MODE_UPPER,
        d, d_cov, d, d_w, d_work, lwork, devInfo));
    std::cout << "    Eigendecomposition complete\n\n";

    // copy the first two principal components back to host
    std::cout << "[6] Copying eigenvectors back to host and displaying results...\n";
    std::vector<float> h_V(d * d);
    CUDA_CHECK(cudaMemcpy(h_V.data(), d_cov, d * d * sizeof(float),
                          cudaMemcpyDeviceToHost));
    std::cout << "\nPrincipal components (eigenvectors):\n";
    for (int i = 0; i < d; ++i) {
        std::cout << "  PC" << (i + 1) << ": [" << h_V[i * d + 0] << ", " << h_V[i * d + 1] << "]\n";
    }
    std::cout << std::endl;

    // cleanup
    std::cout << "[7] Cleaning up GPU resources...\n";
    cudaFree(d_data);
    cudaFree(d_mean);
    cudaFree(d_cov);
    cudaFree(d_w);
    cudaFree(d_work);
    cudaFree(devInfo);
    cublasDestroy(cublas);
    cusolverDnDestroy(solver);
    std::cout << "    GPU resources freed\n\n";
    std::cout << "=== PCA completed successfully ===\n";
    return 0;
}
