//
// Created by Besogonov Aleksei on 12/26/20.
//

#include "logger.h"
#include "log_sink.h"
#include <fmt/format.h>
#include <ostream>
#include <sstream>
#include <charconv>

using namespace llog;

// Convert a reference-based attribute value to stored attribute value
struct saving_visitor {
	stored_attr_val_t val_;

	void operator()(const std::string_view &s) {
		val_ = std::move(std::string(s));
	}

	template<class T> void operator()(const T& val) {
		val_ = val;
	}
};


logger_t::logger_t(logger_cfg_t &&cfg, attribute_pack_t &&attrs) : cfg_(std::move(cfg)) {
	for (auto &i : attrs) {
		saving_visitor sv;
		std::visit(sv, i.val_);
		attrs_.push_back(stored_attr_pair_t{
			.name_ = std::string(i.name_),
			.val_ = std::move(sv.val_),
		});
	}
}

void logger_t::log(severity_t lvl, const std::string_view &msg, attribute_pack_t &&attrs) {

}

struct arg_extracting_visitor {
	fmt::dynamic_format_arg_store<fmt::format_context> &args_;
	const char *name_;

	template<class T> void operator()(const T &val) {
		args_.push_back(fmt::arg(name_, val));
	}
};

struct push_formatter {
	sink_t &sink;
	void push_back(const char c) { sink.write(&c, 1); }
	typedef char value_type;
};

struct print_visitor {
	sink_t &sink_;

	template<class T> void operator ()(T val) {
		char buf[64];
		auto res = std::to_chars(buf, buf+sizeof(buf), val);
		if (!res.ptr) {
			throw std::bad_exception();
		}
		sink_.write(buf, res.ptr - buf);
	}
	void operator ()(double val) {
		char buf[64];
		auto ln = snprintf(buf, 64, "%f", val);
		sink_.write(buf, ln);
	}
	void operator ()(const std::string_view &val) {
		sink_.write(val.data(), val.size());
	}
	void operator ()(bool val) {
		if (val) {
			sink_.write("true", 4);
		} else {
			sink_.write("false", 5);
		}
	}
};


void logger_t::logf(severity_t lvl, const std::string_view &msg, attribute_pack_t &&attrs) {
	// Format the log message
	fmt::dynamic_format_arg_store<fmt::format_context> store;
	bool has_message_arg = false;
	for (auto &i : attrs) {
		// Check if the argument is a special placeholder for the message
		if (strcmp(i.name_, "msg") == 0) {
			has_message_arg = true;
			continue;
		}
		arg_extracting_visitor v{store, i.name_};
		std::visit(v, i.val_);
	}

	sink_t &sink = *(cfg_.sink);
	sink_lock_guard_t lg(sink);

	// Format the string!
	if (cfg_.use_json) {

	} else {
		// Plain text
		sink.write("[", 1);
		sink.write("] ", 1);

		if (cfg_.add_iso_timestamp) {
			print_timestamp(cfg_.clock->now(), sink);
		}
		if (!has_message_arg) {
			push_formatter pf{.sink = sink};
			fmt::vformat_to(std::back_inserter(pf), msg, store);
			sink.write(" ", 1);
		}
		// Iterate over attrs and print them!
		bool first = true;
		for (auto &i : attrs) {
			if (!first) {
				sink.write(" ", 1);
			}
			first = false;
			if (strcmp(i.name_, "msg") == 0) {
				// Msg get special treatment - it's replaced by the rendered message
				push_formatter pf{.sink = sink};
				fmt::vformat_to(std::back_inserter(pf), msg, store);
			} else {
				sink.write(i.name_, strlen(i.name_));
				sink.write("=", 1);
				print_visitor pv{.sink_ = sink};
				std::visit(pv, i.val_);
			}
		}
	}

//	virtual void push_back(const char c) { write(&c, 1); }
//	typedef char value_type;

//	fmt::vformat_to(std::back_inserter(*cfg_.sink), msg, store);
}
