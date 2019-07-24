/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include <limits.h>
#include <stdbool.h>
#include "temporal_value.h"
#include "temporal_utils.h"
#include "../util/rmalloc.h"

const char utcFormat[] = "YYYY-MM-DDTHH:MM:SS.NNNNNNNNNz";
const int dateMask = DATE | DATE_TIME | LOCAL_DATE_TIME;
const int timeMask = TIME | LOCAL_TIME | DATE_TIME | LOCAL_DATE_TIME;

/**
 * @brief  auxilary method to generate a tm struct from temporal value
 * @param  temporalValue:
 * @retval tm struct time descriptor
 */
struct tm *_getTimeDescriptorFromTemporalValue(RG_TemporalValue temporalValue) {
	struct timespec ts;
	ts.tv_sec = temporalValue.seconds;
	ts.tv_nsec = temporalValue.nano;
	return gmtime(&ts.tv_sec);
}
/**
 * @brief  auxilary method to generate a timespec struct from tm struct
 * @param  t: tm struct
 * @retval timespec, with nano value = 0
 */
struct timespec _createTimespecFromTm(struct tm t) {
	time_t epoch = timegm(&t);
	struct timespec ts;
	ts.tv_sec = epoch;
	ts.tv_nsec = 0;
	return ts;
}

RG_TemporalValue RG_TemporalValue_New_FromTimeSpec(RG_TemporalType temporalType,
												   struct timespec ts) {
	RG_TemporalValue temporalValue;
	temporalValue.seconds = ts.tv_sec;
	temporalValue.nano = ts.tv_nsec;
	temporalValue.type = temporalType;
	return temporalValue;
}

RG_TemporalValue RG_TemporalValue_New(RG_TemporalType temporalType) {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return RG_TemporalValue_New_FromTimeSpec(temporalType, ts);

}

RG_TemporalValue RG_Date_New() {
	return RG_TemporalValue_New(DATE);
}

RG_TemporalValue RG_Time_New() {
	return RG_TemporalValue_New(TIME);
}

RG_TemporalValue RG_LocalTime_New() {
	return RG_TemporalValue_New(LOCAL_TIME);
}

RG_TemporalValue RG_DateTime_New() {
	return RG_TemporalValue_New(DATE_TIME);
}

RG_TemporalValue RG_LocalDateTime_New() {
	return RG_TemporalValue_New(LOCAL_DATE_TIME);
}

RG_TemporalValue RG_Date_New_FromTimeSpec(struct timespec ts) {
	return RG_TemporalValue_New_FromTimeSpec(DATE, ts);
}

RG_TemporalValue RG_Time_New_FromTimeSpec(struct timespec ts) {
	return RG_TemporalValue_New_FromTimeSpec(TIME, ts);
}

RG_TemporalValue RG_LocalTime_New_FromTimeSpec(struct timespec ts) {
	return RG_TemporalValue_New_FromTimeSpec(LOCAL_TIME, ts);
}

RG_TemporalValue RG_DateTime_New_FromTimeSpec(struct timespec ts) {
	return RG_TemporalValue_New_FromTimeSpec(DATE_TIME, ts);
}

RG_TemporalValue RG_LocalDateTime_New_FromTimeSpec(struct timespec ts) {
	return RG_TemporalValue_New_FromTimeSpec(LOCAL_DATE_TIME, ts);
}

// TODO: implement
RG_TemporalValue RG_Date_New_FromString(const char *str) {
	struct tm *t = getdate(str);
	time_t time = timegm(t);
	struct timespec ts;
	ts.tv_sec = time;
	return RG_Date_New_FromTimeSpec(ts);
}

RG_TemporalValue RG_Time_New_FromString(const char *str) {
	struct tm *t = getdate(str);
	time_t time = timegm(t);
	struct timespec ts;
	ts.tv_sec = time;
	return RG_Time_New_FromTimeSpec(ts);
}

RG_TemporalValue RG_LocalTime_New_FromString(const char *str) {
	struct tm *t = getdate(str);
	time_t time = timegm(t);
	struct timespec ts;
	ts.tv_sec = time;
	return RG_LocalDateTime_New_FromTimeSpec(ts);
}

RG_TemporalValue RG_DateTime_New_FromString(const char *str) {
	struct tm *t = getdate(str);
	time_t time = timegm(t);
	struct timespec ts;
	ts.tv_sec = time;
	return RG_DateTime_New_FromTimeSpec(ts);
}

RG_TemporalValue RG_LocalDateTime_New_FromString(const char *str) {
	struct tm *t = getdate(str);
	time_t time = timegm(t);
	struct timespec ts;
	ts.tv_sec = time;
	return RG_LocalDateTime_New_FromTimeSpec(ts);
}

/**
 * @brief  return the right week in a year according to ISO 8601. taken from https://stackoverflow.com/questions/42568215/iso-8601-week-number-in-c
 * @note
 * @param  *timeDescriptor: a tm struct
 * @param  *year: out-by-pointer year value
 * @param  *week: out-by-pointer week value
 * @retval return 1 on failure, 0 on success
 */
int _timeDescriptor_YearWeek(const struct tm *timeDescriptor, int64_t *year, int64_t *week) {
	// work with local copy
	struct tm tm = *timeDescriptor;
	// fully populate the yday and wday fields.
	if(mktime(&tm) == -1) {
		return 1;
	}

	// Find day-of-the-week: 0 to 6.
	// Week starts on Monday per ISO 8601
	// 0 <= DayOfTheWeek <= 6, Monday, Tuesday ... Sunday
	int DayOfTheWeek = (tm.tm_wday + (7 - 1)) % 7;

	// Offset the month day to the Monday of the week.
	tm.tm_mday -= DayOfTheWeek;
	// Offset the month day to the mid-week (Thursday) of the week, 3 days later.
	tm.tm_mday += 3;
	// Re-evaluate tm_year and tm_yday  (local time)
	if(mktime(&tm) == -1) {
		return 1;
	}

	*year = tm.tm_year + 1900;
	// Convert yday to week of the year, stating with 1.
	*week = tm.tm_yday / 7 + 1;
	return 0;
}

int64_t RG_TemporalValue_GetYear(RG_TemporalValue temporalValue) {
	if(!(temporalValue.type & dateMask)) {
		// not a date type. should return error/execption
		return INT64_MIN;
	}
	struct tm *timeDescriptor  = _getTimeDescriptorFromTemporalValue(temporalValue);
	return timeDescriptor->tm_year + 1900;
}

int64_t RG_TemporalValue_GetQuarter(RG_TemporalValue temporalValue) {
	if(!(temporalValue.type & dateMask)) {
		// not a date type. should return error/execption
		return INT64_MIN;
	}
	struct tm *timeDescriptor  = _getTimeDescriptorFromTemporalValue(temporalValue);
	int month = timeDescriptor->tm_mon;
	return (month / 3) + 1;
}

int64_t RG_TemporalValue_GetMonth(RG_TemporalValue temporalValue) {
	if(!(temporalValue.type & dateMask)) {
		// not a date type. should return error/execption
		return INT64_MIN;
	}
	struct tm *timeDescriptor  = _getTimeDescriptorFromTemporalValue(temporalValue);
	return timeDescriptor->tm_mon + 1;
}

int64_t RG_TemporalValue_GetWeek(RG_TemporalValue temporalValue) {
	if(!(temporalValue.type & dateMask)) {
		// not a date type. should return error/execption
		return INT64_MIN;
	}
	struct tm *timeDescriptor  = _getTimeDescriptorFromTemporalValue(temporalValue);

	int64_t y = 0, w = 0;
	int err = _timeDescriptor_YearWeek(timeDescriptor, &y, &w);
	return w;
}

int64_t RG_TemporalValue_GetWeekYear(RG_TemporalValue temporalValue) {
	if(!(temporalValue.type & dateMask)) {
		// not a date type. should return error/execption
		return INT64_MIN;
	}
	struct tm *timeDescriptor  = _getTimeDescriptorFromTemporalValue(temporalValue);

	int64_t y = 0, w = 0;
	int err = _timeDescriptor_YearWeek(timeDescriptor, &y, &w);
	return y;
}

int64_t RG_TemporalValue_GetDayOfQuarter(RG_TemporalValue temporalValue) {
	if(!(temporalValue.type & dateMask)) {
		// not a date type. should return error/execption
		return INT64_MIN;
	}
	struct tm *timeDescriptor  = _getTimeDescriptorFromTemporalValue(temporalValue);

	int q = timeDescriptor->tm_mon / 4;
	if(isLeapYear(timeDescriptor->tm_year))
		return timeDescriptor->tm_yday - leapYearQuarterDaysAgg[q] + 1;
	else
		return timeDescriptor->tm_yday - normalYearQuarterDaysAgg[q] + 1;
}

int64_t RG_TemporalValue_GetDay(RG_TemporalValue temporalValue) {
	if(!(temporalValue.type & dateMask)) {
		// not a date type. should return error/execption
		return INT64_MIN;
	}
	struct tm *timeDescriptor  = _getTimeDescriptorFromTemporalValue(temporalValue);

	return timeDescriptor->tm_mday;
}

int64_t RG_TemporalValue_GetOrdinalDay(RG_TemporalValue temporalValue) {
	if(!(temporalValue.type & dateMask)) {
		// not a date type. should return error/execption
		return INT64_MIN;
	}
	struct tm *timeDescriptor  = _getTimeDescriptorFromTemporalValue(temporalValue);

	return timeDescriptor->tm_yday + 1;
}

int64_t RG_TemporalValue_GetDayOfWeek(RG_TemporalValue temporalValue) {
	if(!(temporalValue.type & dateMask)) {
		// not a date type. should return error/execption
		return INT64_MIN;
	}
	struct tm *timeDescriptor  = _getTimeDescriptorFromTemporalValue(temporalValue);
	// tm first day is Sunday, Cypher first day is Monday
	return (timeDescriptor->tm_wday + 6) % 7 + 1;
}

int64_t RG_TemporalValue_GetHour(RG_TemporalValue temporalValue) {
	if(!(temporalValue.type & timeMask)) {
		// not a time type. should return error/execption
		return INT64_MIN;
	}
	struct tm *timeDescriptor  = _getTimeDescriptorFromTemporalValue(temporalValue);

	return timeDescriptor->tm_hour;
}

int64_t RG_TemporalValue_GetMinute(RG_TemporalValue temporalValue) {
	if(!(temporalValue.type & timeMask)) {
		// not a time type. should return error/execption
		return INT64_MIN;
	}
	struct tm *timeDescriptor  = _getTimeDescriptorFromTemporalValue(temporalValue);

	return timeDescriptor->tm_min;
}

int64_t RG_TemporalValue_GetSecond(RG_TemporalValue temporalValue) {
	if(!(temporalValue.type & timeMask)) {
		// not a time type. should return error/execption
		return INT64_MIN;
	}
	struct tm *timeDescriptor  = _getTimeDescriptorFromTemporalValue(temporalValue);

	return timeDescriptor->tm_sec;
}

int64_t RG_TemporalValue_GetMillisecond(RG_TemporalValue temporalValue) {
	if(!(temporalValue.type & timeMask)) {
		// not a time type. should return error/execption
		return INT64_MIN;
	}

	return temporalValue.nano / 1000000;
}

int64_t RG_TemporalValue_GetMicrosecond(RG_TemporalValue temporalValue) {
	if(!(temporalValue.type & timeMask)) {
		// not a time type. should return error/execption
		return INT64_MIN;
	}

	return temporalValue.nano / 1000;
}

int64_t RG_TemporalValue_GetNanosecond(RG_TemporalValue temporalValue) {
	if(!(temporalValue.type & timeMask)) {
		// not a time type. should return error/execption
		return INT64_MIN;
	}

	return temporalValue.nano;
}


const char *RG_TemporalValue_ToString(RG_TemporalValue timestamp) {
	struct tm *time = gmtime(&timestamp.seconds);
	size_t len = strlen(utcFormat) + 1;
	char *buffer = rm_malloc(len);
	len -= strftime(buffer, len, "%Y-%m-%dT%H:%M:%S.", time);
	snprintf(&buffer[strlen(buffer)], len, ".%09uiz", timestamp.nano);
	return buffer;
}

int RG_TemporalValue_Compare(RG_TemporalValue a, RG_TemporalValue b) {
	return a.seconds == b.seconds ? a.nano - b.nano : a.seconds - b.seconds;
}
