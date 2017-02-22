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

/* Attention! 
*  To maintain compliance with Nordic Semiconductor ASA’s Bluetooth profile 
*  qualification listings, this section of source code must not be modified.
*/

#include "ble_BLS.h"
#include <string.h>
#include "nordic_common.h"
#include "ble_srv_common.h"
#include "app_util.h"
#include "SEGGER_RTT.h"

/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_BLS       Battery Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_connect(ble_BLS_t * p_BLS, ble_evt_t * p_ble_evt)
{
    p_BLS->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}


/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_BLS       Button LED Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_disconnect(ble_BLS_t * p_BLS, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_BLS->conn_handle = BLE_CONN_HANDLE_INVALID;
}

static void on_write(ble_BLS_t * p_BLS, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    if (p_BLS == NULL || p_ble_evt == NULL)
    {
        return;
    }
    
    if ((p_evt_write->handle == p_BLS->LED_handles.value_handle) && //write to correct characteristic -- LED char
        (p_evt_write->len == 1) && //write with correct size -- 1 byte
	(p_BLS->evt_handler != NULL)) //handler has been set
    {
	p_BLS->evt_handler(p_BLS, p_evt_write->data[0]);
    }
}

void ble_BLS_on_ble_evt(ble_BLS_t * p_BLS, ble_evt_t * p_ble_evt)
{
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_BLS, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_BLS, p_ble_evt);
            break;
            
        case BLE_GATTS_EVT_WRITE:
            on_write(p_BLS, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for adding the Button characteristic.
 *
 * @param[in]   p_BLS        Button LED Service structure.
 * @param[in]   p_BLS_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t button_char_add(ble_BLS_t * p_BLS, const ble_BLS_init_t * p_BLS_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    // Add Button characteristic
    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   =  1;
    char_md.char_props.notify =  1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_BLS->uuid_type;
    ble_uuid.uuid = BLS_UUID_BTN_CHAR;
    //BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_BATTERY_LEVEL_CHAR);
    //defined in ble_types.h
    //BLE_UUID_BLE_ASSIGN(inst,val) do {inst.type = BLE_UUID_TYPE_BLE; inst.uuid = val;}

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof(uint8_t);
    attr_char_value.p_value   = NULL;

    err_code = sd_ble_gatts_characteristic_add(p_BLS->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_BLS->Button_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    else
    {
	p_BLS->evt_handler(p_BLS,p_BLS_init->initial_LED_state);
    }

    return NRF_SUCCESS;
}

/**@brief Function for adding the LED characteristic.
 *
 * @param[in]   p_BLS        Button LED Service structure.
 * @param[in]   p_BLS_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t LED_char_add(ble_BLS_t * p_BLS, const ble_BLS_init_t * p_BLS_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    uint8_t             descr[] = "LED Characteristic";

    // Add Button characteristic
    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   =  1;
    char_md.char_props.write  =  1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = NULL;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_BLS->uuid_type;
    ble_uuid.uuid = BLS_UUID_LED_CHAR;
    //BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_BATTERY_LEVEL_CHAR);
    //defined in ble_types.h
    //BLE_UUID_BLE_ASSIGN(inst,val) do {inst.type = BLE_UUID_TYPE_BLE; inst.uuid = val;}

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof(uint8_t);
    attr_char_value.p_value   = NULL;

    err_code = sd_ble_gatts_characteristic_add(p_BLS->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_BLS->LED_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add descriptor
    ble_uuid.type = p_BLS->uuid_type;
    ble_uuid.uuid = BLS_UUID_LED_DESCR;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 0;
    
    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(descr);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = attr_char_value.init_len;
    attr_char_value.p_value   = descr;

    err_code = sd_ble_gatts_descriptor_add(p_BLS->LED_handles.value_handle,
                                           &attr_char_value,
                                           &p_BLS->LED_descr_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    return NRF_SUCCESS;
}


uint32_t ble_BLS_init(ble_BLS_t * p_BLS, const ble_BLS_init_t * p_BLS_init)
{
    if (p_BLS == NULL || p_BLS_init == NULL)
    {
        return NRF_ERROR_NULL;
    }
    
    uint32_t   err_code;
    ble_uuid_t ble_uuid;
    ble_uuid128_t base_uuid = BLS_UUID_BASE;

    // Add UUID to BLE stack's list
    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_BLS->uuid_type);
    if (err_code != NRF_SUCCESS)
    {
	 return err_code;
    }

    // Initialize service structure
    p_BLS->evt_handler               = p_BLS_init->evt_handler;
    p_BLS->conn_handle               = BLE_CONN_HANDLE_INVALID;

    // Add service
    ble_uuid.type = p_BLS->uuid_type;
    ble_uuid.uuid = BLS_UUID_SERVICE;

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_BLS->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Button characteristic
    err_code = button_char_add(p_BLS, p_BLS_init);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    // Add LED characteristic
    return LED_char_add(p_BLS, p_BLS_init);
}


uint32_t ble_BLS_on_button_action(ble_BLS_t * p_BLS, uint8_t button_state)
{
    if (p_BLS == NULL)
    {
        return NRF_ERROR_NULL;
    }
    
    uint32_t err_code = NRF_SUCCESS;

    // Send value if connected and notifying.
    if (p_BLS->conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        ble_gatts_hvx_params_t hvx_params;
        uint16_t len = sizeof(button_state);

        SEGGER_RTT_printf(0,"\r\n[ble_BLS_on_button_action]state:%x\r\n",button_state);

        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_BLS->Button_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.p_len  = &len;
        hvx_params.p_data = &button_state;

        err_code = sd_ble_gatts_hvx(p_BLS->conn_handle, &hvx_params);
	if(err_code != NRF_SUCCESS)
	    SEGGER_RTT_printf(0,"\r\n[sd_ble_gatts_hvx]error:%x\r\n",err_code);
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}
