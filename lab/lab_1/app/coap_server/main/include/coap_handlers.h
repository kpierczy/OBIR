/**
 * @brief 
 * 
 * @param ctx 
 * @param resource 
 * @param session 
 * @param request 
 * @param token 
 * @param query 
 * @param response 
 */
void hnd_espressif_get(
    coap_context_t *ctx,
    coap_resource_t *resource,
    coap_session_t *session, 
    coap_pdu_t *request,
    coap_binary_t *token, 
    coap_string_t *query,
    coap_pdu_t *response
);

/**
 * @brief Construct a new hnd espressif put object
 * 
 * @param ctx 
 * @param resource 
 * @param session 
 * @param request 
 * @param token 
 * @param query 
 * @param response 
 */
void hnd_espressif_put(
    coap_context_t *ctx,
    coap_resource_t *resource,
    coap_session_t *session,
    coap_pdu_t *request,
    coap_binary_t *token,
    coap_string_t *query,
    coap_pdu_t *response
);

/**
 * @brief 
 * 
 * @param ctx 
 * @param resource 
 * @param session 
 * @param request 
 * @param token 
 * @param query 
 * @param response 
 */
void hnd_espressif_delete(
    coap_context_t *ctx,
    coap_resource_t *resource,
    coap_session_t *session,
    coap_pdu_t *request,
    coap_binary_t *token,
    coap_string_t *query,
    coap_pdu_t *response
);