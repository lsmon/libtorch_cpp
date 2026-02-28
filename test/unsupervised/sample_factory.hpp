#ifndef SAMPLE_FACTORY_HPP
#define SAMPLE_FACTORY_HPP

#include <torch/torch.h>
#include <iostream>

class sample_factory
{
public:
    static torch::Tensor create_sample_data(int n_samples = 500, int n_features = 32)
    {
        torch::manual_seed(42);
        // Base "true" location influence (simplified: stronger signal closer to "origin")
        auto dist_factor = torch::rand({n_samples}) * 0.8 + 0.2; // 0.2–1.0 distance factor

        // Create correlated signal strengths (stronger near "home" cell, weaker far away)
        std::vector<torch::Tensor> columns;
        // calculate how many feature columns we will generate: the original scheme
        // produced 8*3 + 8 = 32 columns.  work until we reach n_features or run out.
        // int wanted = n_features;

        // first produce the "serving cell" groups of three
        for (int i = 0; i < 8 && (int)columns.size() < n_features; ++i)
        {                                                          // 8 visible sectors
            float base = -55.0 - i * 8.0;                          // closer cells stronger
            auto decay = torch::pow(dist_factor, 1.5 + i * 0.3);   // [500]
            auto group = base * decay.unsqueeze(1).expand({-1, 3}) // [500, 1]
                         + torch::randn({n_samples, 3}) * 4.0f;    // [500, 3]
            for (int j = 0; j < 3 && (int)columns.size() < n_features; ++j)
            {
                columns.push_back(group.slice(1, j, j + 1));
            }
        }

        // then add timing advance columns until we hit the target
        for (int i = 0; i < 8 && (int)columns.size() < n_features; ++i)
        {
            auto ta = 0.1f + dist_factor.unsqueeze(1) * (3.0f + i * 0.5f) + torch::randn({n_samples, 1}) * 0.4f;
            columns.push_back(ta);
        }

        // if the caller requested more features than our generator can supply
        // simply append independent noise columns to fill the gap.
        while ((int)columns.size() < n_features) {
            columns.push_back(torch::randn({n_samples, 1}));
        }

        // finally, if we overshot somehow, trim the list
        if ((int)columns.size() > n_features) {
            columns.resize(n_features);
        }

        return torch::cat(columns, 1);
    }

    static std::vector<float> create_sample_lists(int n_samples = 500, int n_features = 32)
    {
        // generate the tensor and move it to CPU
        auto tensor = create_sample_data(n_samples, n_features).to(torch::kCPU).contiguous();
        // copy data into std::vector<float>
        auto numel = tensor.numel();
        std::vector<float> out;
        out.reserve(numel);
        float *ptr = tensor.data_ptr<float>();
        out.assign(ptr, ptr + numel);
        return out;
    }
};

#endif // SAMPLE_FACTORY_HPP