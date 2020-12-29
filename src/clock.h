//
// Created by Besogonov Aleksei on 12/28/20.
//
#pragma once

#include <cstdint>
#include <ctime>

namespace llog {
	class sink_t;

	void print_timestamp(const timespec &t, sink_t &sink);

	class clock_t
	{
	public:
		virtual ~clock_t() = default;

		virtual timespec now() const {
			struct timespec tp = {};
			clock_gettime(CLOCK_REALTIME, &tp);
			return tp;
		};
	};

	class fixed_clock_t : public clock_t {
	public:
		timespec time_;

		timespec now() const override { return time_; }
	};

	extern const clock_t *system_clock;
}
