#pragma once

#include <set>
#include <map>

namespace bldl {
    enum Response {
        OK,
    };

    std::string extract_bvid(const std::string& bvu);
    void extract_bvids_from_file(const std::string& filename, std::set<std::string>& bvids);
    std::string add_query(const std::string& req, const std::pair<std::string, std::string>& query);
    std::string add_queries(const std::string& req, const std::map<std::string, std::string>& queries);
    // libcurl progress callback.
    int progress_callback([[maybe_unused]] void* clientp, double dltotal, double dlnow, [[maybe_unused]] double ultotal, [[maybe_unused]] double ulnow);
}