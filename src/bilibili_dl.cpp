#include <iostream>
#include <filesystem>
#include <execution>
#include <thread>
#include <chrono>
#include <cassert>

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
#include "bldl_helpers.h"
#include "bldl_manager.h"

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

DEFINE_string(file, "i_wanna_download_videos_2.txt", "List of videos you want to download, default is current dictory i_wanna_download_videos.txt");
DEFINE_validator(file, &validate_filename);

// end.
///////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    //spdlog::set_level(spdlog::level::debug);

    //
    // single task test
    
    //bldl::VideoTask vt("https://www.bilibili.com/video/BV11A411S7zS?spm_id_from=333.1007.tianma.1-2-2.click");
    //vt.run();

    //std::cout << '\n';

    //bldl::VideoTask vt2("https://www.bilibili.com/video/BV18K41127YW?spm_id_from=333.1007.tianma.1-1-1.click");
    //vt2.run();

    //
    // parallel multiple tasks test

    bldl::Manager::get_instance().parsed_address_file(FLAGS_file);
    bldl::Manager::get_instance().run_tasks();

    std::clog << "\nDownload finished!" << std::endl;

    return RUN_ALL_TESTS();
}