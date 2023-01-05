#pragma once

#include <string>

namespace bldl {
	struct Constants {
        inline static const std::string url_bilibili_video = "https://www.bilibili.com/video/";
        inline static const std::string x_web_interface_view = "https://api.bilibili.com/x/web-interface/view";
        inline static const std::string x_player_playurl = "http://api.bilibili.com/x/player/playurl";
        inline static const std::string referer = "https://www.bilibili.com";
        inline static const std::string user_agent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/108.0.0.0 Safari/537.36";
	};
}