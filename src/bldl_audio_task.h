#pragma once

#include "bldl_task.h"
#include "bldl_libcurl.h"

#pragma warning(disable:4996)
#include "indicators/indicators.hpp"

#include <json/json.h>

namespace bldl {
	class AudioTask : public Task {
	public:
		AudioTask(const std::string& general_url);

		bool run() override;
	private:
		std::string _auid;
		Json::Value _play_url;
		Request _request;
		std::string _directed_link;
	};
}