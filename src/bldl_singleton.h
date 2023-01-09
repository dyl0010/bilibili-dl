#pragma once

// Singleton - CRTP
template<typename Deriver>
class Singleton {
public: 
	static Deriver& get_instance() {
		static Deriver _instance;
		return _instance;
	}
protected:
	Singleton() = default;
	~Singleton() = default;
	Singleton(const Singleton&) = delete;
	Singleton(Singleton&&) noexcept = delete;
	Singleton& operator=(const Singleton&) = delete;
	Singleton& operator=(Singleton&&) noexcept = delete;
};