#include <torch/torch.h>
#include <iostream>
#include <iomanip>

int main() {
    std::cout << std::fixed << std::setprecision(4);

    torch::manual_seed(42);

    // Synthetic correlated 2D data (y ≈ 2*x + noise)
    int n_samples = 100;
    auto x = torch::randn({n_samples, 1}) * 3.0;
    auto noise = torch::randn({n_samples, 1}) * 0.5;
    auto y = 2.0 * x + noise;
    auto data = torch::cat({x, y}, 1);  // [100, 2]

    std::cout << "Original data (first 5 rows):\n" << data.slice(0, 0, 5) << "\n\n";

    // Center the data
    auto mean = data.mean(0, true);
    auto centered = data - mean;

    // SVD — now with driver argument
    auto svd_result = torch::linalg::svd(centered,
                                         /*full_matrices=*/false,
                                         /*driver=*/std::nullopt);

    auto U  = std::get<0>(svd_result);
    auto S  = std::get<1>(svd_result);
    auto Vh = std::get<2>(svd_result);

    // V = transpose of Vh
    auto V = Vh.transpose(-2, -1);

    std::cout << "Principal Components (columns of V):\n" << V << "\n\n";

    // Project onto first 2 principal components
    auto projected = torch::matmul(centered, V.slice(1, 0, 2));

    std::cout << "Projected data (first 5 rows):\n" << projected.slice(0, 0, 5) << "\n\n";

    // Explained variance ratio
    auto explained_var = S.pow(2) / (centered.size(0) - 1);
    auto total_var = explained_var.sum();
    auto ratio = explained_var / total_var;

    std::cout << "Explained variance ratio:\n"
              << "PC1: " << ratio[0].item<float>() << "\n"
              << "PC2: " << ratio[1].item<float>() << "\n\n";

    std::cout << "PCA successful!\n";
    return 0;
}