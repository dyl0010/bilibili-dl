#include "bldl_task.h"

namespace bldl {
	Task::Task(const std::string general_url) 
		: _general_url(general_url)
		, _progress_bar(std::make_shared<indicators::BlockProgressBar>()) {
		_progress_bar->set_option(indicators::option::BarWidth(70));
		_progress_bar->set_option(indicators::option::ForegroundColor(indicators::Color::green));
	}

	void Task::set_progress_bar_index(size_t index) {
		_progress_bar_index = index;
	}

	std::shared_ptr<indicators::BlockProgressBar> &Task::progress_bar() {
		return _progress_bar;
	}
}
