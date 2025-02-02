#include "RTC.h"

RTCTime clock_agenda[7][2] = {
  // SUNDAY FIRST!
  {
    RTCTime(1, Month::JANUARY, 1970, 07, 15, 0, (DayOfWeek)0, SaveLight::SAVING_TIME_INACTIVE),
    RTCTime(1, Month::JANUARY, 1970, 19, 15, 0, (DayOfWeek)0, SaveLight::SAVING_TIME_INACTIVE),
  },
  {
    RTCTime(1, Month::JANUARY, 1970, 05, 45, 0, (DayOfWeek)1, SaveLight::SAVING_TIME_INACTIVE),
    RTCTime(1, Month::JANUARY, 1970, 19, 15, 0, (DayOfWeek)1, SaveLight::SAVING_TIME_INACTIVE),
  },
  {
    RTCTime(1, Month::JANUARY, 1970, 05, 46, 0, (DayOfWeek)2, SaveLight::SAVING_TIME_INACTIVE),
    RTCTime(1, Month::JANUARY, 1970, 19, 15, 0, (DayOfWeek)2, SaveLight::SAVING_TIME_INACTIVE),
  },
  {
    RTCTime(1, Month::JANUARY, 1970, 05, 47, 0, (DayOfWeek)3, SaveLight::SAVING_TIME_INACTIVE),
    RTCTime(1, Month::JANUARY, 1970, 19, 15, 0, (DayOfWeek)3, SaveLight::SAVING_TIME_INACTIVE),
  },
  {
    RTCTime(1, Month::JANUARY, 1970, 05, 48, 0, (DayOfWeek)4, SaveLight::SAVING_TIME_INACTIVE),
    RTCTime(1, Month::JANUARY, 1970, 19, 15, 0, (DayOfWeek)4, SaveLight::SAVING_TIME_INACTIVE),
  },
  {
    RTCTime(1, Month::JANUARY, 1970, 05, 49, 0, (DayOfWeek)5, SaveLight::SAVING_TIME_INACTIVE),
    RTCTime(1, Month::JANUARY, 1970, 19, 15, 0, (DayOfWeek)5, SaveLight::SAVING_TIME_INACTIVE),
  },
  {
    RTCTime(1, Month::JANUARY, 1970, 06, 45, 0, (DayOfWeek)6, SaveLight::SAVING_TIME_INACTIVE),
    RTCTime(1, Month::JANUARY, 1970, 19, 15, 0, (DayOfWeek)6, SaveLight::SAVING_TIME_INACTIVE),
  },
};
