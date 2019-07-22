/*******************************************************************************
* @brief    Position Beacon Transmitter Demo Application.
* @author   Taras Zaporozhets <zaporozhets.taras@gmail.com>
* @date     July 23, 2019
*******************************************************************************/
#include "app_timer.h"
#include "app_uart.h"

#include "ble_advdata.h"
#include "ble_radio_notification.h"
#include "bsp.h"
#include "minmea/minmea.h"
#include "nordic_common.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_soc.h"
#include <stdbool.h>
#include <stdint.h>

#define DEVICE_NAME "tracker"
#define APP_COMPANY_IDENTIFIER 0xFFFF // For testing according Bluetooth SIG

#define APP_BLE_CONN_CFG_TAG 1 /**< A tag identifying the SoftDevice BLE configuration. */
#define NON_CONNECTABLE_ADV_INTERVAL MSEC_TO_UNITS(1000, UNIT_0_625_MS) /**< The advertising interval for non-connectable advertisement (100 ms). This value can vary between 100ms to 10.24s). */

#define DEAD_BEEF 0xDEADBEEF /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define UART_TX_BUF_SIZE 256 /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 256 /**< UART RX buffer size. */
#define NMEA_BUFFER 82 /**< NMEA buffer size. */

static ble_gap_adv_params_t m_adv_params; /**< Parameters to be passed to the stack when starting advertising. */
static uint8_t m_adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET; /**< Advertising handle used to identify an advertising set. */
static uint8_t m_enc_advdata[BLE_GAP_ADV_SET_DATA_SIZE_MAX]; /**< Buffer for storing an encoded advertising set. */
/*
 *@brief Struct that contains pointers to the encoded advertising data. 
 */
static ble_gap_adv_data_t m_adv_data = {
    .adv_data = { .p_data = m_enc_advdata, .len = BLE_GAP_ADV_SET_DATA_SIZE_MAX },
    .scan_rsp_data = { .p_data = NULL, .len = 0 }
};

/*
 *@brief Struct that contains position data. 
 */
struct PositionDataStr {
    int32_t latitude;
    int32_t longitude;
} __attribute__((packed)) m_beacon_info = { 0, 0 };

/*
 *@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in]   line_num   Line number of the failing ASSERT call.
 * @param[in]   file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t* p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/*
 *@brief Function for initializing the Advertising functionality.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 */
static void advertising_init(void)
{
    static ble_advdata_t advdata;
    uint32_t err_code;
    uint8_t flags = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED;
    ble_gap_conn_sec_mode_t sec_mode;

    ble_advdata_manuf_data_t manuf_specific_data;

    manuf_specific_data.company_identifier = APP_COMPANY_IDENTIFIER;

    manuf_specific_data.data.p_data = (uint8_t*)&m_beacon_info;
    manuf_specific_data.data.size = sizeof(m_beacon_info);

    // Build and set advertising data.
    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type = BLE_ADVDATA_SHORT_NAME;
    advdata.short_name_len = strlen(DEVICE_NAME);
    advdata.flags = flags;
    advdata.p_manuf_specific_data = &manuf_specific_data;

    // Initialize advertising parameters (used when starting advertising).
    memset(&m_adv_params, 0, sizeof(m_adv_params));

    m_adv_params.properties.type = BLE_GAP_ADV_TYPE_NONCONNECTABLE_NONSCANNABLE_UNDIRECTED;
    m_adv_params.p_peer_addr = NULL; // Undirected advertisement.
    m_adv_params.filter_policy = BLE_GAP_ADV_FP_ANY;
    m_adv_params.interval = NON_CONNECTABLE_ADV_INTERVAL;
    m_adv_params.duration = 0; // Never time out.

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode, (const uint8_t*)DEVICE_NAME, strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    err_code = ble_advdata_encode(&advdata, m_adv_data.adv_data.p_data, &m_adv_data.adv_data.len);
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_adv_set_configure(&m_adv_handle, &m_adv_data, &m_adv_params);
    APP_ERROR_CHECK(err_code);
}

/*
 *@brief Function for starting advertising.
 */
static void advertising_start(void)
{
    ret_code_t err_code;

    err_code = sd_ble_gap_adv_start(m_adv_handle, APP_BLE_CONN_CFG_TAG);
    APP_ERROR_CHECK(err_code);
}

/*
 *@brief Function for stoping advertising.
 */
static void advertising_stop(void)
{
    ret_code_t err_code;

    err_code = sd_ble_gap_adv_stop(m_adv_handle);
    APP_ERROR_CHECK(err_code);
}

/*
 *@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);
}

/*
 *@brief Function for initializing logging. 
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

/*
 *@brief Function for initializing timers. 
 */
static void timers_init(void)
{
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}

/*
 *@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}

/*
 *@brief Function for handling the idle state (main loop).
 */
static void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false) {
        nrf_pwr_mgmt_run();
    }
}

/*
 *@brief   Function for handling app_uart events.
 *
 * @details This function receives a single character from the app_uart module and appends it to
 *          a string. The string is parsed and sent over BLE advertising when the last character received is a
 *          'new line' '\n' (hex 0x0A) or if the string reaches the maximum data length.
 */
void uart_event_handle(app_uart_evt_t* p_event)
{
    static uint8_t data_array[NMEA_BUFFER] = { 0 };
    static uint16_t index = 0;

    switch (p_event->evt_type) {
    case APP_UART_DATA_READY: {
        uint8_t data = 0;
        uint32_t ret_val = app_uart_get(&data);
        if (NRF_SUCCESS != ret_val) {
            NRF_LOG_ERROR("Failed to pop data from FIFO!\n")
            return;
        }
        data_array[index++] = data;

        if ((data_array[index - 1] == '\n') || (index >= (NMEA_BUFFER))) {
            switch (minmea_sentence_id(data_array, false)) {
            case MINMEA_SENTENCE_RMC: {
                struct minmea_sentence_rmc frame;
                if (minmea_parse_rmc(&frame, data_array)) {
                    int32_t latitude = frame.latitude.value;
                    int32_t longitude = frame.longitude.value;
                    int32_t speed = minmea_rescale(&frame.speed, 1000);

                    NRF_LOG_DEBUG("$xxRMC fixed-point RAW coordinates and speed: (%d,%d) %d\n",
                        latitude, longitude, speed);

                    m_beacon_info.latitude = latitude;
                    m_beacon_info.longitude = longitude;
                    // FIXME: Dirty hack, make it without stop/start
                    advertising_stop();
                    advertising_init();
                    advertising_start();
                } else {
                    NRF_LOG_ERROR("$xxRMC sentence is not parsed\n");
                }
                break;
            }
            // TODO: Parse more data!
            default: {
                NRF_LOG_INFO("Unhandled header id\n");
            }
            }

            index = 0;
            memset(data_array, 0, sizeof(data_array));
        }
        break;
    }
    case APP_UART_COMMUNICATION_ERROR: {
        NRF_LOG_ERROR("Communication error occurred while handling UART.\n");
        index = 0;
        memset(data_array, 0, sizeof(data_array));
        break;
    }
    case APP_UART_FIFO_ERROR: {
        NRF_LOG_ERROR("Error occurred in FIFO module used by UART.\n");
        index = 0;
        memset(data_array, 0, sizeof(data_array));
        break;
    }
    default: {
        break;
    }
    }
}

/*
 *@brief Function for initializing the UART.
 */
static void uart_init(void)
{
    ret_code_t err_code;

    app_uart_comm_params_t const comm_params = {
        .rx_pin_no = RX_PIN_NUMBER,
        .tx_pin_no = TX_PIN_NUMBER,
        .rts_pin_no = RTS_PIN_NUMBER,
        .cts_pin_no = CTS_PIN_NUMBER,
        .flow_control = APP_UART_FLOW_CONTROL_DISABLED,
        .use_parity = false,
        .baud_rate = UART_BAUDRATE_BAUDRATE_Baud115200
    };

    APP_UART_FIFO_INIT(&comm_params,
        UART_RX_BUF_SIZE,
        UART_TX_BUF_SIZE,
        uart_event_handle,
        APP_IRQ_PRIORITY_LOWEST,
        err_code);

    APP_ERROR_CHECK(err_code);
}

/*
 *@brief Function for application main entry.
 */
int main(void)
{
    // Initialize.
    log_init();
    timers_init();
    uart_init();
    power_management_init();
    ble_stack_init();
    advertising_init();

    // Start execution.
    NRF_LOG_INFO("Position Beacon Transmitter Demo Application\n");
    advertising_start();

    // Enter main loop.
    for (;;) {
        idle_state_handle();
    }
}