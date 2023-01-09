#pragma once

#include <string>

#pragma warning(disable:4996)
#include "indicators/indicators.hpp"

namespace bldl {
	class Task {
	public:
		Task(const std::string general_url);
		
		void set_progress_bar_index(size_t index);
		std::shared_ptr<indicators::BlockProgressBar> &progress_bar();

		virtual bool run() { return false; };
	protected:

		std::string _general_url;
		size_t _progress_bar_index = 0;
		std::shared_ptr<indicators::BlockProgressBar> _progress_bar;
	};
}