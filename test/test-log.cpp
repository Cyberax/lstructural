//
// Created by Besogonov Aleksei on 12/25/20.
//

#include "logger.h"
#include <gtest/gtest.h>
#include <fmt/format.h>
#include "log_sink.h"

using namespace llog;

TEST(AttrsTest, time) {
	logger_cfg_t cfg{};
	cfg.sink = std::make_shared<llog::mem_sink_t>(1024*1024*1024*5);
	logger_t tp(cfg, {});

//	std::string_view v("hello");
//	fmt::print("Hello {name}", fmt::arg("name", "World"));
	std::cerr << "START" << std::endl;
	auto start = std::chrono::system_clock::now();

	for(uint64_t f = 0; f < 5*1000*1000; ++f) {
		tp.log(llog::severity_t::DEBUG, "hello {world} {world2}", {
			{"world",  1},
			{"world2", "2"},
		});

		//tp.log(llog::severity_t::DEBUG, "hello {world} {world2}", {});
	}

	auto val = std::chrono::duration_cast<std::chrono::milliseconds>(
		(std::chrono::system_clock::now() - start)).count();
	std::cerr << "TIME " << val << std::endl;
}
