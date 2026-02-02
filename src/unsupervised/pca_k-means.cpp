#include <torch/torch.h>
#include <iostream>
#include <iomanip>
#include <vector>


torch::Tensor sampleSimulator(int n_samples, int n_features) {
    // Base "true" location influence (simplified: stronger signal closer to "origin")
    auto dist_factor = torch::rand({n_samples}) * 0.8 + 0.2;  // 0.2–1.0 distance factor

    // Create correlated signal strengths (stronger near "home" cell, weaker far away)
    std::vector<torch::Tensor> columns;
    // Simulate 3 main "serving" cells with decay + noise
    for (int i = 0; i < 8; ++i) {   // 8 visible sectors
        float base = -55.0 - i * 8.0;           // closer cells stronger
        auto decay = torch::pow(dist_factor, 1.5 + i * 0.3);    // [500]
        auto group = base * decay.unsqueeze(1).expand({-1, 3})  // [500, 1] 
               + torch::randn({n_samples, 3}) * 4.0f;           // [500, 3]
        for (int j = 0; j < 3; ++j) {
            columns.push_back(group.slice(1, j, j+1));
        }
    }

    // Add timing advance columns (roughly increase with distance)
    for (int i = 0; i < 8; ++i) {
        auto ta = 0.1f + dist_factor.unsqueeze(1) * (3.0f + i * 0.5f)
                + torch::randn({n_samples, 1}) * 0.4f;
        columns.push_back(ta);
    }
    return torch::cat(columns, 1);  // final fingerprint matrix [n_samples × n_features]
}

torch::Tensor pca(torch::Tensor data){
     // ────────────────────────────────────────────────
    // PCA
    // ────────────────────────────────────────────────
    auto mean = data.mean(0, true);
    auto centered = data - mean;

    // Use economy Singular Value Decomposition
    std::cout << "CUDA available: " << (torch::cuda::is_available() ? "Yes" : "No") << "\n";
    std::cout << "cuDNN available: " << (torch::cuda::cudnn_is_available() ? "Yes" : "No") << "\n";
    auto svd_result = std::tuple<torch::Tensor, torch::Tensor, torch::Tensor>();
    if (torch::cuda::is_available()) {
        svd_result = torch::linalg::svd(centered,
                                        /*full_matrices=*/false,
                                        /*driver=*/"gesdd");
    } else {
        svd_result = torch::linalg::svd(centered,
                                        /*full_matrices=*/false,
                                        /*driver=*/std::nullopt);
    }

    auto U  = std::get<0>(svd_result);
    auto S  = std::get<1>(svd_result);
    auto Vh = std::get<2>(svd_result);
    auto V  = Vh.transpose(-2, -1);

    // Explained variance
    auto explained_var = S.pow(2) / (centered.size(0) - 1);
    auto total_var     = explained_var.sum();
    auto ratio         = explained_var / total_var;

    std::cout << "Explained variance ratios (first 12 components):\n";
    float cumulative = 0.0f;
    int keep_components = 0;

    for (int i = 0; i < std::min(15, (int)ratio.size(0)); ++i) {
        float pct = ratio[i].item<float>() * 100.0f;
        cumulative += pct;
        std::cout << "PC" << (i+1) << ": " << pct << "%   (cumulative: " << cumulative << "%)\n";

        if (cumulative >= 92.0f && keep_components == 0) {
            keep_components = i + 1;
        }
    }
    if (keep_components == 0) keep_components = 12;  // fallback

    std::cout << "\n→ Keeping first " << keep_components << " components "
              << "(explains ~" << cumulative << "% variance)\n\n";

    // Project data to reduced space
    auto V_reduced = V.slice(1, 0, keep_components);
    return torch::matmul(centered, V_reduced);
}

int main() {
    std::cout << std::fixed << std::setprecision(4);
    torch::manual_seed(123);

    // ────────────────────────────────────────────────
    // Simulate realistic RF fingerprint data
    // ────────────────────────────────────────────────
    int n_samples   = 500;          // many location measurements
    int n_features  = 24;           // e.g. RSSI from 8 sectors × 3 bands + TA from 8 cells

    auto data = sampleSimulator(n_samples, n_features);

    std::cout << "Generated RF fingerprint data shape: [" << data.size(0)
              << ", " << data.size(1) << "]\n\n";

    auto projected = pca(data);

    // ────────────────────────────────────────────────
    // K-Means in reduced (PCA) space
    // ────────────────────────────────────────────────
    std::cout << "=== K-Means in PCA-reduced space ===\n\n";

    int K = 6;              // e.g. 6 location zones / grids
    int max_iters = 150;
    float tol = 1e-5;

    auto perm = torch::randperm(n_samples);
    auto centroids = projected.index_select(0, perm.slice(0, 0, K)).clone();

    torch::Tensor labels;

    int iter;
    for (iter = 0; iter < max_iters; ++iter) {
        auto dists = torch::cdist(projected, centroids);

        torch::Tensor min_dists;
        std::tie(min_dists, labels) = dists.min(1);

        auto new_centroids = torch::zeros_like(centroids);

        for (int k = 0; k < K; ++k) {
            auto mask = (labels == k);
            auto nz = mask.nonzero().squeeze(1);
            if (nz.size(0) > 0) {
                new_centroids[k] = projected.index_select(0, nz).mean(0);
            } else {
                new_centroids[k] = centroids[k];
            }
        }

        auto shift = (new_centroids - centroids).norm();
        centroids = new_centroids;

        if (iter % 20 == 0 || iter == max_iters - 1) {
            std::cout << "Iter " << iter << "  shift = " << shift.item<float>() << "\n";
        }

        if (shift.item<float>() < tol) break;
    }

    std::cout << "\nConverged after " << iter << " iterations.\n";
    std::cout << "Cluster sizes:\n";
    for (int k = 0; k < K; ++k) {
        int64_t cnt = (labels == k).sum().item<int64_t>();
        std::cout << "  Zone " << k << ": " << cnt << " measurements\n";
    }

    std::cout << "\nDone.\n";
    return 0;
}