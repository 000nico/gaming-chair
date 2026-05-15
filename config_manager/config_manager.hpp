#include <vector>

namespace config {
    inline std::vector<std::string> configs;
    
    int save(const std::string& filename);
    int load(const std::string& filename);
    int del(const std::string& filename);
    std::vector<std::string> getConfigs(); 
}