
#include <string.h>            // Basic string operations
#include "coap.h"              // CoAP implementation

/* --------------------------- Global & static definitions --------------------------- */

// Source file's tag
static char *TAG = "coap_handlers";

/* ------------------------------------- Declarations --------------------------------- */

// Data buffers for CoAP communication
extern char espressif_data[100];
extern int espressif_data_len;

/* ------------------------------------ Static Code ----------------------------------- */

/**
 * @brief Handler for GET request
 */
void hnd_espressif_get(
    coap_context_t *ctx,
    coap_resource_t *resource,
    coap_session_t *session, 
    coap_pdu_t *request,
    coap_binary_t *token, 
    coap_string_t *query,
    coap_pdu_t *response
){
    // Send data with dedicated function
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
void hnd_espressif_put(
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
void hnd_espressif_delete(
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