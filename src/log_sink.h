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
	const size_t DEFAULT_BUF_SIZE = 15000;

	class sink_t
	{
		std::mutex lock_;
	public:
		virtual ~sink_t() = default;

		void lock() {
			lock_.lock();
		}
		void unlock() {
			lock_.unlock();
		}

		virtual void write(const char *data, size_t sz) = 0;
	};

	class mem_sink_t : public sink_t {
		std::vector<char> content_;
	public:
		mem_sink_t() = default;
		explicit mem_sink_t(size_t cap) {content_.reserve(cap);}

		const std::vector<char>& data() const { return content_; }

	protected:
		void write(const char *data, size_t sz) override {
			content_.insert(content_.end(), data, data + sz);
		}
	};

	class stream_sink_t : public sink_t {
		std::ostream &stream_;
	public:
		explicit stream_sink_t(std::ostream &stream) : stream_(stream) {}
	protected:
		void write(const char *data, size_t sz) override;
	};

	class sink_writer_t {
		char *buffer_;
		size_t cur_pos_ = 0;
		size_t buf_size_;
		std::shared_ptr<sink_t> sink_;

		bool in_entry_ = false, locked_ = false;
	public:
		sink_writer_t(const std::shared_ptr<sink_t> &sink,
								size_t buf_size = DEFAULT_BUF_SIZE) : sink_(sink), buf_size_(buf_size) {
			buffer_ = static_cast<char *>(malloc(buf_size));
		}
		~sink_writer_t() {
			free(buffer_);
		}

		void begin_entry() {
			assert(!in_entry_);
			assert(!locked_);
			in_entry_ = true;
		}

		void end_entry() {
			assert(in_entry_);
			if (cur_pos_ != 0) {
				flush_buffer();
			}
			if (locked_) {
				sink_->unlock();
			}
			locked_ = in_entry_ = false;
		}

		inline void write(const char *data, size_t sz) {
			if (sz >= buf_size_/2) {
				flush_buffer();
				sink_->write(data, sz);
				return;
			}

			if (buf_size_ - cur_pos_ <= sz) {
				flush_buffer();
			}
			memcpy(buffer_ + cur_pos_, data, sz);
			cur_pos_ += sz;
		}

		inline void flush_buffer() {
			if (!locked_) {
				sink_->lock();
				locked_ = true;
			}

			if (cur_pos_ != 0) {
				sink_->write(buffer_, cur_pos_);
				cur_pos_ = 0;
			}
		}
	};

	inline sink_writer_t& operator << (sink_writer_t &writer, int64_t val);

	class entry_sentry_t {
		sink_writer_t &sink_;
	public:
		explicit entry_sentry_t(sink_writer_t &sink) : sink_(sink) {
			sink_.begin_entry();
		}
		~entry_sentry_t() {
			sink_.end_entry();
		}
	};

	extern std::shared_ptr<sink_t> cout_sink;
	extern std::shared_ptr<sink_t> cerr_sink;
}
