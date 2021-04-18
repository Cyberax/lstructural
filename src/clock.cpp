/*
	MIT License

	Copyright (c) 2013-2017 Evgeny Safronov <division494@gmail.com>
	Copyright (c) 2013-2017 Other contributors as noted in the AUTHORS file.

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
 */
#include <cstdio>

#include "clock.h"
#include "log_sink.h"

using namespace llog;

const llog::clock_t the_clock;
const llog::clock_t *llog::system_clock = &the_clock;

static auto fast_gmtime(time_t t, struct tm* tp) noexcept -> struct tm* {
	int yday;
	uintptr_t n, sec, min, hour, mday, mon, year, wday, days, leap;

	// The calculation is valid for positive time_t only.
	n = t;
	days = n / 86400;

	// Jaunary 1, 1970 was Thursday
	wday = (4 + days) % 7;
	n %= 86400;
	hour = n / 3600;
	n %= 3600;
	min = n / 60;
	sec = n % 60;

	// The algorithm based on Gauss's formula, see src/http/ngx_http_parse_time.c.
	// Days since March 1, 1 BC.
	days = days - (31 + 28) + 719527;

	// The "days" should be adjusted to 1 only, however, some March 1st's go to previous year, so
	// we adjust them to 2.  This causes also shift of the last Feburary days to next year, but we
	// catch the case when "yday" becomes negative.
	year = (days + 2) * 400 / (365 * 400 + 100 - 4 + 1);
	yday = static_cast<int>(days - (365 * year + year / 4 - year / 100 + year / 400));

	leap = (year % 4 == 0) && (year % 100 || (year % 400 == 0));

	if (yday < 0) {
		yday = static_cast<int>(365 + leap + static_cast<unsigned long>(yday));
		year--;
	}

	// The empirical formula that maps "yday" to month. There are at least 10 variants, some of
	// them are:
	// mon = (yday + 31) * 15 / 459
	// mon = (yday + 31) * 17 / 520
	// mon = (yday + 31) * 20 / 612
	mon = static_cast<uintptr_t>((yday + 31) * 10 / 306);

	// The Gauss's formula that evaluates days before the month.
	mday = static_cast<unsigned long>(yday)- (367 * mon / 12 - 30) + 1;

	if (yday >= 306) {
		year++;
		mon -= 10;
		yday -= 306;
	} else {
		mon += 2;
		yday += 31 + 28 + static_cast<int>(leap);
	}

	tp->tm_sec = static_cast<int>(sec);
	tp->tm_min = static_cast<int>(min);
	tp->tm_hour = static_cast<int>(hour);
	tp->tm_mday = static_cast<int>(mday);
	tp->tm_mon = static_cast<int>(mon - 1);
	tp->tm_year = static_cast<int>(year - 1900);
	tp->tm_yday = yday;
	tp->tm_wday = static_cast<int>(wday);
	tp->tm_isdst = 0;

	return tp;
}

class tzinit_t {
public:
	tzinit_t() { tzset(); }
};

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"

static const tzinit_t tz;

#pragma clang diagnostic pop

static auto localtime(time_t t, struct tm* tp) noexcept -> struct tm* {
	time_t time = t - timezone;
	gmtime_r(&time, tp);
	tp->tm_gmtoff = timezone;
	tp->tm_zone = *tzname;

	return tp;
}

void llog::print_timestamp(const timespec &t, sink_writer_t &sink) {
	char buff[100];
	tm gmt = {};
	fast_gmtime(time_t(t.tv_sec), &gmt);

//	size_t ln = snprintf(buff, sizeof(buff),"%4d-%2d-%2dT%2d:%2d:%2d.%ldZ",
//											gmt.tm_year, gmt.tm_mon, gmt.tm_mday, gmt.tm_hour, gmt.tm_min, gmt.tm_sec,
//											t.tv_nsec/1000);

//	sink.write(buff, ln);
}
