#include "bldl_audio_task.h"
#include "bldl_helpers.h"
#include "bldl_constants.h"

#include <fstream>

#include <spdlog/spdlog.h>

namespace bldl {
	AudioTask::AudioTask(const std::string& general_url) : Task(general_url) {
		
		spdlog::debug("current _general_url: {}", _general_url);

		assert(!general_url.empty());

		_auid = extract_auid(general_url);

		_progress_bar->set_option(indicators::option::PrefixText(std::format("{}'s progress ", _auid)));

		//
		// get audio play url.
		auto url_req = add_query(Constants::audio_service_web_url, { "sid", _auid });
	
		_request.set_url(url_req);
		_request.set_accept_encoding("gzip");

		std::string reply;

		if (_request.save_to_string(reply); !Json::Reader{}.parse(reply, _play_url, false)) {
			spdlog::error("_play_url parsed failed, {}", _auid);
			_play_url.clear();
			return;
		}

		//spdlog::debug(_play_url.toStyledString());

		if (_play_url["code"].asInt() != Response::OK) {
			spdlog::error("_play_url responses error, {}", _auid);
			return;
		}

		_directed_link = _play_url["data"]["cdns"][0].asString();

		spdlog::debug("{}'s _directed_link: {}", _auid, _directed_link);
	}

	// TODO move run() function to base.
	bool AudioTask::run() {
		if (_directed_link.empty()) {
			spdlog::warn("{}'s _directed_link is empty");
			return false;
		}

		_request.set_url(_directed_link);
		_request.set_referer(Constants::referer);
		_request.set_user_agent(Constants::user_agent);
		_request.set_noprogress(false);
		_request.set_progress_data(&_progress_bar_index);

		return _request.save_to_file(std::format("{}-from-bilibili-dl.mp3", _auid));
	}

}