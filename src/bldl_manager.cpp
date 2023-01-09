#include <algorithm>
#include <execution>

#include <spdlog/spdlog.h>

#include "bldl_manager.h"
#include "bldl_video_task.h"
#include "bldl_helpers.h"

namespace bldl {
	Manager::Manager() {
		_progress_bar_mgr.set_option(indicators::option::HideBarWhenComplete{ false });
	}

	void Manager::parsed_address_file(const std::string &path) {
		std::set<std::string> adres;
		
		read_lines(path, adres);

		std::ranges::for_each(adres, [](auto const& url) {
			bldl::Manager::get_instance().add_task(std::make_shared<bldl::VideoTask>(url));
		 });
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
