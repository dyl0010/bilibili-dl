#include <iostream>
#include <fstream>
#include <cassert>
#include <filesystem>

#include <spdlog/spdlog.h>
#include <gflags/gflags.h>
#include <gtest/gtest.h>

#include <json/json.h>

#pragma warning(disable:4996)

#include "indicators/indicators.hpp"

#include "libcurl_wrapper.h"

namespace bldl {

    namespace fs = std::filesystem;

    //
    // response code.
    enum Response {
        OK,
    };

    //
    // flags validators.
    static bool validate_url([[maybe_unused]] const char* flagname, const std::string& url) {
        if (url.starts_with("https://www.bilibili.com/video/"))
            return true;
        spdlog::error("Invalid url for {}", url);
        return false;
    }

    static bool validate_filename([[maybe_unused]] const char* flagname, const std::string& filename) {
        return exists(fs::path{ filename });  // exists from ADL
    }

    // 
    // constants.
    const std::string url_bilibili_video("https://www.bilibili.com/video/");

    const std::string x_web_interface_view("https://api.bilibili.com/x/web-interface/view");

    const std::string x_player_playurl("http://api.bilibili.com/x/player/playurl");

    const std::string referer("https://www.bilibili.com");
    
    const std::string user_agent("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/108.0.0.0 Safari/537.36");

    //
    // url parsed helpers.
    std::string extract_bvid(const std::string& bvu) {
        if (!bvu.starts_with(url_bilibili_video)) {
            spdlog::error("extract_bvid invalid parameter: {}", bvu);
            return "";
        }

        auto it = std::find_if(bvu.begin() + url_bilibili_video.length(), bvu.end(), [](char c) {return !std::isalnum(c);});
        auto rnt = std::string(bvu.begin() + url_bilibili_video.length(), it);
        
        spdlog::debug("bldl::extract_bvid() return: {}", rnt);
        return rnt;
    }

    TEST(UrlParsedHelpersTest, ExtractBvidFunc) {
        EXPECT_EQ("BV1424y1m7mF", extract_bvid("https://www.bilibili.com/video/BV1424y1m7mF"));
        EXPECT_EQ("BV1xd4y1a7kz", extract_bvid("https://www.bilibili.com/video/BV1xd4y1a7kz/?spm_id_from=333.1007.tianma.1-1-1.click"));
        EXPECT_EQ("BV1ug411x75F", extract_bvid("https://www.bilibili.com/video/BV1ug411x75F?spm_id_from=333.1007.tianma.1-3-3.click"));
        EXPECT_EQ("", extract_bvid("https://www.google.com"));
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
        EXPECT_EQ(bvids, std::set<std::string>({ "BV1wv4y1B7UC", "BV19M41127S6", "BV1T14y1G7wm", }));

        bvids.clear();
        extract_bvids_from_file("i_wanna_download_videos_2.txt", bvids);
        EXPECT_EQ(bvids, std::set<std::string>({ "BV1jg411x7FL", "BV1RG4y1m71J", "BV1zY411Z7PX", "BV1eM411m7mq", "BV19M41127S6",}));
    }

    std::string add_query(const std::string& req, const std::pair<std::string, std::string> &query) {
        assert(!query.first.empty() && !query.second.empty());

        bool first = !req.ends_with("&");
        auto rnt = std::format("{}{}{}={}&", req, first ? "?" : "", query.first, query.second);

        spdlog::debug("bldl::add_query() return: {}", rnt);
        return rnt;
    }

    TEST(UrlParsedHelpersTest, AddQueryFunc) {
        EXPECT_EQ("https://api.bilibili.com/x/web-interface/view?bvid=BV1YY41127LR&", add_query("https://api.bilibili.com/x/web-interface/view", {"bvid", "BV1YY41127LR"}));
        EXPECT_EQ("https://api.bilibili.com/x/player/playurl?avid=771650346&cid=796049063&", add_query("https://api.bilibili.com/x/player/playurl?avid=771650346&", {"cid", "796049063"}));
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
        EXPECT_EQ("https://api.bilibili.com/x/player/playurl?avid=771650346&cid=796049063&", add_queries("https://api.bilibili.com/x/player/playurl", { {"avid", "771650346"}, {"cid", "796049063"}}));
        //EXPECT_EQ("https://api.bilibili.com/x/player/playurl", add_queries("https://api.bilibili.com/x/player/playurl", {}));
    }

    //
    // callback.
    size_t write_file_callback(FILE* f, char* ptr, size_t size, size_t nmemb) {
        return fwrite(ptr, size, nmemb, f);
    }

    static int progress_callback([[maybe_unused]] void* clientp, double dltotal, double dlnow, [[maybe_unused]] double ultotal, [[maybe_unused]] double ulnow)
    {
        auto bar = static_cast<indicators::BlockProgressBar*>(clientp);

        if (!bar) {
            spdlog::warn("libcurl progress_callback() called with null(clientp)");
            return CURLE_BAD_FUNCTION_ARGUMENT;
        }

        indicators::show_console_cursor(false);

        auto progress = (dltotal == 0) ? 0 : (dlnow / dltotal);
        //spdlog::debug("dltotal: {}, dlnow: {}, progress: {}%", dltotal, dlnow, progress * 100);
        bar->set_progress(static_cast<float>(progress) * 100);

        indicators::show_console_cursor(true);
        return 0;
    }
}

DEFINE_string(url, "https://www.bilibili.com/video/BV1ug411x75F", "Video normal url of bilibili,like https://www.bilibili.com/video/BV1ug411x75F");
DEFINE_validator(url, &bldl::validate_url);

DEFINE_string(file, "i_wanna_download_videos.txt", "List of videos you want to download, default is current dictory i_wanna_download_videos.txt");
DEFINE_validator(file, &bldl::validate_filename);

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    spdlog::set_level(spdlog::level::debug);

    spdlog::info(FLAGS_url);

    ///////////////////////////////////////////////////////////////////////////
    // video detail

    const std::string bvid = bldl::extract_bvid(FLAGS_url);
    const std::string video_detail_req = bldl::add_query(bldl::x_web_interface_view, { "bvid", bvid });
    Json::Value video_detail;
    std::string reply;
    clwper::Request request;

    request.set_url(video_detail_req);

    if (request.save_to_string(reply); !Json::Reader{}.parse(reply, video_detail, false)) {
        spdlog::error("video_detail_req's reply parsed failure: {}", reply);
        return -1;
    }

    spdlog::info(video_detail.toStyledString());

    ///////////////////////////////////////////////////////////////////////////
    // video url

    if (video_detail["code"].asInt() != bldl::Response::OK)
        spdlog::warn("video_detail response: {}", video_detail["message"].asString());

    const std::string video_url_req = bldl::add_queries(bldl::x_player_playurl,
        {{"avid", video_detail["data"]["aid"].asString()}, {"cid", video_detail["data"]["cid"].asString()}});
 
    Json::Value video_url;

    request.set_url(video_url_req);

    if (request.save_to_string(reply); !Json::Reader{}.parse(reply, video_url, false)) {
        spdlog::error("video_url_req's reply parsed failure: {}", reply);
        return -1;
    }

    spdlog::info(video_url.toStyledString());

    ///////////////////////////////////////////////////////////////////////////
    // video stream

    if (video_url["code"].asInt() != bldl::Response::OK)
        spdlog::warn("video_url response: {}", video_url["message"].asString());

    const std::string video_stream_req = video_url["data"]["durl"][0]["url"].asString();

    spdlog::info("video_stream_req: {}", video_stream_req);

    indicators::BlockProgressBar bar{
        indicators::option::BarWidth{80},
        indicators::option::Start{"["},
        indicators::option::End{"]"},
        indicators::option::PostfixText{"downnnnnnnnnnload~"},
        indicators::option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}}
    };

    request.set_url(video_stream_req);
    request.set_referer(bldl::referer);
    request.set_user_agent(bldl::user_agent);
    request.set_noprogress(false);
    request.set_progress_data(&bar);
    request.set_progress_function(bldl::progress_callback);
   
    request.save_to_file("bilibili-dl-test-video.mp4");

    return RUN_ALL_TESTS();
}
