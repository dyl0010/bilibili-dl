#include <algorithm>
#include <execution>

#include <spdlog/spdlog.h>

#include "bldl_manager.h"
#include "bldl_video_task.h"
#include "bldl_constants.h"

namespace bldl {
	Manager::Manager() {
		_progress_spinner.set_option(indicators::option::PrefixText{ Constants::progress_spinner_text });
		_progress_spinner.set_option(indicators::option::ForegroundColor{ indicators::Color::yellow });
		_progress_spinner.set_option(indicators::option::SpinnerStates{ std::vector<std::string>{"|", "/", "-", "\\",} });

		_progress_bar_mgr.set_option(indicators::option::HideBarWhenComplete{ false });
	}

	void Manager::add_task(std::shared_ptr<Task> task) {
		task->set_progress_bar_index(_progress_bar_mgr.push_back(*(task->progress_bar())));
		_tasks.push_back(task);
	}

	void Manager::run_tasks() {
		std::for_each(std::execution::par_unseq, _tasks.begin(), _tasks.end(), [](auto task) {task->run();});
	}

	Manager::ProgressBarMgr& Manager::progress_bar_mgr() {
		return _progress_bar_mgr;
	}
}
