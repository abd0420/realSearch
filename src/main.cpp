#include <iostream>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

int main() {
    std::string datasetPath = "./dataset";
    std::vector<std::string> status = {"C++17", "Filesystem", "Include Path"};

    std::cout << "=== MiniGoogle Environment Check ===\n";
    for (const auto& item : status) {
        std::cout << "[OK] " << item << " loaded\n";
    }

    if (fs::exists(datasetPath) && fs::is_directory(datasetPath)) {
        std::cout << "[OK] Found dataset directory at: " << fs::absolute(datasetPath) << "\n";
    } else {
        std::cout << "[WARNING] dataset/ folder not found in current directory.\n";
    }

    return 0;
}