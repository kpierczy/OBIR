**coap_run_once(context, timeout)**:
    -> **coap_write(context, sockets_list, sockets_list_len, &sockets_num, now)** [All actions related to writing data to peers: observers' otifications, adding endpoint's and context/endpoint sessions's sockets to the select's list, when they are r/w-wanting, freeing unused endpoints' sessions, retransmitting all packet's from the context's sendqueue whose ACK timeout expired]
        -> **coap_check_notify(context)** [Notifies all observers if the corresponding resource has changed, or some observers was not notified earlier]
            -> **coap_notify_observers(context, resource)** [Notifies observers of a single resource]
                -> **coap_send(observer->session, response)** [Starts the process of notification's sending to the observer]
                    -> **coap_send_pdu(observer->session, response, NULL)** [Decides whether response should be sent or delayed]
                        -> **coap_session_delay_pdu(observer->session, response, NULL)** [Puts the response in the session's delayqueue]
                        -> **coap_session_send_pdu(observer->session, response)** [Turns response PDU object into the typeless data payload and pass it deeper; calls coap_show_pdu(LOG_DEBUG, ...)]
                            -> **coap_session_send(observer->session, response->payload_buffer, response->payload_size)** [Checks whether session has it's own socket. If not, selects socket of the endpoint that session belongs to]
                                -> **coap_socket_send(choosen_sock, observer->session, response->payload_buffer, response->payload_size)** [Calls networ_send handler registered in the context (Default: coap_network_send(...))]
                                    -> **coap_network_send(choosen_socket, observer->session, response->payload_buffer, response->payload_size)** [Checks coap_debug_send_packet(). If it returns true, sends data using sys-call send() or sendto() depending whether the choosen_sock was connected or not (socket is connected if the session was created with coap_session_create_client())]
                    -> **coap_wait_ack(observer->session, some_node)** [If the response was sent and it was a CON one, adds the the node representing PDU to the context's sendqueue]
        -> **coap_session_free(session)** [Called in the loops. All sessions hold by all endpoints that are not used, are freed]
        -> **coap_retransmit(context, context->sendqueue_head)** [Retansmits a message from the context's sendqueue (if retransmission limit is not exceeded)]
            -> **coap_send_pdu(context->sendqueue_head->session, context->sendqueue_head->pdu, context->sendqueue_head)** [Decides whether response should be sent or delayed]
                        -> **coap_session_delay_pdu(context->sendqueue_head->session, context->sendqueue_head->pdu, context->sendqueue_head)** [Puts the PDU in the session's delayqueue. Removes it from the context's sendqueue]
                        -> **coap_session_send_pdu(context->sendqueue_head->session, context->sendqueue_head->pdu)** [Turns a PDU object into the typeless data payload and pass it deeper; calls coap_show_pdu(LOG_DEBUG, ...)]
                            -> **coap_session_send(context->sendqueue_head->session, context->sendqueue_head->pdu->payload_buffer, context->sendqueue_head->pdu->payload_size)** [Checks whether session has it's own socket. If not, selects socket of the endpoint that session belongs to]
                                -> **coap_socket_send(choosen_sock, context->sendqueue_head->session, context->sendqueue_head->pdu->payload_buffer, context->sendqueue_head->pdu->payload_size)** [Calls send networ_send handler registered in the context (Default: coap_network_send(...))]
                                    -> **coap_network_send(choosen_sock, context->sendqueue_head->session, context->sendqueue_head->pdu->payload_buffer, context->sendqueue_head->pdu->payload_size)** [Checks coap_debug_send_packet(). If it returns true, sends data using sys-call send() or sendto() depending whether the choosen_sock was connected or not (socket is connected if the session was created with coap_session_create_client())]
            -> **coap_handle_failed_notify(context->sendqueue_head->session, context->sendqueue_head->session->token)** [When the message 's retransmission counter ecxeeded the limit, calls coap_remove_failed_observer() fall all registered resource (just in case the message was a notification)]
                -> **coap_remove_failed_observer(context, resource, context->sendqueue_head->session, context->sendqueue_head->session->token)** [Looks for observers who match (session, token) pair and removes them]
                    -> **coap_cancel_all_messages(context, context->sendqueue_head->session, context->sendqueue_head->session->token, context->sendqueue_head->session->token_length)** [Removes all CON messages from the context's sendqueue that was related to the removed observer]
            -> **coap_session_connected(context->sendqueue_head->session)** [If the message was discarded (due to retransmission limit met) the context->sendqueue_head->session->con_active counter is decremented. Then, the session can open a new CON connection and so potentially flush a PDU that has been delayed to the session's delayqueue. Call to this function iterates over the delayqueue and ]
                -> **coap_session_send_pdu(context->sendqueue_head->session, delayed_node->pdu)** [Turns response PDU object into the typeless data payload and pass it deeper; calls coap_show_pdu(LOG_DEBUG, ...)]
                            -> ...
                -> **coap_wait_ack(context->sendqueue_head->session, delayed_node)** [If the response was sent and it was a CON one, adds the the node representing PDU to the context's sendqueue]
            -> **coap_delete_node(context->sendqueue_head)** [Delete the retansmission unit form the sendqueue]
    -> **coap_read(context, now)** [Reads data from all sockets marked as COAP_SOCKET_CAN_READ]
        -> **coap_read_endpoint(endpoint, now)** [Reads data received by the underlaying socket and handles the response, if needed. Uses context's network_read handler (Default: coap_network_read(...))]
            -> **coap_network_read(endpoint->sock, packet)** [Reads data using recv() or recvfrom() system-call]
            -> **coap_endpoint_get_session(endpoint, &packet, now)** [Returns an endpoint's session associated with the packet or creates a new one, if not found]
                -> **coap_make_session(COAP_SESSION_TYPE_SERVER, NULL, &packet->dst, &packet->src, packet->ifindex, endpoint->context, endpoint)** [Creates a new session for the endpoint]
            -> **coap_handle_dgram(session, packet.payload, packet.length)** [Parses a received datagram and passes it to the coap_dispatch(...)]
                -> **coap_dispatch(session, pdu)** [Performs an appropriate steps depending on the message type and code. Sends response if needed]
                    -> **coap_session_connected(session)** [If the message was discarded (due to retransmission limit met) the session->con_active counter is decremented. Then, the session can open a new CON connection and so potentially flush a PDU that has been delayed to the session's delayqueue. Call to this function iterates over the delayqueue and ]
                        -> ...
                    -> **coap_touch_observer(session, &sent_token)** [Marks an observer related to the (session, token) pair in case the received message was an ACK to the notification]
                    -> **coap_cancel(session->context, sent_message)** [Deletes every relationship related to the peer who sent a RST (i.e. observations, sessions, entries in the sendqueue)]
                    -> **coap_send(session, response)** [Starts the process of response sending]
                        -> ...
                    -> **coap_send_message_type(session, pdu, COAP_MESSAGE_RST)** [Sends RST response for the given message]
                        -> **coap_send(session, response)** [Starts the process of response sending]
                            -> ...
        -> **coap_read_session(session, now)** [Receives data from the session's socket]
            -> **coap_network_read(session->sock, packet)** [Reads data using recv() or recvfrom() system-call]
            -> **coap_handle_dgram(session, packet.payload, packet.length)** [Parses a received datagram and passes it to the coap_dispatch(...)]
                -> ...