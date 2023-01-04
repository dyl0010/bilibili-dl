#include <iostream>
#include <cassert>

#include <spdlog/spdlog.h>
#include <gflags/gflags.h>
#include <gtest/gtest.h>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

#include <json/json.h>

#pragma warning(disable:4996)

#include "indicators/indicators.hpp"

namespace bldl {

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

    indicators::BlockProgressBar bar{
        indicators::option::BarWidth{80},
        indicators::option::Start{"["},
        indicators::option::End{"]"},
        indicators::option::PostfixText{"BV1ug411x75F"},
        indicators::option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}}
    };

    static size_t progress_callback([[maybe_unused]] void* clientp, double dltotal, double dlnow, [[maybe_unused]] double ultotal, [[maybe_unused]] double ulnow)
    {
        indicators::show_console_cursor(false);

        auto progress = (dltotal == 0) ? 0 : (dlnow / dltotal);
        //spdlog::debug("dltotal: {}, dlnow: {}, progress: {}%", dltotal, dlnow, progress * 100);
        bar.set_progress(progress * 100);

        indicators::show_console_cursor(true);
        return 0;
    }
}

DEFINE_string(url, "https://www.bilibili.com/video/BV1ug411x75F", "Video normal url of bilibili,like https://www.bilibili.com/video/BV1ug411x75F");
DEFINE_validator(url, &bldl::validate_url);

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

    try {
        curlpp::Cleanup my_cleanup;

        //curlpp::Easy my_request;
        //my_request.setOpt<curlpp::Options::Url>(bldl::test_url);
        //my_request.perform();

        std::stringstream ss;
        ss << curlpp::options::Url(video_detail_req);

        if (std::string error; !Json::parseFromStream(Json::CharReaderBuilder{}, ss, &video_detail, &error)) {
            spdlog::error("Json::parseFromStream error: {}", error);
        }

        spdlog::info(video_detail.toStyledString());

    }
    catch (curlpp::RuntimeError &e) {
        spdlog::error("{}", e.what());
    }
    catch (curlpp::LogicError& e) {
        spdlog::error("{}", e.what());
    }

    ///////////////////////////////////////////////////////////////////////////
    // video url

    if (video_detail["code"].asInt() != bldl::Response::OK)
        spdlog::warn("video_detail response: {}", video_detail["message"].asString());

    const std::string video_url_req = bldl::add_queries(bldl::x_player_playurl,
        {{"avid", video_detail["data"]["aid"].asString()}, {"cid", video_detail["data"]["cid"].asString()}});
 
    Json::Value video_url;

    try {
        std::stringstream ss;
        ss << curlpp::options::Url(video_url_req);

        if (std::string error; !Json::parseFromStream(Json::CharReaderBuilder{}, ss, &video_url, &error)) {
            spdlog::error("Json::parseFromStream error: {}", error);
        }

        spdlog::info(video_url.toStyledString());
    }
    catch (...) {
        spdlog::error("exception catched.");
    }

    ///////////////////////////////////////////////////////////////////////////
    // video stream

    if (video_url["code"].asInt() != bldl::Response::OK)
        spdlog::warn("video_url response: {}", video_url["message"].asString());

    const std::string video_stream_req = video_url["data"]["durl"][0]["url"].asString();

    spdlog::info("video_stream_req: {}", video_stream_req);
    
    FILE* file = nullptr;

    if (file = fopen("bilibili_dl_test_file.mp4", "wb"); !file)
        spdlog::error("error opening: bilibili_dl_test_file.mp4");

    try {
        curlpp::Easy req;
        req.setOpt<curlpp::options::Url>(video_stream_req);
        req.setOpt<curlpp::options::Referer>(bldl::referer);
        req.setOpt<curlpp::options::UserAgent>(bldl::user_agent);
        //req.setOpt<curlpp::options::Verbose>(true);
        req.setOpt<curlpp::options::NoProgress>(false);
        req.setOpt<curlpp::options::ProgressFunction>(std::bind(&bldl::progress_callback, nullptr, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
        req.setOpt<curlpp::options::WriteFunction>(std::bind(&bldl::write_file_callback, file, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        req.perform();
    }
    catch (...) {
        spdlog::error("exception catched.");
    }

    return RUN_ALL_TESTS();
}
