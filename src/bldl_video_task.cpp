#include "bldl_video_task.h"
#include "bldl_helpers.h"
#include "bldl_constants.h"

#include <spdlog/spdlog.h>

namespace bldl {
	VideoTask::VideoTask(const std::string& general_url) : Task(general_url) {

		spdlog::debug("current _general_url: {}", _general_url);

		assert(!general_url.empty());

		//
		// get video detail infomation.
		_bvid = extract_bvid(_general_url);

		_progress_bar->set_option(indicators::option::PrefixText(std::format("{}'s progress ", _bvid)));

		auto detail_req = add_query(Constants::x_web_interface_view, { "bvid", _bvid });

		_request.set_url(detail_req);
		
		std::string reply;

		if (_request.save_to_string(reply); !Json::Reader{}.parse(reply, _detail_info, false)) {
			spdlog::error("_detail_info parsed failed, {}", _bvid);
			_detail_info.clear();
			return;
		}

		//spdlog::debug(_detail_info.toStyledString());

		if (_detail_info["code"].asInt() != Response::OK) {
			spdlog::error("_detail_info responses error, {}", _bvid);
			return;
		}

		//
		// get video play url.
		auto url_req = add_queries(Constants::x_player_playurl, 
			{ {"avid", _detail_info["data"]["aid"].asString()}, {"cid", _detail_info["data"]["cid"].asString()} });

		_request.set_url(url_req);

		if (_request.save_to_string(reply); !Json::Reader{}.parse(reply, _play_url, false)) {
			spdlog::error("_play_url parsed failed, {}", _bvid);
			_play_url.clear();
			return;
		}

		//spdlog::debug(_play_url.toStyledString());

		if (_play_url["code"].asInt() != Response::OK) {
			spdlog::error("_play_url responses error, {}", _bvid);
			return;
		}

		_directed_link = _play_url["data"]["durl"][0]["url"].asString();

		spdlog::debug("{}'s _directed_link: {}", _bvid, _directed_link);
	}

	size_t VideoTask::progress_bar_index() const {
		return _progress_bar_index;
	}
	
	bool VideoTask::run() {
		if (_directed_link.empty()) {
			spdlog::warn("{}'s _directed_link is empty");
			return false;
		}

		_request.set_url(_directed_link);
		_request.set_referer(Constants::referer);
		_request.set_user_agent(Constants::user_agent);
		_request.set_noprogress(false);
		_request.set_progress_data(&_progress_bar_index);

		return _request.save_to_file(std::format("{}-from-bilibili-dl.mp4", _bvid));
	}
}
