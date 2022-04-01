#ifndef ORQA_CLOCK_H
#define ORQA_CLOCK_H

#define ORQA_IN
#define ORQA_REF
#define ORQA_OUT
#define ORQA_NOARGS


#include <time.h>

//========================================================

typedef enum orqa_sleep_measure
{
  ORQA_SLEEP_NSEC,
  ORQA_SLEEP_USEC,
  ORQA_SLEEP_MSEC,
  ORQA_SLEEP_SEC

} OrqaSleepType;

// Makes the program sleep for the requested time.
// @param unit_type
//   A value from the OrqaSleepType enum. Denotes the
//   measure which is applied to the amount.
// @param amount
//   The amount to sleep for.
void orqa_sleep(
  ORQA_IN OrqaSleepType const unit_type,
  ORQA_IN unsigned int const amount);

//========================================================
//================= HD clock utilities ===================

typedef struct timespec orqa_clock_t;

#define ORQA_NS_TO_US(nsec) ((double)((nsec) / (double)(1e3)))
#define ORQA_NS_TO_MS(nsec) ((double)((nsec) / (double)(1e6)))
#define ORQA_NS_TO_S(nsec)  ((double)((nsec) / (double)(1e9)))

#define ORQA_US_TO_NS(usec) ((double)((usec) * (double)(1e3)))
#define ORQA_US_TO_MS(usec) ((double)((usec) / (double)(1e3)))
#define ORQA_US_TO_S(usec)  ((double)((usec) / (double)(1e6)))

#define ORQA_MS_TO_NS(msec) ((double)((msec) * (double)(1e6)))
#define ORQA_MS_TO_US(msec) ((double)((msec) * (double)(1e3)))
#define ORQA_MS_TO_S(msec)  ((double)((msec) / (double)(1e3)))

#define ORQA_S_TO_NS(sec)   ((double)((sec)  * (double)(1e9)))
#define ORQA_S_TO_US(sec)   ((double)((sec)  * (double)(1e6)))
#define ORQA_S_TO_MS(sec)   ((double)((sec)  * (double)(1e3)))

// Returns a high-precision clock containing current time.
extern orqa_clock_t orqa_time_now(
  ORQA_NOARGS void);

// Calculates the elapsed time between start and stop. Returns the elapsed time in nanoseconds.
// @param start
//   The point in time when measuring started.
// @param stop
//   The point in time when measuring stopped.
// @return
//   orqa_clock_t contaning the elapsed time.
extern double orqa_get_time_diff_nsec(
  ORQA_IN orqa_clock_t const start,
  ORQA_IN orqa_clock_t const stop);

// Calculates the elapsed time between start and stop. Returns the elapsed time in microseconds.
// @param start
//   The point in time when measuring started.
// @param stop
//   The point in time when measuring stopped.
// @return
//   orqa_clock_t contaning the elapsed time.
extern double orqa_get_time_diff_usec(
  ORQA_IN orqa_clock_t const start,
  ORQA_IN orqa_clock_t const stop);

// Calculates the elapsed time between start and stop. Returns the elapsed time in milliseconds.
// @param start
//   The point in time when measuring started.
// @param stop
//   The point in time when measuring stopped.
// @return
//   orqa_clock_t contaning the elapsed time.
extern double orqa_get_time_diff_msec(
  ORQA_IN orqa_clock_t const start,
  ORQA_IN orqa_clock_t const stop);

// Calculates the elapsed time between start and stop. Returns the elapsed time in seconds.
// @param start
//   The point in time when measuring started.
// @param stop
//   The point in time when measuring stopped.
// @return
//   orqa_clock_t contaning the elapsed time.
extern double orqa_get_time_diff_sec(
  ORQA_IN orqa_clock_t const start,
  ORQA_IN orqa_clock_t const stop);

//========================================================

#endif // ORQA_CLOCK_H