#include "bldl_helpers.h"
#include "bldl_constants.h"

#include <fstream>
#include <cassert>
#include <filesystem>

#include <curl/curl.h>
#include <spdlog/spdlog.h>
#include <gtest/gtest.h>

namespace bldl {
    namespace fs = std::filesystem;

    std::string extract_bvid(const std::string& bvu) {
        if (!bvu.starts_with(Constants::url_bilibili_video)) {
            spdlog::error("extract_bvid invalid parameter: {}", bvu);
            return "";
        }

        auto it = std::find_if(bvu.begin() + Constants::url_bilibili_video.length(), bvu.end(), [](char c) {return !std::isalnum(c); });
        auto rnt = std::string(bvu.begin() + Constants::url_bilibili_video.length(), it);

        spdlog::debug("bldl::extract_bvid() return: {}", rnt);
        return rnt;
    }

    TEST(UrlParsedHelpersTest, ExtractBvidFunc) {
        EXPECT_EQ("BV1424y1m7mF", extract_bvid("https://www.bilibili.com/video/BV1424y1m7mF"));
        EXPECT_EQ("BV1xd4y1a7kz", extract_bvid("https://www.bilibili.com/video/BV1xd4y1a7kz/?spm_id_from=333.1007.tianma.1-1-1.click"));
        EXPECT_EQ("BV1ug411x75F", extract_bvid("https://www.bilibili.com/video/BV1ug411x75F?spm_id_from=333.1007.tianma.1-3-3.click"));
        EXPECT_EQ("", extract_bvid("https://www.google.com"));
    }

    void read_lines(const std::string& filename, std::set<std::string>& lines) {
        std::ifstream ifstrm(filename, std::ios::in);
        std::string line;
        size_t line_num = 0;

        while (std::getline(ifstrm, line)) {
            ++line_num;

            if (line.empty()) {
                spdlog::warn("line ({}:{}) is empty and this will be ignored", filename, line_num);
                continue;
            }

            lines.insert(line);
        }
    }

    TEST(FileParsedHelpersTest, ReadLinesFunc) {
        std::set<std::string> lines;

        lines.clear();
        read_lines("i_wanna_download_videos.txt", lines);
        EXPECT_EQ(lines, std::set<std::string>({
            "https://www.bilibili.com/video/BV11A411S7zS?spm_id_from=333.1007.tianma.1-2-2.click",
            "https://www.bilibili.com/video/BV1jG4y1E7RY?spm_id_from=333.1007.tianma.2-2-5.click",
            "https://www.bilibili.com/video/BV18K41127YW?spm_id_from=333.1007.tianma.1-1-1.click",}));

        lines.clear();
        read_lines("i_wanna_download_videos_2.txt", lines);
        EXPECT_EQ(lines, std::set<std::string>({
            "https://www.bilibili.com/video/BV1jg411x7FL?spm_id_from=333.1007.tianma.2-2-5.click",
            "https://www.bilibili.com/video/BV1RG4y1m71J?spm_id_from=333.1007.tianma.2-2-5.click",
            "https://www.bilibili.com/video/BV1zY411Z7PX?spm_id_from=333.1007.tianma.1-1-1.click",
            "https://www.bilibili.com/video/BV1eM411m7mq?spm_id_from=333.1007.tianma.3-2-8.click",
            "https://www.bilibili.com/video/BV19M41127S6?spm_id_from=333.1007.tianma.2-2-5.click",}));
    }

    void extract_bvids_from_file(const std::string& filename, std::set<std::string>& bvids) {
        std::ifstream ifstrm(filename, std::ios::in);
        std::string url;
        size_t line_num = 0;

        while (std::getline(ifstrm, url)) {
            ++line_num;

            auto bvid = extract_bvid(url);

            if (bvid.empty()) {
                spdlog::warn("bvid extracted from url ({}:{}:{}) is empty and this will be ignored", filename, line_num, url);
                continue;
            }

            bvids.insert(bvid);
        }
    }

    TEST(FileParsedHelpersTest, ExtractBvidFromFileFunc) {
        std::set<std::string> bvids;

        bvids.clear();
        extract_bvids_from_file("i_wanna_download_videos.txt", bvids);
        EXPECT_EQ(bvids, std::set<std::string>({ "BV11A411S7zS", "BV1jG4y1E7RY", "BV18K41127YW", }));

        bvids.clear();
        extract_bvids_from_file("i_wanna_download_videos_2.txt", bvids);
        EXPECT_EQ(bvids, std::set<std::string>({ "BV1jg411x7FL", "BV1RG4y1m71J", "BV1zY411Z7PX", "BV1eM411m7mq", "BV19M41127S6", }));
    }

    std::string add_query(const std::string& req, const std::pair<std::string, std::string>& query) {
        assert(!query.first.empty() && !query.second.empty());

        bool first = !req.ends_with("&");
        auto rnt = std::format("{}{}{}={}&", req, first ? "?" : "", query.first, query.second);

        spdlog::debug("bldl::add_query() return: {}", rnt);
        return rnt;
    }

    TEST(UrlParsedHelpersTest, AddQueryFunc) {
        EXPECT_EQ("https://api.bilibili.com/x/web-interface/view?bvid=BV1YY41127LR&", add_query("https://api.bilibili.com/x/web-interface/view", { "bvid", "BV1YY41127LR" }));
        EXPECT_EQ("https://api.bilibili.com/x/player/playurl?avid=771650346&cid=796049063&", add_query("https://api.bilibili.com/x/player/playurl?avid=771650346&", { "cid", "796049063" }));
        //EXPECT_EQ("https://api.bilibili.com/x/web-interface/view", add_query("https://api.bilibili.com/x/web-interface/view", {}));
    }

    std::string add_queries(const std::string& req, const std::map<std::string, std::string>& queries) {
        std::string append;

        std::ranges::for_each(queries, [&](auto const& query) {
            auto const& [field, value] = query;
        assert(!field.empty() && !value.empty());
        append += std::format("{}={}&", field, value);
            });

        bool first = !req.ends_with("&");
        auto rnt = std::format("{}{}{}", req, first ? "?" : "", append);

        spdlog::debug("bldl::add_queries() return: {}", rnt);
        return rnt;
    }

    TEST(UrlParsedHelpersTest, AddQueriesFunc) {
        EXPECT_EQ("https://api.bilibili.com/x/player/playurl?avid=771650346&cid=796049063&", add_queries("https://api.bilibili.com/x/player/playurl", { {"avid", "771650346"}, {"cid", "796049063"} }));
        //EXPECT_EQ("https://api.bilibili.com/x/player/playurl", add_queries("https://api.bilibili.com/x/player/playurl", {}));
    }
}