#include "utils.h"
#include <algorithm>

std::string_view ReadToken(std::string_view& s, std::string_view delimiter) {
    std::string_view token;
    if (const size_t token_pos = s.find(delimiter); token_pos != std::string_view::npos) {
        token = s.substr(0, token_pos);
        s.remove_prefix(token_pos + std::max(1ul, delimiter.size()));
    } else {
        token = s;
        s.remove_prefix(s.size());
    }
    while (!token.empty() && token.front() == ' ') token.remove_prefix(1);
    while (!token.empty() && token.back() == ' ') token.remove_suffix(1);
    return token;
}