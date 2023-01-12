#pragma once
#include <memory>
#include <list>
#include <set>

#include "bldl_singleton.h"
#include "bldl_helpers.h"

#pragma warning(disable:4996)
#include "indicators/indicators.hpp"

namespace bldl {
	class Task;

	class Manager : public Singleton<Manager> {
	public:
		template<typename TaskType> 
		void parsed_address_file(const std::string& path);
		
		void add_task(std::shared_ptr<Task>);
		void run_tasks();

		using ProgressBarMgr = indicators::DynamicProgress<indicators::BlockProgressBar>;

		ProgressBarMgr& progress_bar_mgr();

	protected:
		Manager();

		indicators::ProgressSpinner _progress_spinner;
		ProgressBarMgr _progress_bar_mgr;
		std::vector<std::shared_ptr<Task>> _tasks;

		friend class Singleton<Manager>;
	};

	// TODO Be implemented as a non-member template function.
	template<typename TaskType>
	inline void Manager::parsed_address_file(const std::string& path)
	{
		std::set<std::string> adres;

		read_lines(path, adres);

		size_t finished = 0;

		_progress_spinner.set_option(indicators::option::MaxProgress(adres.size()));

		for (auto it = adres.begin(); it != adres.end(); ++it) {
			bldl::Manager::get_instance().add_task(std::make_shared<TaskType>(*it));

			_progress_spinner.set_option(indicators::option::PostfixText(std::format("{}/{}", ++finished, adres.size())));

			_progress_spinner.tick();

			if (_progress_spinner.is_completed()) {
				_progress_spinner.mark_as_completed();
				break;
			}
		}
	}
}