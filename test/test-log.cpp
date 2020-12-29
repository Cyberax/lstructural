//
// Created by Besogonov Aleksei on 12/25/20.
//

#include "logger.h"
#include <gtest/gtest.h>
#include <fmt/format.h>

using namespace llog;

TEST(AttrsTest, time) {
	logger_t tp(logger_cfg_t{}, {});

//	std::string_view v("hello");
//	fmt::print("Hello {name}", fmt::arg("name", "World"));

	tp.logf(llog::severity_t::DEBUG, "hello {world} {world2}",{
		{"world", 1},
		{"world2", "2"},
	});
}
