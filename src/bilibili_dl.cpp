#include <filesystem>

#include <gflags/gflags.h>
#include <gtest/gtest.h>
#include <json/json.h>
#include <spdlog/spdlog.h>

#pragma warning(disable:4996)
#include "indicators/indicators.hpp"

#include "bldl_constants.h"
#include "bldl_helpers.h"
#include "bldl_libcurl.h"
#include "bldl_video_task.h"

///////////////////////////////////////////////////////////////////////////////
// About command line parameters parsed.

static bool validate_url([[maybe_unused]] const char* flagname, const std::string& url) {
    if (url.starts_with("https://www.bilibili.com/video/"))
        return true;
    spdlog::error("Invalid url for {}", url);
    return false;
}

static bool validate_filename([[maybe_unused]] const char* flagname, const std::string& filename) {
    return exists(std::filesystem::path{ filename });  // exists from ADL
}

DEFINE_string(url, "https://www.bilibili.com/video/BV1ug411x75F", "Video normal url of bilibili,like https://www.bilibili.com/video/BV1ug411x75F");
DEFINE_validator(url, &validate_url);

DEFINE_string(file, "i_wanna_download_videos.txt", "List of videos you want to download, default is current dictory i_wanna_download_videos.txt");
DEFINE_validator(file, &validate_filename);

// end.
///////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    spdlog::set_level(spdlog::level::debug);
    
    //
    // test
    bldl::VideoTask task(FLAGS_url);
    task.run();

    bldl::VideoTask task2("https://www.bilibili.com/video/BV1T14y1G7wm?spm_id_from=333.1007.tianma.1-2-2.click");
    task2.run();

    return RUN_ALL_TESTS();
}