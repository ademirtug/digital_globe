#pragma once
#include <memory>
#include <string>
#include <vector>
class http_client {
public:
    static std::shared_ptr<std::vector<unsigned char>> get_binary_page(const std::string& url);
    static std::shared_ptr<std::string> get_page(const std::string& url) {
        auto bytes = get_binary_page(url);
        return std::make_shared<std::string>(bytes->begin(), bytes->end());
    }
};
