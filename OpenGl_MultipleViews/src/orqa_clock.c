
#include "orqa_clock.h"

#include <time.h> // clock_gettime
#include <unistd.h> // usleep

//=========================================================

void orqa_sleep(
  ORQA_IN OrqaSleepType const unit_type,
  ORQA_IN unsigned int const amount)
{
  switch(unit_type)
  {
    case ORQA_SLEEP_NSEC:
      usleep(ORQA_NS_TO_US(amount));
      return;
    case ORQA_SLEEP_USEC:
      usleep(amount);
      return;
    case ORQA_SLEEP_MSEC:
      usleep(ORQA_MS_TO_US(amount));
      return;
    case ORQA_SLEEP_SEC:
      usleep(ORQA_S_TO_US(amount));
      return;

    default:
      return;
  }
}

//=========================================================

orqa_clock_t orqa_time_now(
  ORQA_NOARGS void)
{
  orqa_clock_t start_time;

  if (clock_gettime(CLOCK_REALTIME, &start_time) < 0)
  {
    // oprint_ea("[comm EE] [%s:%d] Error: clock_gettime failed\n", __FILE__, __LINE__);
    // operror("[comm EE] perror");
  }

  return start_time;
}

//=========================================================

double orqa_get_time_diff_nsec(
  ORQA_IN orqa_clock_t const start,
  ORQA_IN orqa_clock_t const stop)
{
  return ORQA_S_TO_NS(stop.tv_sec - start.tv_sec) + (stop.tv_nsec - start.tv_nsec);
}

double orqa_get_time_diff_usec(
  ORQA_IN orqa_clock_t const start,
  ORQA_IN orqa_clock_t const stop)
{
  return ((stop.tv_sec - start.tv_sec) * (time_t)(1e6)) + ORQA_NS_TO_US(stop.tv_nsec - start.tv_nsec);
}

double orqa_get_time_diff_msec(
  ORQA_IN orqa_clock_t const start,
  ORQA_IN orqa_clock_t const stop)
{
  return ((stop.tv_sec - start.tv_sec) * (time_t)(1e3)) + ORQA_NS_TO_MS(stop.tv_nsec - start.tv_nsec);
}

double orqa_get_time_diff_sec(
  ORQA_IN orqa_clock_t start,
  ORQA_IN orqa_clock_t stop)
{
  return (stop.tv_nsec - start.tv_sec) + ORQA_NS_TO_S(stop.tv_nsec - start.tv_nsec);
}

//=========================================================