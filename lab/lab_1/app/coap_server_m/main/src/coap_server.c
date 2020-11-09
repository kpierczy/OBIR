#include <string.h>
#include <sys/socket.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "coap.h"

/* ---------------------------------- Configuration ---------------------------------- */

// Set this to 9 to get verbose logging from within libcoap
#define COAP_LOGGING_LEVEL 9

/* --------------------------- Global & static definitions --------------------------- */

// Source file's tag
static char *TAG = "coap_server";

// Data buffers for CoAP communication
static char espressif_data[100];
static int espressif_data_len = 0;

/* ------------------------------------- Declarations --------------------------------- */

// Global variables initialization
extern TaskHandle_t main_handler;

/* ------------------------------------ Static Code ----------------------------------- */

/**
 * @brief Handler for GET request
 */
static void
hnd_espressif_get(
    coap_context_t *ctx,
    coap_resource_t *resource,
    coap_session_t *session, 
    coap_pdu_t *request,
    coap_binary_t *token, 
    coap_string_t *query,
    coap_pdu_t *response
){
    // Sens data with dedicated function
    coap_add_data_blocked_response(
        resource,
        session,
        request,
        response,
        token,
        COAP_MEDIATYPE_TEXT_PLAIN,
        0,
        (size_t)espressif_data_len,
        (const u_char *) espressif_data
    );
}

/**
 * @brief Handler for PUT request
 */
static void
hnd_espressif_put(
    coap_context_t *ctx,
    coap_resource_t *resource,
    coap_session_t *session,
    coap_pdu_t *request,
    coap_binary_t *token,
    coap_string_t *query,
    coap_pdu_t *response
){
    // Initializes sending notifications about resource state's change to observers
    coap_resource_notify_observers(resource, NULL);

    // Response with 'Created' code if data was not initialized by client yet
    if (strcmp (espressif_data, "no data") == 0) {
        response->code = COAP_RESPONSE_CODE(201);
    }
    // Response with 'Changed' code if data was initialized by client
    else {
        response->code = COAP_RESPONSE_CODE(204);
    }

    // Get payload from the PDU
    size_t size;
    unsigned char *data;
    coap_get_data(request, &size, &data);

    // If payload is empty, deinitialize the data
    if (size == 0) {
        snprintf(espressif_data, sizeof(espressif_data), "no data");
        espressif_data_len = strlen(espressif_data);
    } 
    // Otherwise, update value of the resource
    else {
        espressif_data_len = size > sizeof (espressif_data) ? sizeof (espressif_data) : size;
        memcpy (espressif_data, data, espressif_data_len);
    }
}

/**
 * @brief Handler for DELETE request
 */
static void
hnd_espressif_delete(
    coap_context_t *ctx,
    coap_resource_t *resource,
    coap_session_t *session,
    coap_pdu_t *request,
    coap_binary_t *token,
    coap_string_t *query,
    coap_pdu_t *response
){
    // Notify observers about rejecting a resource 
    coap_resource_notify_observers(resource, NULL);

    // Deinitialize the resource
    snprintf(espressif_data, sizeof(espressif_data), "no data");
    espressif_data_len = strlen(espressif_data);

    // Set return code on 'Deleted'
    response->code = COAP_RESPONSE_CODE(202);
}

/* ------------------------------------ Thread Code ----------------------------------- */

/**
 * @brief Thread running CoAP server
 * @param pvParameters
 */
void coap_example_thread(void *pvParameters){

    // Wait for main to block on xTaskNotifyTake()
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Initialize data buffer
    snprintf(espressif_data, sizeof(espressif_data), "no data");
    espressif_data_len = strlen(espressif_data);

    // Set CoAP module logging level
    coap_set_log_level(COAP_LOGGING_LEVEL);

    // Create CoAP module's context
    coap_context_t *ctx = NULL;

    // Run CoAP initialization process
    while (1) {

        /* Prepare the CoAP server socket (it's wrapper around BSD socket approach)
         *
         *     .sin_addr.s_addr : address of the socket (setting to 0 picks random
         *                        addres possessed by the server. ESP has only one
         *                        IP available at the time)
         *     .sin_family : protocoly type (AF_INET = IPv4)
         *     .sin_port : port number for th socket (setting to 0 picks random port)
         * 
         * @note : htonl() and htons() functions revert IP and PORT byte order to 
         * suite network order.
         */
        coap_address_t   serv_addr;
        coap_address_init(&serv_addr);
        serv_addr.addr.sin.sin_addr.s_addr = INADDR_ANY;
        serv_addr.addr.sin.sin_family      = AF_INET;
        serv_addr.addr.sin.sin_port        = htons(COAP_DEFAULT_PORT);

        // Initialize CoAP's contex structure
        ctx = coap_new_context(NULL);
        if (!ctx) {
           continue;
        }

        // Create UDP endpoint
        coap_endpoint_t *ep_udp = coap_new_endpoint(ctx, &serv_addr, COAP_PROTO_UDP);
        if (!ep_udp) {
           break;
        }

        /* ================================================================= */
        /*                   Initialize server's resources                   */
        /* ================================================================= */

        // Resource: 'Espressif' (string) observable
        coap_resource_t *resource = NULL;
        if( !(resource = coap_resource_init(coap_make_str_const("Espressif"), 0) ))
           break;
        coap_register_handler(resource,    COAP_REQUEST_GET, hnd_espressif_get);
        coap_register_handler(resource,    COAP_REQUEST_PUT, hnd_espressif_put);
        coap_register_handler(resource, COAP_REQUEST_DELETE, hnd_espressif_delete);
        coap_resource_set_get_observable(resource, 1);
        coap_add_resource(ctx, resource);


        // Run main processing loop
        unsigned wait_ms = COAP_RESOURCE_CHECK_TIME * 1000;
        while (1) {

            // Server incoming and outcoming packages
            int result = coap_run_once(ctx, wait_ms);

            // Back to CoAP server initialization, when error occurs 
            if (result < 0)
                break;
            // Decrement timeout if the last one was shorter than expected
            else if (result < wait_ms)
                wait_ms -= result;
            // Reset the timeout otherwise
            else
                wait_ms = COAP_RESOURCE_CHECK_TIME * 1000;
        }
    }

    // Clean context of the CoAP module before finishing task
    coap_free_context(ctx);
    coap_cleanup();

    // Notify main to disconec from WiFi AP
    xTaskNotifyGive(main_handler);

    // Delete task itself
    vTaskDelete(NULL);
}
