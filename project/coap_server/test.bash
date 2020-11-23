coap-client -m get coap://192.168.0.143:5683/.well-known/core
coap-client -m get coap://192.168.0.143:5683/time
coap-client -m get coap://192.168.0.143:5683/colour
echo -n "20 276 -30" | coap-client -m put coap://192.168.0.143:5683/colour -f-
coap-client -m get coap://192.168.0.143:5683/colour