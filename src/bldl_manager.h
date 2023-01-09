#pragma once
#include <memory>
#include <list>

#include "bldl_singleton.h"

#pragma warning(disable:4996)
#include "indicators/indicators.hpp"

namespace bldl {
	class Task;

	class Manager : public Singleton<Manager> {
	public:
		void parsed_address_file(const std::string &);
		void add_task(std::shared_ptr<Task>);
		void run_tasks();

		using ProgressBarMgr = indicators::DynamicProgress<indicators::BlockProgressBar>;

		ProgressBarMgr& progress_bar_mgr();

	protected:
		Manager();

		ProgressBarMgr _progress_bar_mgr;
		std::vector<std::shared_ptr<Task>> _tasks;

		friend class Singleton<Manager>;
	};
}