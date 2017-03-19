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

#include "app_calendar.h"
#include <stdlib.h>
#include "nrf.h"
#include "nrf_soc.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "app_util.h"
#include "app_util_platform.h"

static app_calendar_t m_calendar;                                           /**< Calendar record located in app_timer.c */
static uint32_t m_timer_interval;                                           /**< Period (interval) for app timer generate event */

//bool m_in_calendar_set = 0;

APP_TIMER_DEF(m_calendar_timer_id);                                         /**< Allocate static memory for app_timer_t. When using functions from app_timer module, you can refer to it simply by m_calendar_timer_id. If you want to access the data directly, using m_calendar_timer_id_data! */

/**@brief Handler for every 1 second time out.
 *
 * @param[in] pcontext    Pointer to any context.
 */
static void calendar_timer_handler(void * pcontext)
{
    uint8_t daydayup = 0;
    //SEGGER_RTT_printf(0,"\r\n[calender_timer_handler]1s elapse!\r\n");
    if(m_calendar.second < 59)
	    m_calendar.second++;
    else
    {
	    m_calendar.second = 0;
	    if(m_calendar.minute < 59)
		    m_calendar.minute++;
	    else
	    {
		    m_calendar.minute = 0;
		    if(m_calendar.hour < 23)
			    m_calendar.hour++;
		    else
		    {
			    m_calendar.hour = 0;
			    daydayup = 1;
		    }
	    }
    }

    if(daydayup)
	    switch(m_calendar.month)
	    {
		    case Jan:
		    case Mar:
	            case May:
		    case Jul:
		    case Aug:
		    case Oct: if(m_calendar.date<31)
				      m_calendar.date++;
			      else
			      {
				      m_calendar.date=1;
				      m_calendar.month++;
			      }
			      break;
		    case Dec: if(m_calendar.date<31)
				      m_calendar.date++;
			      else
			      {
				      m_calendar.date=1;
				      m_calendar.month=Jan;
				      m_calendar.year++;
			      }
			      break;
		    case Apr:
		    case Jun:
		    case Sep:
		    case Nov: if(m_calendar.date<30)
				      m_calendar.date++;
			      else
			      {
				      m_calendar.date=1;
				      m_calendar.month++;
			      }
			      break;
		    case Feb: if((m_calendar.year%4)==0 && (m_calendar.year%400)!=0)
			      {
				      if(m_calendar.date<29)
					      m_calendar.date++;
				      else
				      {
					      m_calendar.date=1;
					      m_calendar.month++;
				      }
			      }
			      else
			      {
				      if(m_calendar.date<28)
					      m_calendar.date++;
				      else
				      {
					      m_calendar.date=1;
					      m_calendar.month++;
				      }
			      }
			      break;
	    }
}

/**@brief Function to initialize app calendar.
 *
 * @param[in] prescaler   Value of the RTC1 PRESCALER register. Set to 0 for no prescaling.
 */
uint32_t app_calendar_init(uint32_t prescaler)
{
    uint32_t err_code;
    
    err_code = app_timer_create(&m_calendar_timer_id,
	                        APP_TIMER_MODE_REPEATED,
		                calendar_timer_handler);
    APP_ERROR_CHECK(err_code);

    m_timer_interval = (uint32_t)ROUNDED_DIV((uint64_t)APP_TIMER_CLOCK_FREQ, (prescaler + 1));
    err_code = app_timer_start(m_calendar_timer_id,m_timer_interval,NULL);
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}

/**@brief Exported Function for setting calendar, used by application.
 */
void app_calendar_set(app_calendar_t const calendar)
{
    uint32_t err_code;

    err_code = app_timer_stop(m_calendar_timer_id);
    APP_ERROR_CHECK(err_code);
    m_calendar.year   = calendar.year;
    m_calendar.month  = calendar.month;
    m_calendar.date   = calendar.date;
    m_calendar.hour   = calendar.hour;
    m_calendar.minute = calendar.minute;
    m_calendar.second = calendar.second;
    err_code = app_timer_start(m_calendar_timer_id,m_timer_interval,NULL);
    APP_ERROR_CHECK(err_code);
}

/**@brief Exported Function for getting calendar, used by application.
 */
void app_calendar_get(app_calendar_t * p_calendar)
{
    p_calendar->year   = m_calendar.year;
    p_calendar->month  = m_calendar.month;
    p_calendar->date   = m_calendar.date;
    p_calendar->hour   = m_calendar.hour;
    p_calendar->minute = m_calendar.minute;
    p_calendar->second = m_calendar.second;
}

/**@brief Function for storing the calendar.
 */
uint32_t app_calendar_store(void)
{
    return NRF_SUCCESS;
}


