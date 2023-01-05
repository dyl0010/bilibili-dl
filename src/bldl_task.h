#pragma once

#include <string>

namespace bldl {
	class Task {
	public:
		Task(const std::string general_url);

	protected:
		std::string _general_url;
	};
}