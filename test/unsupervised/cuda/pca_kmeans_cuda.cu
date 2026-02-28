#include <cuda_runtime.h>
#include <cublas_v2.h>
#include <cusolverDn.h>
#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include "sample_factory.hpp"
#include "config.hpp"

// simple kernels reused from pca_cuda.cu
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

// compute squared distances between n points and K centroids
__global__ void compute_dists(const float* points, const float* centroids,
                              float* dists, int n, int d, int K)
{
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= n) return;
    for (int k = 0; k < K; ++k) {
        float sum = 0.0f;
        for (int j = 0; j < d; ++j) {
            float diff = points[i * d + j] - centroids[k * d + j];
            sum += diff * diff;
        }
        dists[i * K + k] = sum;
    }
}

int main()
{
    std::cout << "=== PCA + K-Means with CUDA ===\n" << std::endl;
    const int n = 500;
    const int d = 32;

    // 1) simulate data on host
    std::cout << "[1] Generating synthetic fingerprint data (" << n << " samples x " << d 
              << " features) on host...\n";
    // std::vector<float> h_data(n * d);
    std::vector<float> h_data = sample_factory::create_sample_lists(n, d);
    // std::mt19937 rng(123);
    // std::normal_distribution<float> dist(0.0f, 1.0f);
    // for (int i = 0; i < n; ++i) {
    //     for (int j = 0; j < d; ++j) {
    //         h_data[i * d + j] = dist(rng);
    //     }
    // }
    std::cout << "    [OK] Data generated\n\n";

    float* d_data = nullptr;
    float* d_mean = nullptr;
    std::cout << "[2] Allocating device memory and copying data...\n";
    CUDA_CHECK(cudaMalloc(&d_data, n * d * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&d_mean, d * sizeof(float)));
    CUDA_CHECK(cudaMemcpy(d_data, h_data.data(), n * d * sizeof(float),
                          cudaMemcpyHostToDevice));
    std::cout << "    [OK] Data copied to GPU\n\n";

    // 2) PCA stage (reuse code from previous file)
    std::cout << "[3] === PCA STAGE ===\n";
    std::cout << "    [3.1] Computing mean and centering data...\n";
    int threads = 256;
    int blocks_d = (d + threads - 1) / threads;
    int blocks_nd = ((n * d) + threads - 1) / threads;
    compute_mean<<<blocks_d, threads>>>(d_data, d_mean, n, d);
    compute_mean<<<blocks_nd, threads>>>(d_data, d_mean, n, d);
    subtract_mean<<<blocks_nd, threads>>>(d_data, d_mean, n, d);
    CUDA_CHECK(cudaGetLastError());
    std::cout << "           [OK] Data centered\n";

    // covariance
    std::cout << "    [3.2] Computing covariance matrix (" << d << "x" << d << ")...\n";
    cublasHandle_t cublas;
    CUBLAS_CHECK(cublasCreate(&cublas));
    const float alpha = 1.0f / (n - 1);
    const float beta  = 0.0f;
    float* d_cov = nullptr;
    CUDA_CHECK(cudaMalloc(&d_cov, d * d * sizeof(float)));
    CUBLAS_CHECK(
        cublasSgemm(cublas,
                    CUBLAS_OP_N, CUBLAS_OP_T,
                    d, d, n,
                    &alpha,
                    d_data, d,
                    d_data, d,
                    &beta,
                    d_cov, d));
    std::cout << "           [OK] Covariance computed\n";

    // eigen decomposition
    std::cout << "    [3.3] Computing eigendecomposition...\n";
    cusolverDnHandle_t solver;
    CUSOLVER_CHECK(cusolverDnCreate(&solver));
    int lwork = 0;
    CUSOLVER_CHECK(cusolverDnSsyevd_bufferSize(
        solver, CUSOLVER_EIG_MODE_VECTOR, CUBLAS_FILL_MODE_UPPER,
        d, d_cov, d, nullptr, &lwork));
    float* d_work = nullptr;
    CUDA_CHECK(cudaMalloc(&d_work, lwork * sizeof(float)));
    float* d_w = nullptr;
    CUDA_CHECK(cudaMalloc(&d_w, d * sizeof(float)));
    int* devInfo = nullptr;
    CUDA_CHECK(cudaMalloc(&devInfo, sizeof(int)));
    CUSOLVER_CHECK(cusolverDnSsyevd(
        solver, CUSOLVER_EIG_MODE_VECTOR, CUBLAS_FILL_MODE_UPPER,
        d, d_cov, d, d_w, d_work, lwork, devInfo));
    std::cout << "           [OK] Eigendecomposition complete\n";

    // keep first two components
    std::cout << "    [3.4] Projecting data onto first 2 principal components...\n";
    float* d_proj = nullptr;
    CUDA_CHECK(cudaMalloc(&d_proj, n * 2 * sizeof(float)));
    float* d_evecs = d_cov;
    const float one = 1.0f, zero = 0.0f;
    CUBLAS_CHECK(cublasSgemm(cublas,
                             CUBLAS_OP_N, CUBLAS_OP_N,
                             n, 2, d,
                             &one,
                             d_data, n,
                             d_evecs, d,
                             &zero,
                             d_proj, n));
    std::cout << "           [OK] Data projected to 2D\n\n";

    // 3) run K-means on projected points entirely on device
    const int K = 6;; // e.g. 6 location zones / grids
    std::cout << "[4] === K-MEANS STAGE ===\n";
    std::cout << "    Initializing " << K << " clusters...\n";
    float* d_centroids = nullptr;
    float* d_dists = nullptr;
    int* d_labels = nullptr;
    CUDA_CHECK(cudaMalloc(&d_centroids, K * 2 * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&d_dists, n * K * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&d_labels, n * sizeof(int)));

    // initialize centroids by picking first K projected points
    CUDA_CHECK(cudaMemcpy(d_centroids, d_proj, K * 2 * sizeof(float),
                          cudaMemcpyDeviceToDevice));
    std::cout << "    [OK] Centroids initialized\n";
    std::cout << "    Running K-means iterations...\n\n";

    const int max_iters = 150;
    const float tol = 1e-5f;
    // copy initial centroids to host for convergence checking
    std::vector<float> h_prev_cent(K * 2);
    CUDA_CHECK(cudaMemcpy(h_prev_cent.data(), d_centroids, K * 2 * sizeof(float),
                          cudaMemcpyDeviceToHost));
    // labels vector lives outside the loop so we can inspect it later
    std::vector<int> h_labels(n);
    for (int iter = 0; iter < max_iters; ++iter) {
        // compute distances
        int b = (n + threads - 1) / threads;
        compute_dists<<<b, threads>>>(d_proj, d_centroids, d_dists, n, 2, K);

        // assign labels (simple sequential implementation on host)
        std::vector<float> h_dists(n * K);
        CUDA_CHECK(cudaMemcpy(h_dists.data(), d_dists, n * K * sizeof(float),
                              cudaMemcpyDeviceToHost));
        for (int i = 0; i < n; ++i) {
            float minv = h_dists[i * K + 0];
            int arg = 0;
            for (int k = 1; k < K; ++k) {
                if (h_dists[i * K + k] < minv) {
                    minv = h_dists[i * K + k];
                    arg = k;
                }
            }
            h_labels[i] = arg;
        }
        CUDA_CHECK(cudaMemcpy(d_labels, h_labels.data(), n * sizeof(int),
                              cudaMemcpyHostToDevice));

        // recompute centroids (naive host fallback for clarity)
        std::vector<float> h_cent(K * 2, 0.0f);
        std::vector<int> count(K, 0);
        std::vector<float> h_proj(n * 2);
        CUDA_CHECK(cudaMemcpy(h_proj.data(), d_proj, n * 2 * sizeof(float),
                              cudaMemcpyDeviceToHost));
        for (int i = 0; i < n; ++i) {
            int lbl = h_labels[i];
            count[lbl]++;
            h_cent[lbl * 2 + 0] += h_proj[i * 2 + 0];
            h_cent[lbl * 2 + 1] += h_proj[i * 2 + 1];
        }
        for (int k = 0; k < K; ++k) {
            if (count[k] > 0) {
                h_cent[k * 2 + 0] /= count[k];
                h_cent[k * 2 + 1] /= count[k];
            }
        }
        // check convergence on host by maximum centroid movement
        float max_shift = 0.0f;
        for (int k = 0; k < K; ++k) {
            float dx = h_cent[k * 2 + 0] - h_prev_cent[k * 2 + 0];
            float dy = h_cent[k * 2 + 1] - h_prev_cent[k * 2 + 1];
            float shift = std::sqrt(dx * dx + dy * dy);
            if (shift > max_shift) max_shift = shift;
        }

        CUDA_CHECK(cudaMemcpy(d_centroids, h_cent.data(), K * 2 * sizeof(float),
                              cudaMemcpyHostToDevice));

        if (iter % 20 == 0) {
            std::cout << "      Iter " << iter << ": max_shift = " << max_shift << "\n";
        }
        if (max_shift < tol) {
            std::cout << "      Iter " << iter << ": max_shift = " << max_shift 
                      << " (converged!)\n\n";
            break;
        }

        // update previous centroids
        h_prev_cent = h_cent;
    }

    std::cout << "[5] Computing final cluster statistics...\n";
    std::cout << "    Cluster sizes:\n";
    int total_assigned = 0;
    for (int k = 0; k < K; ++k) {
        int cnt = 0;
        for (int i = 0; i < n; ++i) {
            if (h_labels[i] == k) cnt++;
        }
        std::cout << "      Cluster " << k << ": " << cnt << " points";
        if (cnt == 0) std::cout << " (empty!)";
        std::cout << "\n";
        total_assigned += cnt;
    }
    std::cout << "    Total assigned: " << total_assigned << " / " << n << "\n\n";

    // cleanup
    std::cout << "[6] Cleaning up GPU resources...\n";
    cudaFree(d_data);
    cudaFree(d_mean);
    cudaFree(d_cov);
    cudaFree(d_proj);
    cudaFree(d_centroids);
    cudaFree(d_dists);
    cudaFree(d_labels);
    cudaFree(d_w);
    cudaFree(d_work);
    cudaFree(devInfo);
    cublasDestroy(cublas);
    cusolverDnDestroy(solver);
    std::cout << "    [OK] GPU resources freed\n\n";
    std::cout << "=== PCA + K-Means completed successfully ===\n";
    return 0;
}
