//
// Created by Besogonov Aleksei on 12/26/20.
//
#pragma once

#include <memory>
#include <vector>
#include <mutex>
#include <iosfwd>
#include "clock.h"


namespace llog {

	class sink_t
	{
	public:
		virtual ~sink_t() = default;

		virtual void begin_entry() = 0;
		virtual void write(const char *data, size_t sz) = 0;
		virtual void end_entry() = 0;
	};

	class sink_lock_guard_t {
		sink_t &sink_;
	public:
		explicit sink_lock_guard_t(sink_t &sink) : sink_(sink) {
			sink_.begin_entry();
		}
		~sink_lock_guard_t() {
			sink_.end_entry();
		}
	};

	class locked_sink_t : public sink_t {
		std::mutex lock_;
		bool locked_ = false;
	protected:
		void check_locked() const {
			if (!locked_) {
				throw std::bad_exception();
			}
		}
	public:
		void begin_entry() override {
			lock_.lock();
			locked_ = true;
		}
		void end_entry() override {
			locked_ = false;
			lock_.unlock();
		}
	};

	class mem_sink_t : public locked_sink_t {
		std::vector<char> content_;
	public:
		void write(const char *data, size_t sz) override {
			check_locked();
			content_.insert(content_.end(), data, data + sz);
		}
	};

	class stream_sink_t : public locked_sink_t {
		std::ostream &stream_;
	public:
		explicit stream_sink_t(std::ostream &stream) : stream_(stream) {}
		void write(const char *data, size_t sz) override;
	};

	extern std::shared_ptr<sink_t> cout_sink;
}
