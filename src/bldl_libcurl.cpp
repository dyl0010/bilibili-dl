#include "bldl_libcurl.h"
#include "bldl_helpers.h"
#include "bldl_manager.h"

#include <cassert>

#include <spdlog/spdlog.h>

#pragma warning(disable:4996)
#include "indicators/indicators.hpp"

namespace bldl {
	Request::Request() : _handle(curl_easy_init()) {
		assert(_handle);
		//spdlog::info("_handle: {}", _handle);
	}

	Request::~Request() {
		assert(_handle);
		curl_easy_cleanup(_handle);
	}

	CURLcode Request::set_url(const std::string& url) {
		return curl_easy_setopt(_handle, CURLOPT_URL, url.c_str());
	}

	CURLcode Request::set_noprogress(bool onoff) const {
		return curl_easy_setopt(_handle, CURLOPT_NOPROGRESS, onoff);
	}

	CURLcode Request::set_progress_data(void* progress_data) const {
		return curl_easy_setopt(_handle, CURLOPT_PROGRESSDATA, progress_data);
	}

	//CURLcode Request::set_progress_function(ProgressFunction progress_func) const {
	//	return curl_easy_setopt(_handle, CURLOPT_PROGRESSFUNCTION, progress_func);
	//}

	CURLcode Request::set_referer(const std::string referer) const {
		return curl_easy_setopt(_handle, CURLOPT_REFERER, referer.c_str());
	}

	CURLcode Request::set_user_agent(const std::string user_agent) const {
		return curl_easy_setopt(_handle, CURLOPT_USERAGENT, user_agent.c_str());
	}

	CURLcode Request::set_accept_encoding(const char* enc) const {
		return curl_easy_setopt(_handle, CURLOPT_ACCEPT_ENCODING, enc);
	}

	void Request::save_to_string(std::string& str) const {
		assert(_handle);

		_libcurl_write_data.clear();

		curl_easy_setopt(_handle, CURLOPT_WRITEFUNCTION, libcurl_write_callback);
		curl_easy_setopt(_handle, CURLOPT_WRITEDATA, &_libcurl_write_data);
		perform();

		str = _libcurl_write_data;
	}

	bool Request::save_to_file(const std::string& filename) {
		if (fopen_s(&_file, filename.c_str(), "wb"); !_file) {
			spdlog::error("could not open file {} for writing: {}", filename, errno);
			return false;
		}

		assert(_handle);
		
		curl_easy_setopt(_handle, CURLOPT_WRITEFUNCTION, 0);
		curl_easy_setopt(_handle, CURLOPT_WRITEDATA, _file);
		curl_easy_setopt(_handle, CURLOPT_PROGRESSFUNCTION, porgress_callback);
		perform();

		fclose(_file);

		return true;
	}

	CURLcode Request::perform() const {
		return curl_easy_perform(_handle);
	}

	int Request::libcurl_write_callback(char* data, size_t size, size_t nmemb, std::string* writerData) {
		if (!writerData)
			return 0;

		writerData->append(data, size * nmemb);

		return size * nmemb;
	}

	int Request::porgress_callback(void* clientp, double dltotal, double dlnow, [[maybe_unused]] double ultotal, [[maybe_unused]] double ulnow) {
		// TODO move implention to task.

		auto bar_index = static_cast<size_t*>(clientp);

		if (!bar_index) {
			spdlog::info("libcurl progress_callback() called with null(clientp)");
			return CURLE_BAD_FUNCTION_ARGUMENT;
		}

		indicators::show_console_cursor(false);

		auto progress = (dltotal == 0) ? 0 : (dlnow / dltotal);
		//spdlog::info("dltotal: {}, dlnow: {}, progress: {}%", dltotal, dlnow, progress * 100);
		Manager::get_instance().progress_bar_mgr()[*bar_index].set_progress(static_cast<float>(progress) * 100);

		if (Manager::get_instance().progress_bar_mgr()[*bar_index].is_completed()) {
			Manager::get_instance().progress_bar_mgr()[*bar_index].mark_as_completed();
		}

		indicators::show_console_cursor(true);
		return 0;
	}
}