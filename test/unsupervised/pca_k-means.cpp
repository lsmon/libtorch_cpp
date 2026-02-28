#include <torch/torch.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include "sample_factory.hpp"
#include "config.hpp"

torch::Tensor pca(torch::Tensor data)
{
    // 1. Determine Device
    auto device = torch::cuda::is_available() ? torch::kCUDA : torch::kCPU;
    std::cout << "Running PCA on: " << device << " (cuDNN: "
              << (torch::cuda::cudnn_is_available() ? "Yes" : "No") << ")\n";

    // 2. Center the data ON the device
    // By moving 'data' first, 'mean' and 'centered_on_device' are created on GPU/CPU automatically
    auto data_on_device = data.to(device);
    auto mean = data_on_device.mean(0, true);
    auto centered_on_device = data_on_device - mean;

    // 3. Configure SVD Driver
    std::optional<std::string> driver = std::nullopt;
    if (device == torch::kCUDA)
    {
        driver = "gesvdj";
    }
    else
    {
        driver = "gesdd";
    }

    // 4. Run SVD
    // Note: LibTorch returns V-Hermitian (transposed), so we call it Vh
    auto [U, S, Vh] = torch::linalg::svd(centered_on_device, false, driver);

    // 5. Variance Analysis (Logic remains same, move to CPU for the loop)
    auto explained_var = S.pow(2) / (centered_on_device.size(0) - 1);
    auto total_var = explained_var.sum();
    auto ratio = (explained_var / total_var).to(torch::kCPU);
    int max_display = std::min(15, (int)ratio.size(0));
    float cumulative = 0.0f;
    int keep_components = 0;
    std::cout << "Explained variance ratios:\n";
    for (int i = 0; i < (int)ratio.size(0); ++i)
    {
        float pct = ratio[i].item<float>() * 100.0f;
        cumulative += pct;
        if (i < max_display)
        {
            std::cout << "  PC" << (i + 1) << ": " << pct << "% (cum: " << cumulative << "%)\n";
        }

        if (cumulative >= 92.0f && keep_components == 0)
        {
            keep_components = i + 1;
        }
    }
    if (keep_components == 0)
        keep_components = 12;
    std::cout << "→ Keeping " << keep_components << " components to explain "
              << cumulative << "% variance.\n";
    // 6. Project data
    // Vh is [Components, Features]. We need the transpose of the first 'keep' rows.
    // This is equivalent to taking the first 'keep' columns of V.
    auto V = Vh.mH(); // Transpose Vh to get V [Features, Components]
    auto V_reduced = V.slice(1, 0, keep_components);

    // 7. MULTIPLY (Both tensors are on 'device')
    return torch::matmul(centered_on_device, V_reduced);
}

torch::Tensor kmeans(torch::Tensor projected, int n_samples, int K, int max_iters = 100, float tol = 1e-4)
{
    std::cout << "=== K-Means in PCA-reduced space ===\n\n";

    // 1. Get the device of the input data
    auto device = projected.device();
    // Helper to create options for indices (must be Long/Int64)
    auto index_options = torch::TensorOptions().device(device).dtype(torch::kLong);

    // 2. Initialize Centroids
    // Create 'perm' directly on the correct device
    auto perm = torch::randperm(n_samples, index_options);
    auto selection_indices = perm.slice(0, 0, K);
    auto centroids = projected.index_select(0, selection_indices).clone();

    torch::Tensor labels;

    int iter;
    for (iter = 0; iter < max_iters; ++iter)
    {
        // 3. Compute Distances (cdist supports both CPU and GPU)
        auto dists = torch::cdist(projected, centroids);

        // 4. Assign Labels
        std::tie(std::ignore, labels) = dists.min(1);

        // 5. Update Centroids
        auto new_centroids = torch::zeros_like(centroids);

        for (int k = 0; k < K; ++k)
        {
            auto mask = (labels == k);
            // mask.nonzero() will automatically be on 'device'
            auto nz = mask.nonzero().squeeze(1);

            if (nz.size(0) > 0)
            {
                new_centroids[k] = projected.index_select(0, nz).mean(0);
            }
            else
            {
                // Handle empty cluster: keep old centroid
                new_centroids[k] = centroids[k];
            }
        }

        // 6. Check for Convergence
        auto shift = (new_centroids - centroids).norm();
        centroids = new_centroids;

        // Move shift to CPU just for printing/comparison
        float shift_val = shift.item<float>();

        if (iter % 20 == 0 || iter == max_iters - 1)
        {
            std::cout << "Iter " << iter << "  shift = " << shift_val << "\n";
        }

        if (shift_val < tol)
            break;
    }

    std::cout << "\nConverged after " << iter << " iterations.\n";
    return labels;
}

int main()
{
    std::cout << std::fixed << std::setprecision(4);

    // ────────────────────────────────────────────────
    // Simulate realistic RF fingerprint data
    // ────────────────────────────────────────────────
    int n_samples = 500; // many location measurements
    int n_features = 42; // e.g. RSSI from 8 sectors × 3 bands + TA from 8 cells

    auto data = sample_factory::create_sample_data(n_samples, n_features);

    std::cout << "Generated RF fingerprint data shape: [" << data.size(0)
              << ", " << data.size(1) << "]\n\n";

    auto projected = pca(data);

    // ────────────────────────────────────────────────
    // K-Means in reduced (PCA) space
    // ────────────────────────────────────────────────
    int K = 6; // e.g. 6 location zones / grids
    int max_iters = 150;
    float tol = 1e-5;

    auto labels = kmeans(projected, n_samples, K, max_iters, tol);

    std::cout << "Cluster sizes:\n";
    for (int k = 0; k < K; ++k)
    {
        int64_t cnt = (labels == k).sum().item<int64_t>();
        std::cout << "  Zone " << k << ": " << cnt << " measurements\n";
    }

    std::cout << "\nDone.\n";
    return 0;
}