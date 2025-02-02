#include "RTC.h"

RTCTime clock_agenda[7][2] = {
  {
    // Sunday
    RTCTime(1, Month::JANUARY, 1970,  7, 15, 0, (DayOfWeek)0, SaveLight::SAVING_TIME_INACTIVE),
    RTCTime(1, Month::JANUARY, 1970, 19, 15, 0, (DayOfWeek)0, SaveLight::SAVING_TIME_INACTIVE),
  },
  {
    // Monday
    RTCTime(1, Month::JANUARY, 1970,  5, 45, 0, (DayOfWeek)1, SaveLight::SAVING_TIME_INACTIVE),
    RTCTime(1, Month::JANUARY, 1970, 19, 15, 0, (DayOfWeek)1, SaveLight::SAVING_TIME_INACTIVE),
  },
  {
    // Tuesday
    RTCTime(1, Month::JANUARY, 1970,  5, 45, 0, (DayOfWeek)2, SaveLight::SAVING_TIME_INACTIVE),
    RTCTime(1, Month::JANUARY, 1970, 19, 15, 0, (DayOfWeek)2, SaveLight::SAVING_TIME_INACTIVE),
  },
  {
    // Wednesday
    RTCTime(1, Month::JANUARY, 1970,  5, 45, 0, (DayOfWeek)3, SaveLight::SAVING_TIME_INACTIVE),
    RTCTime(1, Month::JANUARY, 1970, 19, 15, 0, (DayOfWeek)3, SaveLight::SAVING_TIME_INACTIVE),
  },
  {
    // Thursday
    RTCTime(1, Month::JANUARY, 1970,  5, 45, 0, (DayOfWeek)4, SaveLight::SAVING_TIME_INACTIVE),
    RTCTime(1, Month::JANUARY, 1970, 19, 15, 0, (DayOfWeek)4, SaveLight::SAVING_TIME_INACTIVE),
  },
  {
    // Friday
    RTCTime(1, Month::JANUARY, 1970,  5, 45, 0, (DayOfWeek)5, SaveLight::SAVING_TIME_INACTIVE),
    RTCTime(1, Month::JANUARY, 1970, 19, 15, 0, (DayOfWeek)5, SaveLight::SAVING_TIME_INACTIVE),
  },
  {
    // Saturday
    RTCTime(1, Month::JANUARY, 1970,  6, 45, 0, (DayOfWeek)6, SaveLight::SAVING_TIME_INACTIVE),
    RTCTime(1, Month::JANUARY, 1970, 19, 15, 0, (DayOfWeek)6, SaveLight::SAVING_TIME_INACTIVE),
  },
};
