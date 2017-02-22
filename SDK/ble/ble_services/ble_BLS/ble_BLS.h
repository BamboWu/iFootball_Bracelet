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
 * @defgroup ble_sdk_srv_BLS Button LED Service
 * @{
 * @ingroup ble_sdk_srv
 * @brief Button LED Service module.
 *
 * @details This module implements the Button LED Service with the Button characteristic and LED characteristic.
 *          During initialization it adds the two characteristics
 *          to the BLE stack database. 
 *          The module will support notification of the Button action
 *          through the ble_BLS_on_button_action() function.
 *          If an event handler is supplied by the application, the Button LED Service will
 *          generate LED write event to the application.
 *
 * @note The application must propagate BLE stack events to the Button LED Service module by calling
 *       ble_BLS_on_ble_evt() from the @ref softdevice_handler callback that is ble_evt_dispatch .
 *
 * @note Attention! 
 *  To maintain compliance with Nordic Semiconductor ASA Bluetooth profile 
 *  qualification listings, this section of source code must not be modified.
 */

#ifndef BLE_BLS_H__
#define BLE_BLS_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"

//#define BLS_UUID_BASE 0x23, 0xD1, 0xBC, 0xEA, 0x5F, 0x78, 0x23, 0x15, 0xDE, 0xEF, 0x12, 0x12, 0x00, 0x00, 0x00, 0x00
#define BLS_UUID_BASE {{0xEF,0xCD,0xAB,0x89,0x67,0x45,0x23,0x01,0xFE,0xAF,0xDA,0xBC,0x00,0x00,0x00,0x00}}
#define BLS_UUID_SERVICE   0x1024
#define BLS_UUID_LED_CHAR  0x1025
#define BLS_UUID_LED_DESCR 0x2025
#define BLS_UUID_BTN_CHAR  0x1026
#define BLS_UUID_BTN_DESCR 0x2026

/**@brief Button LED Service event type. */
typedef enum
{
    BLE_BLS_EVT_NOTIFICATION_ENABLED,                             /**< Battery value notification enabled event. */
    BLE_BLS_EVT_NOTIFICATION_DISABLED                             /**< Battery value notification disabled event. */
} ble_BLS_evt_type_t;

/**@brief Button LED Service event. */
typedef struct
{
    ble_BLS_evt_type_t evt_type;                                  /**< Type of event. */
} ble_BLS_evt_t;

// Forward declaration of the ble_BLS_t type. 
typedef struct ble_BLS_s ble_BLS_t;

/**@brief Button LED Service event handler type. */
typedef void (*ble_BLS_evt_handler_t) (ble_BLS_t * p_BLS, uint8_t new_state);

/**@brief Button LED Service init structure. This contains all options and data needed for
 *        initialization of the service.*/
typedef struct ble_BLS_init_s
{
    ble_BLS_evt_handler_t         evt_handler;                    /**< Event handler to be called for handling events in the Button LED Service. */
    uint8_t                       initial_LED_state;              /**< Initial LED state */
} ble_BLS_init_t;

/**@brief Button LED Service structure. This contains various status information for the service. */
typedef struct ble_BLS_s
{
    ble_BLS_evt_handler_t         evt_handler;                    /**< Event handler to be called for handling events in the Button LED Service. */
    uint16_t                      service_handle;                 /**< Handle of Button LED Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t      Button_handles;                 /**< Handles related to the Button characteristic. */
    ble_gatts_char_handles_t      LED_handles;                    /**< Handles related to the LED characteristic. */
    uint16_t                      LED_descr_handles;              /**< Handles related to the LED descriptor. */
    uint16_t                      conn_handle;                    /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
    uint8_t                       uuid_type;
} ble_BLS_t;

/**@brief Function for initializing the Button LED Service.
 *
 * @param[out]  p_BLS       Button LED Service structure. This structure will have to be supplied by
 *                          the application. It will be initialized by this function, and will later
 *                          be used to identify this particular service instance.
 * @param[in]   p_bas_init  Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 */
uint32_t ble_BLS_init(ble_BLS_t * p_BLS, const ble_BLS_init_t * p_BLS_init);

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the Button LED Service.
 *
 * @note For the requirements in the BLS specification to be fulfilled,
 *       ble_BLS_on_button_action() must be called upon reconnection if the
 *       button state has changed while the service has been disconnected from a bonded
 *       client.
 *
 * @param[in]   p_BLS      Button LED Service handle.
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 */
void ble_BLS_on_ble_evt(ble_BLS_t * p_BLS, ble_evt_t * p_ble_evt);

/**@brief Function for updating the Button state.
 *
 * @details The application calls this function after a button action reported by the app_button module.
 *          The Button characteristic is sent to the client.
 *
 * @note For the requirements in the BLS specification to be fulfilled,
 *       this function must be called upon reconnection if the Button state has changed
 *       while the service has been disconnected from a bonded client.
 *
 * @param[in]   p_BLS          Button LED Service structure.
 * @param[in]   Btn_state      New button state.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_BLS_on_button_action(ble_BLS_t * p_BLS, uint8_t Btn_state);

#endif // BLE_BLS_H__

/** @} */
