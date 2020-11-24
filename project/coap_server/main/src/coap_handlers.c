
#include <string.h>            // Basic string operations
#include <errno.h>             // erno variable
#include "coap.h"              // CoAP implementation
#include "lwip/apps/sntp.h"
#include "coap_handlers.h"

/* --------------------------- Global & static definitions --------------------------- */

// Source file's tag
static char *TAG = "coap_handlers";

// '/colour' resource buffer
enum{R, G, B};
uint8_t colour[3] = {0};

/* ---------------------------------------- Code -------------------------------------- */

/**
 * @brief Initializes resources present on the server
 * 
 * @param context Pointer to the CoAP context stack
 * @returns 0 on success and a negative number at failure. At the failure
 *  all resources are deleted from the context.
 */
int resources_init(coap_context_t *context){

    coap_resource_t *resource = NULL;

    /* =================================================== */
    /*            Resource: 'time' (observable)            */
    /* =================================================== */

    // Synchronise system time with global
    if(! sntp_enabled() ){
        sntp_setoperatingmode(SNTP_OPMODE_POLL);
        sntp_setservername(0, "pool.ntp.org");
        sntp_init();
    }

    // Create a new resource
    if( !(resource = coap_resource_init(coap_make_str_const("time"), 0)) ){
        coap_delete_all_resources(context);
        return 0;
    }

    // Document a resource with attributes (describe resource when GET /.well-known/core is called)
    coap_add_attr(resource,    coap_make_str_const("ct"), coap_make_str_const("\"plain text\""), 0);
    coap_add_attr(resource,    coap_make_str_const("rt"),       coap_make_str_const("\"time\""), 0);
    coap_add_attr(resource,    coap_make_str_const("if"),        coap_make_str_const("\"GET\""), 0);

    // Register handlers for methods called on the resourse
    coap_register_handler(resource, COAP_REQUEST_GET, hnd_get);

    // Set the resource as observable
    coap_resource_set_get_observable(resource, 1);

    // Add the resource to the context
    coap_add_resource(context, resource);


    /* =================================================== */
    /*               Resource: 'colour'                    */
    /* =================================================== */

    // Create a new resource
    if( !(resource = coap_resource_init(coap_make_str_const("colour"), 0)) ){
        coap_delete_all_resources(context);
        return 0;
    }

    // Document a resource with attributes (describe resource when GET /.well-known/core is called)
    coap_add_attr(resource,  coap_make_str_const("ct"), coap_make_str_const("\"plain text\""), 0);
    coap_add_attr(resource,  coap_make_str_const("rt"),     coap_make_str_const("\"colour\""), 0);
    coap_add_attr(resource,  coap_make_str_const("if"),    coap_make_str_const("\"GET PUT\""), 0);
    coap_add_attr(resource, coap_make_str_const("put"),   coap_make_str_const("\"%d %d %d\""), 0);
    

    // Register handlers for methods called on the resourse
    coap_register_handler(resource,    COAP_REQUEST_GET, hnd_get);
    coap_register_handler(resource,    COAP_REQUEST_PUT, hnd_put);

    // Set the resource as observable
    coap_resource_set_get_observable(resource, 1);

    // Add the resource to the context
    coap_add_resource(context, resource);

    return 0;
}

/**
 * @brief Clears all resources from the context and closes SNTP deamon.
 * 
 * @param context Context related with resources to delete
 */
void resources_deinit(coap_context_t *context){
    sntp_stop();
    if(context)
        coap_delete_all_resources(context);
}

/**
 * @brief Handler for GET request
 */
void hnd_get(
    coap_context_t *ctx,
    coap_resource_t *resource,
    coap_session_t *session, 
    coap_pdu_t *request,
    coap_binary_t *token, 
    coap_string_t *query,
    coap_pdu_t *response
){

    // Handle ' GET /time' request
    if( resource == coap_get_resource_from_uri_path(ctx, coap_make_str_const("time")) ){
        
        // Get currnt time
        time_t now;
        time(&now);
        
        // Transform us time into string-formatted tm struct 
        char strftime_buf[100];
        struct tm timeinfo;
        localtime_r(&now, &timeinfo);
        size_t length = 
            strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);

        // Send data with dedicated function
        coap_add_data_blocked_response(
            resource,
            session,
            request,
            response,
            token,
            COAP_MEDIATYPE_TEXT_PLAIN,
            0,
            length,
            (uint8_t *) strftime_buf
        );
    }    
    // Handle ' GET /colour' request
    if( resource == coap_get_resource_from_uri_path(ctx, coap_make_str_const("colour")) ){
        
        // Construct text from the resource
        char colour_buf[18];
        size_t size = snprintf(colour_buf, 18, "R:%d G:%d B:%d", colour[R], colour[G], colour[B]);

        // Send data with dedicated function
        coap_add_data_blocked_response(
            resource,
            session,
            request,
            response,
            token,
            COAP_MEDIATYPE_TEXT_PLAIN,
            0,
            size,
            (uint8_t *) colour_buf
        );
    }
}

/**
 * @brief Handler for PUT request
 */
void hnd_put(
    coap_context_t *ctx,
    coap_resource_t *resource,
    coap_session_t *session,
    coap_pdu_t *request,
    coap_binary_t *token,
    coap_string_t *query,
    coap_pdu_t *response
){

    // Get requests's payload
    size_t size;
    uint8_t *data;
    coap_get_data(request, &size, &data);

    // If data parsing fails (or no payload is present) set response code to 2.04 (Changed)
    if (size == 0)
        response->code = COAP_RESPONSE_CODE(400);
    // Otherwise, try to parse data
    else {
        
        // Create colour backup
        uint8_t colour_bck[3];
        colour_bck[R] = colour[R];
        colour_bck[G] = colour[G];
        colour_bck[B] = colour[B];

        // By default, set response code to 2.04 (Changed)
        response->code = COAP_RESPONSE_CODE(204);

        // Parse three integers
        for(int i = 0; i < 3; ++i){

            char * endp;
            errno = 0;
            long col = strtol((char *) data, &endp, 10);

            // If parsing failed reasume backup and call error
            if((char *) data == endp){
                colour[R] = colour_bck[R];
                colour[G] = colour_bck[G];
                colour[B] = colour_bck[B];
                response->code = COAP_RESPONSE_CODE(400);
                break;
            } 
            // If parsed value is out of range, cut it to the range's edge 
            else if( col < 0)
                colour[i] = 0;            
            else if( col > 255)
                colour[i] = 255;
            // Otherwise, set colour value
            else
                colour[i] = (uint8_t) col;

            data = (uint8_t *) endp;
        }
    }
}
