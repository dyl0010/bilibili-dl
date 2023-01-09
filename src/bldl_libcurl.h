#pragma once

#include <string>

#include <curl/curl.h>

namespace bldl {
	enum Response {
		OK,
	};

	class Request {
	public:
		explicit Request();
		~Request();

		typedef int (*ProgressFunction)(void*, double, double, double, double);
		
		CURLcode set_url(const std::string& url);
		CURLcode set_noprogress(bool onoff) const;
		CURLcode set_progress_data(void* progress_data) const;
		//CURLcode set_progress_function(ProgressFunction progress_func) const;
		CURLcode set_referer(const std::string referer) const;
		CURLcode set_user_agent(const std::string user_agent) const;

		void save_to_string(std::string &str) const;
		bool save_to_file(const std::string& filename);

		static int libcurl_write_callback(char* data, size_t size, size_t nmemb, std::string* writerData);
		static int porgress_callback(void* clientp, double dltotal, double dlnow, double ultotal, double ulnow);

		inline static std::string _libcurl_write_data;

	private:
		CURLcode perform() const;

	private:
		CURL* _handle = 0;
		FILE* _file = 0;
	};
}