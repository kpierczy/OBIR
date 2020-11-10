#include <string.h>            // Basic string operations
#include <sys/socket.h>        // Sockets-related constants
#include "esp_log.h"           // Logging
#include "coap.h"              // CoAP implementation
#include "coap_handlers.h"     // Handlers for CoAP resources [auth]

/* ---------------------------------- Configuration ---------------------------------- */

// Set this to 9 to get verbose logging from within libcoap
#define COAP_LOGGING_LEVEL 0

/* --------------------------- Global & static definitions --------------------------- */

// Source file's tag
static char *TAG = "coap_server";

// Data buffers for CoAP communication
char espressif_data[100];
int espressif_data_len = 0;

/* ------------------------------------- Declarations --------------------------------- */

// Global variables initialization
extern TaskHandle_t main_handler;

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
        ESP_LOGI(TAG, "Initializing CoAP socket");
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
        ESP_LOGI(TAG, "Creating the endpoint");
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
        ESP_LOGI(TAG, "Initializing server's resources");
        coap_register_handler(resource,    COAP_REQUEST_GET, hnd_espressif_get);
        coap_register_handler(resource,    COAP_REQUEST_PUT, hnd_espressif_put);
        coap_register_handler(resource, COAP_REQUEST_DELETE, hnd_espressif_delete);
        coap_resource_set_get_observable(resource, 1);
        coap_add_resource(ctx, resource);


        // Run main processing loop
        ESP_LOGI(TAG, "Beginning dispatch loop");
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
    ESP_LOGI(TAG, "Cleaning up CoAP context");
    coap_free_context(ctx);
    coap_cleanup();

    // Notify main to disconec from WiFi AP
    xTaskNotifyGive(main_handler);

    // Delete task itself
    vTaskDelete(NULL);
}
