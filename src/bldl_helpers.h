#pragma once

#include <set>
#include <map>

#pragma warning(disable:4996)
#include "indicators/indicators.hpp"

namespace bldl {
    std::string extract_bvid(const std::string& bvu);
    void read_lines(const std::string& filename, std::set<std::string>& lines);
    void extract_bvids_from_file(const std::string& filename, std::set<std::string>& bvids);
    std::string add_query(const std::string& req, const std::pair<std::string, std::string>& query);
    std::string add_queries(const std::string& req, const std::map<std::string, std::string>& queries);
}