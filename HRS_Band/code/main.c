/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
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
 * @defgroup ble_sdk_app_template_main main.c
 * @{
 * @ingroup ble_sdk_app_template
 * @brief Template project main file.
 *
 * This file contains a template for creating a new application. It has the code necessary to wakeup
 * from button, advertise, get a connection restart advertising on disconnect and if no new
 * connection created go back to system-off mode.
 * It can easily be used as a starting point for creating a new application, the comments identified
 * with 'YOUR_JOB' indicates where and how you can customize.
 */

#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_drv_twi.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_mhrs.h"
#include "ble_conn_params.h"
#include "boards.h"
#include "sensorsim.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "device_manager.h"
#include "pstorage.h"
#include "app_trace.h"
#include "bsp.h"
#include "bsp_btn_ble.h"
#include "SEGGER_RTT.h"
#include "SEGGER_RTT_Conf.h"
#include "nrf_delay.h"
#include "app_util_platform.h"
#include "nrf_drv_gpiote.h"
#include "twi_master.h"

#include "app_scheduler.h"
#include "app_timer_appsh.h"
#include "ble_cts_c.h"
#include "ble_db_discovery.h"

#define IS_SRVC_CHANGED_CHARACT_PRESENT  1                                          /**< Include or not the service_changed characteristic. if not enabled, the server's datamhrse cannot be changed for the lifetime of the device*/

#define DEVICE_NAME                      "HRS_Band"                               /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME                "ZJU"                      /**< Manufacturer. Will be passed to Device Information Service. */
#define APP_ADV_INTERVAL                 300                                        /**< The advertising interval (in units of 0.625 ms. This value corresponds to 25 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS       180                                        /**< The advertising timeout in units of seconds. */

#define APP_TIMER_PRESCALER              0                                          /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE          6                                          /**< Size of timer operation queues. */

#define HEART_RATE_MEAS_INTERVAL      APP_TIMER_TICKS(25, APP_TIMER_PRESCALER) /**< Battery level measurement interval (ticks). */
#define MIN_BATTERY_LEVEL                81                                         /**< Minimum simulated battery level. */
#define MAX_BATTERY_LEVEL                100                                        /**< Maximum simulated 7battery level. */
#define BATTERY_LEVEL_INCREMENT          1                                          /**< Increment between each simulated battery level measurement. */

#define MIN_CONN_INTERVAL                MSEC_TO_UNITS(100, UNIT_1_25_MS)           /**< Minimum acceptable connection interval (0.1 seconds). */
#define MAX_CONN_INTERVAL                MSEC_TO_UNITS(200, UNIT_1_25_MS)           /**< Maximum acceptable connection interval (0.2 second). */
#define SLAVE_LATENCY                    0                                          /**< Slave latency. */
#define CONN_SUP_TIMEOUT                 MSEC_TO_UNITS(4000, UNIT_10_MS)            /**< Connection supervisory timeout (4 seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER) /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY    APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER)/**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT     3                                          /**< Number of attempts before giving up the connection parameter negotiation. */

#define SECURITY_REQUEST_DELAY           APP_TIMER_TICKS(4000,APP_TIMER_PRESCALER)/**< Delay after connection until security request is sent, if necessary (ticks). */
#define SEC_PARAM_TIMEOUT                30
    /**< Time-out for pairing request or security request (in seconds). */

#define SEC_PARAM_BOND                   1                                          /**< Perform bonding. */
#define SEC_PARAM_MITM                   0                                          /**< Man In The Middle protection not required. */
#define SEC_PARAM_IO_CAPABILITIES        BLE_GAP_IO_CAPS_NONE                       /**< No I/O capabilities. */
#define SEC_PARAM_OOB                    0                                          /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE           7                                          /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE           16                                         /**< Maximum encryption key size. */

#define DEAD_BEEF                        0xDEADBEEF                                 /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

/*Common addresses definition for EM7028. */
#define EM7028_ADDR         0x24U

#define EM7028_CONFIGURE    0x01U
#define EM7028_HRS1_CONTROL 0x0DU
#define EM7028_INT_CTRL     0x0EU
#define EM7028_SOFT_RESET   0x0FU
#define HRS1_DATA0_LOW      0x28U
#define HRS1_DATA0_HIGH     0x29U



#define CONTINUOUS_MODE     0x0FU

static dm_application_instance_t         m_app_handle;                              /**< Application identifier allocated by device manager */
static dm_handle_t                       m_peer_handle;                             /**< The pear that is currently connected. */

static uint16_t                          m_conn_handle = BLE_CONN_HANDLE_INVALID;   /**< Handle of the current connection. */

static ble_db_discovery_t                m_ble_db_discovery;                        /**< Structure used to identify the DB Discovery module. */
static ble_cts_c_t                       m_cts;                                     /**< Structure to store the data of the current time service. */

/* YOUR_JOB: Declare all services structure your application is using
static ble_xx_service_t                     m_xxs;
static ble_yy_service_t                     m_yys;
*/
static uint8_t count;            // 记录每次心跳间隔
static uint8_t time_space[30];   // 记录最近十次心跳间隔
static int16_t HRS_buffer[100];  // 记录最近200个采样点
static uint8_t heart_rate;   // 计算出的心率
static ble_mhrs_t                         m_mhrs;                                     /**< Structure used to identify the battery service. */
//static sensorsim_cfg_t                   m_battery_sim_cfg;                         /**< Battery Level sensor simulator configuration. */
//static sensorsim_state_t                 m_battery_sim_state;                       /**< Battery Level sensor simulator state. */
// YOUR_JOB: Use UUIDs for service(s) used in your application.
static ble_uuid_t m_adv_uuids[] = {{BLE_UUID_DEVICE_INFORMATION_SERVICE, BLE_UUID_TYPE_BLE},{BLE_UUID_CURRENT_TIME_SERVICE, BLE_UUID_TYPE_BLE}}; /**< Universally unique service identifiers. */
/* Indicates if reading operation from accelerometer has ended. */
static volatile bool m_xfer_done = true;
/* Indicates if setting mode operation has ended. */
static volatile bool m_set_mode_done;
/* TWI instance. */
static const nrf_drv_twi_t m_twi_em_7028 = NRF_DRV_TWI_INSTANCE(0);
APP_TIMER_DEF(m_mhrs_timer_id);                                                 /**< Battery timer. */
                                   
APP_TIMER_DEF(m_sec_req_timer_id);                                              /**< Security Request timer. */

#define SCHED_MAX_EVENT_DATA_SIZE sizeof(app_timer_event_t)                     /**< Maximum size of scheduler events. Note that scheduler BLE stack events do not contain andy data, ad the events are being pulled from the stack in the event handler. */
#define SCHED_QUEUE_SIZE          8                                             /**< Maximum number of events in the scheduler queue. */

//static ble_uuid_t m_adv_uuids[] = {};

/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num   Line number of the failing ASSERT call.
 * @param[in] file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for handling the Current Time Service errors.
 *
 * @param[in]  nrf_error  Error code containing information about what went wrong.
 */
static void current_time_error_handler(uint32_t nrf_error)
{
    SEGGER_RTT_printf(0,"[cts err handler]\r\n");
    SEGGER_RTT_WaitKey();
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for handling the security request timer time-out.
 *
 * @details This function will be called each time the security request timer expires.
 *
 * @param[in] p_context  Pointer used for passing some arbitrary information (context) from the app_start_timer() call to the time-out handler.
 *
 */
static void sec_req_timeout_handler(void * p_context)
{
    uint32_t              err_code;
    dm_security_status_t  status;

    if (m_conn_handle != BLE_CONN_HANDLE_INVALID)
    {
	err_code = dm_security_status_req(&m_peer_handle, &status);
	if(err_code != NRF_SUCCESS)
	{
		SEGGER_RTT_printf(0,"[dm sec sta req]err:%x\r\n",err_code);
		SEGGER_RTT_WaitKey();
	}
	APP_ERROR_CHECK(err_code);

	// If the link is still not secured by the pear, initiate security procedure.
	if (status == NOT_ENCRYPTED)
	{
	    err_code = dm_security_setup_req(&m_peer_handle);
	    if(err_code != NRF_SUCCESS)
	    {
		    SEGGER_RTT_printf(0,"[dm sec setup req]err:%x\r\n",err_code);
		    SEGGER_RTT_WaitKey();
	    }
	    APP_ERROR_CHECK(err_code);
	}
    }
}

/**@brief Function for printing current time.
 *
 * @param[in] p_evt  Event received from the Current Time Service client.
 *
 */
static void current_time_print(ble_cts_c_evt_t * p_evt)
{
    SEGGER_RTT_printf(0,"\r\n[current_time_print]:\r\n");
    SEGGER_RTT_printf(0,"%2d:%2d'%2d\"%3d %d/%d/%4d Weekday:%d\r\n",
		        p_evt->params.current_time.exact_time_256.day_date_time.date_time.hours,
		        p_evt->params.current_time.exact_time_256.day_date_time.date_time.minutes,
		        p_evt->params.current_time.exact_time_256.day_date_time.date_time.seconds,
		        p_evt->params.current_time.exact_time_256.fractions256,
		        p_evt->params.current_time.exact_time_256.day_date_time.date_time.month,
		        p_evt->params.current_time.exact_time_256.day_date_time.date_time.day,
		        p_evt->params.current_time.exact_time_256.day_date_time.date_time.year,
		        p_evt->params.current_time.exact_time_256.day_date_time.day_of_week);
    SEGGER_RTT_printf(0,"Adjust reason:Daylight savings %x, Time zone %x, External update %x, Manual updata %x\r\n",
		        p_evt->params.current_time.adjust_reason.change_of_daylight_savings_time,
			p_evt->params.current_time.adjust_reason.change_of_time_zone,
			p_evt->params.current_time.adjust_reason.external_reference_time_update,
			p_evt->params.current_time.adjust_reason.manual_time_update);
}

/**@brief Function for performing battery measurement and updating the Battery Level characteristic
 *        in Battery Service.
 */
static void heart_rate_update(uint8_t heart_rate)
{
    uint32_t err_code;

    err_code = ble_mhrs_heart_rate_update(&m_mhrs, heart_rate);
    if ((err_code != NRF_SUCCESS) &&
        (err_code != NRF_ERROR_INVALID_STATE) &&
        (err_code != BLE_ERROR_NO_TX_BUFFERS) &&
        (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
        )
    {
        APP_ERROR_HANDLER(err_code);
    }
}


/**@brief Function for handling the Battery measurement timer timeout.
 *
 * @details This function will be called each time the battery level measurement timer expires.
 *
 * @param[in] p_context  Pointer used for passing some arbitrary information (context) from the
 *                       app_start_timer() call to the timeout handler.
 */
static void heart_rate_meas_timeout_handler(void * p_context)
{
	ret_code_t err_code;
	uint8_t reg = HRS1_DATA0_LOW;
	
    UNUSED_PARAMETER(p_context);
	heart_rate_update(heart_rate);
	
	count++;  // 每次计数器溢出count+1
	
	do
    {
        __WFE();
    }while(m_xfer_done == false);
    err_code = nrf_drv_twi_tx(&m_twi_em_7028, EM7028_ADDR, &reg, sizeof(reg), true);
    APP_ERROR_CHECK(err_code);
    m_xfer_done = false;
}


/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static void timers_init(void)
{
	uint32_t err_code;

    // Initialize timer module.
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);

    // Create timers.

    /* YOUR_JOB: Create any timers to be used by the application.
                 Below is an example of how to create a timer.
                 For every new timer needed, increase the value of the macro APP_TIMER_MAX_TIMERS by
                 one.
    uint32_t err_code;
    err_code = app_timer_create(&m_app_timer_id, APP_TIMER_MODE_REPEATED, timer_timeout_handler);
    APP_ERROR_CHECK(err_code); */
	
    err_code = app_timer_create(&m_mhrs_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                heart_rate_meas_timeout_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&m_sec_req_timer_id,
		                APP_TIMER_MODE_SINGLE_SHOT,
			       	sec_req_timeout_handler);
    APP_ERROR_CHECK(err_code); 

}


/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    /* YOUR_JOB: Use an appearance value matching the application's use case.
    err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_);
    APP_ERROR_CHECK(err_code); */

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the YYY Service events. 
 * YOUR_JOB implement a service handler function depending on the event the service you are using can generate
 *
 * @details This function will be called for all YY Service events which are passed to
 *          the application.
 *
 * @param[in]   p_yy_service   YY Service structure.
 * @param[in]   p_evt          Event received from the YY Service.
 *
 *
static void on_yys_evt(ble_yy_service_t     * p_yy_service, 
                       ble_yy_service_evt_t * p_evt)
{
    switch (p_evt->evt_type)
    {
        case BLE_YY_NAME_EVT_WRITE:
            APPL_LOG("[APPL]: charact written with value %s. \r\n", p_evt->params.char_xx.value.p_str);
            break;
        
        default:
            // No implementation needed.
            break;
    }
}*/

/**@brief Function for handling the Current Time Service client events.
 *
 * @details This function will be called for all events in the Current Time Service client that are passed to the application.
 *
 * @param[in] p_evt Event received from the Current Time Service client.
 *
 */
static void on_cts_c_evt(ble_cts_c_t * p_cts, ble_cts_c_evt_t * p_evt)
{
    switch (p_evt->evt_type)
    {
	case BLE_CTS_C_EVT_DISCOVERY_COMPLETE:
	    SEGGER_RTT_printf(0,"\r\nCurrent Time Service discoverd on server.\r\n");
	    break;
	case BLE_CTS_C_EVT_SERVICE_NOT_FOUND:
	    SEGGER_RTT_printf(0,"\r\nCurrent Time Service not found on server.\r\n");
	    break;
	case BLE_CTS_C_EVT_DISCONN_COMPLETE:
	    SEGGER_RTT_printf(0,"\r\nDisconnect Complete.\r\n");
	    break;
	case BLE_CTS_C_EVT_CURRENT_TIME:
	    SEGGER_RTT_printf(0,"\r\nCurrent Time received.\r\n");
	    current_time_print(p_evt);
	    break;
	case BLE_CTS_C_EVT_INVALID_TIME:
	    SEGGER_RTT_printf(0,"\r\nInvalid Time received.\r\n");
	    break;

	default:
	    break;
    }
}

/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
		
	uint32_t       err_code;
    ble_mhrs_init_t mhrs_init;
    ble_cts_c_init_t cts_init_obj;
		
	// Initialize Battery Service.
    memset(&mhrs_init, 0, sizeof(mhrs_init));

    // Here the sec level for the Battery Service can be changed/increased.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&mhrs_init.heart_rate_char_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&mhrs_init.heart_rate_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&mhrs_init.heart_rate_char_attr_md.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&mhrs_init.heart_rate_report_read_perm);

    mhrs_init.evt_handler          = NULL;
    mhrs_init.support_notification = true;
    mhrs_init.p_report_ref         = NULL;
    mhrs_init.initial_batt_level   = 0;

    err_code = ble_mhrs_init(&m_mhrs, &mhrs_init);
    APP_ERROR_CHECK(err_code);

    cts_init_obj.evt_handler   = on_cts_c_evt;
    cts_init_obj.error_handler = current_time_error_handler;
    err_code                   = ble_cts_c_init(&m_cts, &cts_init_obj);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module which
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail config parameter, but instead we use the event
 *                handler mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for starting timers.
*/
static void application_timers_start(void)
{
    /* YOUR_JOB: Start your timers. below is an example of how to start a timer.
    uint32_t err_code;
    err_code = app_timer_start(m_app_timer_id, TIMER_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code); */
   
		uint32_t err_code;

    // Start application timers.
    err_code = app_timer_start(m_mhrs_timer_id, HEART_RATE_MEAS_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
	uint8_t reg[2] = {EM7028_CONFIGURE, 0x00U};
    uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);
	

	SEGGER_RTT_printf(0,"enter sleep mode\n");
	err_code = nrf_drv_twi_tx(&m_twi_em_7028, EM7028_ADDR, reg, sizeof(reg), false);  
    APP_ERROR_CHECK(err_code);
	nrf_delay_ms(500);
	
    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break;
        case BLE_ADV_EVT_IDLE:
            //sleep_mode_enter();
            break;
        default:
            break;
    }
}


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t err_code;

    switch (p_ble_evt->header.evt_id)
            {
        case BLE_GAP_EVT_CONNECTED:
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
			//application_timers_start();  // 连接上后开始计时器
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
			sleep_mode_enter();
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the BLE Stack event interrupt handler after a BLE stack
 *          event has been received.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    dm_ble_evt_handler(p_ble_evt);
    ble_db_discovery_on_ble_evt(&m_ble_db_discovery, p_ble_evt);
    ble_conn_params_on_ble_evt(p_ble_evt);
    bsp_btn_ble_on_ble_evt(p_ble_evt);
    on_ble_evt(p_ble_evt);
    ble_advertising_on_ble_evt(p_ble_evt);
    /*YOUR_JOB add calls to _on_ble_evt functions from each service your application is using
    ble_xxs_on_ble_evt(&m_xxs, p_ble_evt);
    ble_yys_on_ble_evt(&m_yys, p_ble_evt);
    */
    ble_mhrs_on_ble_evt(&m_mhrs, p_ble_evt);
    ble_cts_c_on_ble_evt(&m_cts, p_ble_evt);
}


/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 * @param[in] sys_evt  System stack event.
 */
static void sys_evt_dispatch(uint32_t sys_evt)
{
    pstorage_sys_event_handler(sys_evt);
    ble_advertising_on_sys_evt(sys_evt);
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    uint32_t err_code;

    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_XTAL_20_PPM, NULL);

#if defined(S110) || defined(S130) || defined(S132)
    // Enable BLE stack.
    ble_enable_params_t ble_enable_params;
    memset(&ble_enable_params, 0, sizeof(ble_enable_params));
#if (defined(S130) || defined(S132))
    ble_enable_params.gatts_enable_params.attr_tab_size   = BLE_GATTS_ATTR_TAB_SIZE_MIN;
#else
    ble_enable_params.gatts_enable_params.attr_tab_size   = 0X500;
#endif
    ble_enable_params.gatts_enable_params.service_changed = IS_SRVC_CHANGED_CHARACT_PRESENT;
    err_code = sd_ble_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);
#endif

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for the Event Schedular initialization.
 */
static void scheduler_init(void)
{
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}

/**@brief Function for handling the Device Manager events.
 *
 * @param[in] p_evt  Data associated to the device manager event.
 */
static uint32_t device_manager_evt_handler(dm_handle_t const * p_handle,
                                           dm_event_t const  * p_event,
                                           ret_code_t        event_result)
{
    uint32_t  err_code;
    APP_ERROR_CHECK(event_result);

    switch (p_event->event_id)
    {
	case DM_EVT_CONNECTION:
	    m_peer_handle = (*p_handle);
	    err_code      = app_timer_start(m_sec_req_timer_id, SECURITY_REQUEST_DELAY,NULL);
	    APP_ERROR_CHECK(err_code);
	    break;
	case DM_EVT_LINK_SECURED:
	    err_code = ble_db_discovery_start(&m_ble_db_discovery,
			                      p_event->event_param.p_gap_param->conn_handle);
	    APP_ERROR_CHECK(err_code);
	    break;

	default:
	    // No implementation needed.
	    break;
    }
#ifdef BLE_DFU_APP_SUPPORT
    if (p_event->event_id == DM_EVT_LINK_SECURED)
    {
        app_context_load(p_handle);
    }
#endif // BLE_DFU_APP_SUPPORT

    return NRF_SUCCESS;
}


/**@brief Function for the Device Manager initialization.
 *
 * @param[in] erase_bonds  Indicates whether bonding information should be cleared from
 *                         persistent storage during initialization of the Device Manager.
 */
static void device_manager_init(bool erase_bonds)
{
    uint32_t               err_code;
    dm_init_param_t        init_param = {.clear_persistent_data = erase_bonds};
    dm_application_param_t register_param;

    // Initialize persistent storage module.
    err_code = pstorage_init();
    APP_ERROR_CHECK(err_code);

    err_code = dm_init(&init_param);
    APP_ERROR_CHECK(err_code);

    memset(&register_param.sec_param, 0, sizeof(ble_gap_sec_params_t));

    register_param.sec_param.bond         = SEC_PARAM_BOND;
    register_param.sec_param.mitm         = SEC_PARAM_MITM;
    register_param.sec_param.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    register_param.sec_param.oob          = SEC_PARAM_OOB;
    register_param.sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    register_param.sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
    register_param.evt_handler            = device_manager_evt_handler;
    register_param.service_type           = DM_PROTOCOL_CNTXT_ALL;

    err_code = dm_register(&m_app_handle, &register_param);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    uint32_t      err_code;
    ble_advdata_t advdata;

    // Build advertising data struct to pass into @ref ble_advertising_init.
    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance      = true;
    advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;
    advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    advdata.uuids_complete.p_uuids  = m_adv_uuids;

    ble_adv_modes_config_t options = {0};
    options.ble_adv_fast_enabled  = BLE_ADV_FAST_ENABLED;
    options.ble_adv_fast_interval = APP_ADV_INTERVAL;
    options.ble_adv_fast_timeout  = APP_ADV_TIMEOUT_IN_SECONDS;

    err_code = ble_advertising_init(&advdata, NULL, &options, on_adv_evt, NULL);
    APP_ERROR_CHECK(err_code);
}

/**
 * @brief Database discovery collector initialization.
 */
static void db_discovery_init(void)
{
    uint32_t err_code = ble_db_discovery_init();

    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated by button press.
 */
void bsp_event_handler(bsp_event_t event)
{
    uint32_t err_code;
    switch (event)
    {
        case BSP_EVENT_SLEEP:		
            sleep_mode_enter();				
            break;

        case BSP_EVENT_DISCONNECT:
            err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
			SEGGER_RTT_printf(0,"disconnect\n");
			sleep_mode_enter();  // 断开连接后进入睡眠模式
            break;

        case BSP_EVENT_WHITELIST_OFF:
            err_code = ble_advertising_restart_without_whitelist();
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;
						
        default:
            break;
    }
}


/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
static void buttons_leds_init(bool * p_erase_bonds)
{
    bsp_event_t startup_event;

    uint32_t err_code = bsp_init(BSP_INIT_LED | BSP_INIT_BUTTONS,
                                 APP_TIMER_TICKS(100, APP_TIMER_PRESCALER), 
                                 bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);

    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}


/**
 * 低通滤波器
 */
int HRS_filter(int data){
	static double filter_buffer[41];
	static double filter_coefficient[21] = {0, 0.00678190440717335, 0.0128304886003170, 0.0166758096650767,  \
	                                        0.0171482412850874,	0.0136648483471950,	0.00642694715522822, -0.00351836771450904,  \
	                                        -0.0143765462449265, -0.0238580166855590, -0.0295488021894095, -0.0293384634673633, \
	                                        -0.0218299499731156, -0.00665502893774734, 0.0153675652743912, 0.0422682866768967,  \
	                                        0.0711605486728263, 0.0986399430404164, 0.121286108430723, 0.136184500458578,       \
	                                        0.141379966397442};
	double average = 0;
	double filtered_data = 0;
	int i;
											
	for(i=0; i<40; i++){
		filter_buffer[i] = filter_buffer[i+1];
	}
	filter_buffer[40] = (double)data;
	
	for(i=0; i<20; i++){
		filtered_data = filtered_data + filter_coefficient[i] * (filter_buffer[i]+filter_buffer[40-i]);
		average = average + filter_buffer[i] + filter_buffer[40-i];
	}
	
	filtered_data = filtered_data + filter_coefficient[20] * filter_buffer[20];
	average = (average+filter_buffer[20])/41;
	filtered_data = filtered_data - average;
	return (int16_t)(filtered_data*100);
}


void HRS_detect(int16_t data){
	static bool flag = false;
	int i;
	int sum = 0;
	int threshold = 0;
	
	// 更新阈值和采样值数组
	for(i=0;i<99;i++){
		HRS_buffer[i] = HRS_buffer[i+1];
		threshold = threshold + abs(HRS_buffer[i]);
	}
	HRS_buffer[99] = data;
	threshold = (threshold + abs(data))/100;
//	SEGGER_RTT_printf(0,"%d\n",threshold);
	
	if (data<(0-threshold)){
		flag = true;
	}else if(flag && (data>threshold)){
		flag = false;
		//SEGGER_RTT_printf(0,"%d\n",++count);
		for(i=0;i<29;i++){  // 更新心跳时间间隔
			time_space[i] = time_space[i+1];
			sum = sum + time_space[i];
		}
		time_space[29] = count;
		sum = sum + time_space[29];  // 累加30次心跳的时间间隔
		heart_rate = 72000/sum;
		SEGGER_RTT_printf(0,"%d\n",count);
		count = 0;                   // 清零计数
		
	}
}


/**
 * @brief Function for setting active mode on MMA7660 accelerometer.
 */
void EM7028_set_mode(void)
{
    ret_code_t err_code;
    uint8_t reg[2] = {EM7028_CONFIGURE, 0x08U};
		
    err_code = nrf_drv_twi_tx(&m_twi_em_7028, EM7028_ADDR, reg, sizeof(reg), false);  
    APP_ERROR_CHECK(err_code);
	nrf_delay_ms(500);
	reg[0] = EM7028_HRS1_CONTROL;
	reg[1] = 0xC1U;
	err_code = nrf_drv_twi_tx(&m_twi_em_7028, EM7028_ADDR, reg, sizeof(reg), false);  
    APP_ERROR_CHECK(err_code);
	nrf_delay_ms(500);
	reg[0] = EM7028_INT_CTRL;
	reg[1] = 0x00U;
	err_code = nrf_drv_twi_tx(&m_twi_em_7028, EM7028_ADDR, reg, sizeof(reg), false);  
    APP_ERROR_CHECK(err_code);
	nrf_delay_ms(500);
    //while(m_set_mode_done == false);
	m_set_mode_done = true;
}


/**
 * @brief TWI events handler.
 */
void twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context)
{   
    ret_code_t err_code;
	int16_t HRS_data;
    //static sample_t m_sample;
	static uint8_t HRS_ADC[2];
	
    switch(p_event->type)
    {
        case NRF_DRV_TWI_RX_DONE:
			HRS_data = HRS_filter((HRS_ADC[1]<<8)+HRS_ADC[0]);
//			SEGGER_RTT_printf(0,"%d ",(HRS_ADC[1]<<8)+HRS_ADC[0]);
//			SEGGER_RTT_printf(0,"%d ",HRS_data);
			HRS_detect(HRS_data);
            m_xfer_done = true;
            break;
        case NRF_DRV_TWI_TX_DONE:
            if(m_set_mode_done != true)
            {
                //m_set_mode_done  = true;
                return;
            }
            m_xfer_done = false;
            /* Read 4 bytes from the specified address. */
            err_code = nrf_drv_twi_rx(&m_twi_em_7028, EM7028_ADDR, HRS_ADC, sizeof(HRS_ADC), false);
						
            APP_ERROR_CHECK(err_code);
            break;
		case NRF_DRV_TWI_ERROR:
            break;
        default:
            break;        
    }   
}

/**
 * @brief UART initialization.
 */
void twi_init (void)
{
    ret_code_t err_code;
    
    const nrf_drv_twi_config_t twi_em_7028_config = {
       .scl                = I2C_SCL_PIN,
       .sda                = I2C_SDA_PIN,
       .frequency          = NRF_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH
    };
    
    err_code = nrf_drv_twi_init(&m_twi_em_7028, &twi_em_7028_config, twi_handler, NULL);
    APP_ERROR_CHECK(err_code);
    nrf_drv_twi_enable(&m_twi_em_7028);
}

/**@brief Function for the Power manager.
 */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for application main entry.
 */
int main(void)
{
    uint32_t err_code;
	//double test = 0.1234567891;
    bool erase_bonds;
    uint8_t  loop_id,key;

    // Initialize.
	SEGGER_RTT_Init();
	//SEGGER_RTT_printf(0,"%lf\n", test);
    timers_init();
    buttons_leds_init(&erase_bonds);
    ble_stack_init();
    device_manager_init(erase_bonds);
    db_discovery_init();
    scheduler_init();
    gap_params_init();
    advertising_init();
    services_init();
	//sensor_simulator_init();
    conn_params_init();

		
	twi_init();
    EM7028_set_mode();
    // Start execution.
    application_timers_start();
    err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);

    // Enter main loop.
    for (loop_id=0;;loop_id++)
    {
	app_sched_execute();
        power_manage();
	SEGGER_RTT_printf(0,"\r\n[loop%d]Press R to read time\r\n",loop_id);
	key = SEGGER_RTT_WaitKey();
	if(key == 'r' || key == 'R')
	{
		err_code = ble_cts_c_current_time_read(&m_cts);
		if(err_code == NRF_ERROR_NOT_FOUND)
			SEGGER_RTT_printf(0,"\r\n[current_time_read]CTS is not discovered.\r\n");
	}

    }
}

/**
 * @}
 */
