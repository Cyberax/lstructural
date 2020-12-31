//
// Created by Besogonov Aleksei on 12/26/20.
//

#include "log_sink.h"
#include <memory>
#include <iostream>

using namespace llog;

std::shared_ptr<sink_t> llog::cout_sink = std::make_shared<stream_sink_t>(std::cout);
std::shared_ptr<sink_t> llog::cerr_sink = std::make_shared<stream_sink_t>(std::cerr);

void stream_sink_t::write(const char *data, size_t sz) {
	check_locked();
	stream_.write(data, sz);
}
