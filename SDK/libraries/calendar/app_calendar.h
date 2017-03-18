/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/** @file
 *
 * @defgroup app_calendar Application Calendar
 * @{
 * @ingroup app_common
 *
 * @brief Application timer functionality.
 *
 * @details This module offer calendar feature based on app_timer module. Make sure that app_timer module has been initialized ahead.
 */

#ifndef APP_CALENDAR_H__
#define APP_CALENDAR_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "app_error.h"
#include "app_util.h"
#include "compiler_abstraction.h"
#include "app_timer.h"
#include "SEGGER_RTT.h"


typedef enum
{
	Jan = 1,
	Feb,
	Mar,
	Apr,
	May,
	Jun,
	Jul,
	Aug,
	Sep,
	Oct,
	Nov,
	Dec
} month_t;

typedef enum
{
	AM0,
	AM1,
	AM2,
	AM3,
	AM4,
	AM5,
	AM6,
	AM7,
	AM8,
	AM9,
	AM10,
	AM11,
	AM12,
	PM1,
	PM2,
	PM3,
	PM4,
	PM5,
	PM6,
	PM7,
	PM8,
	PM9,
	PM10,
	PM11,
	PM12
} hour_t;

/**@brief Calender type.
 */
typedef struct app_calendar_s {
       	uint16_t year;
	month_t  month;
	uint8_t  date;
	hour_t   hour;
	uint8_t  minute;
} app_calendar_t;


/**@brief Function for initializing the calendar module.
 *
 * @param[in]  prescaler           Value of the RTC1 PRESCALER register. Set to 0 for no prescaling.
 *
 * @retval     NRF_SUCCESS               If the module was initialized successfully.
 */
uint32_t app_calendar_init(uint32_t prescaler);

/**@brief Function for setting calendar.
 *
 * @param[in]  calendar          A calendar structure with the values to set.
 *
 *
 * @note This function does the calendar modification in the caller's context. It is also not protected
 *       by a critical region. Therefore care must be taken not to call it from several interrupt
 *       levels simultaneously.
 */
void app_calendar_set(app_calendar_t const    calendar);

/**@brief Function for getting the calendar.
 *
 * @param[in]       p_calendar    Pointer to the calendar which is accessible for application program.
 *
 */
void app_calendar_get(app_calendar_t * p_calendar);

/**@brief Function for storing the calendar into persistent storage.
 *
 * @retval     NRF_SUCCESS              If the calendar was successfully stored.
 *                                       has not been created.
 */
uint32_t app_calendar_store(void);

#endif // APP_CALENDAR_H__

/** @} */
