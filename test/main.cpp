#include <torch/torch.h>  // This includes PyTorch stuff
#include <iostream>       // For printing to screen

int main() {
    // Set a random seed for reproducibility
    torch::manual_seed(42);  // Any number is fine

    // Create a random tensor (like a 2x3 matrix of numbers)
    auto my_tensor = torch::rand({2, 3});  // Shape: 2 rows, 3 columns

    // Print it
    std::cout << "Here's a random tensor:\n";
    std::cout << my_tensor << "\n";

    auto sum = my_tensor.sum(); 
    std::cout << sum << "\n";
    std::cout << "PyTorch C++ is working! 🎉\n";
    return 0;
}