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
#include "bldl_audio_task.h"
#include "bldl_helpers.h"
#include "bldl_manager.h"
#include "bldl_version.h"

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

DEFINE_string(file, "i_wanna_download_music.txt", "List of audios you want to download, default is current dictory i_wanna_download_music.txt");
DEFINE_validator(file, &validate_filename);

// end.
///////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {

    ::testing::InitGoogleTest(&argc, argv);

    gflags::SetVersionString(bldl::Version::_info);
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    //spdlog::set_level(spdlog::level::debug);

    //
    // single video task test
    // now it's incorrect!
    // TODO fix: _progress_bar_mgr is a member of bldl::Manager, 
    // so if you do not use the manager, the _progress_bar_mgr will not be created properly.
    
    //bldl::VideoTask vt("https://www.bilibili.com/video/BV11A411S7zS?spm_id_from=333.1007.tianma.1-2-2.click");
    //vt.run();

    //
    // Now, single download task also must use the bldl::Manager.

    //bldl::Manager::get_instance().add_task(std::make_shared<bldl::VideoTask>("https://www.bilibili.com/video/BV18K41127YW?spm_id_from=333.1007.tianma.1-1-1.click"));
    //bldl::Manager::get_instance().run_tasks();

    //
    // Similarly, an example of downloading audio.

    //bldl::Manager::get_instance().add_task(std::make_shared<bldl::AudioTask>("https://www.bilibili.com/audio/au12655"));
    //bldl::Manager::get_instance().run_tasks();

    //
    // parallel multiple tasks test

    //bldl::Manager::get_instance().parsed_address_file<bldl::VideoTask>("i_wanna_download_videos_3.txt");
    //bldl::Manager::get_instance().run_tasks();

    bldl::Manager::get_instance().parsed_address_file<bldl::AudioTask>(FLAGS_file);
    bldl::Manager::get_instance().run_tasks();

    std::clog << "\nDownload finished!" << std::endl;

    return RUN_ALL_TESTS();
}