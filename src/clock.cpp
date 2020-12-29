//
// Created by Besogonov Aleksei on 12/28/20.
//
#include <chrono>
#include <cstdio>

#include "clock.h"
#include "log_sink.h"

using namespace llog;

const llog::clock_t the_clock;
const llog::clock_t *llog::system_clock = &the_clock;

void llog::print_timestamp(const timespec &t, sink_t &sink) {
	char buff[100];
	size_t ln = strftime(buff, sizeof(buff), "%FT%T", gmtime(&t.tv_sec));
	ln += snprintf(buff+ln, sizeof(buff)-ln, ".%ld", t.tv_nsec/1000);

	sink.write(buff, ln);
}
