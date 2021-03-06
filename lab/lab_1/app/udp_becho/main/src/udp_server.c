#include "freertos/FreeRTOS.h" // FreeRTOS tasks
#include "freertos/task.h"     // xTaskNotifify API
#include "esp_log.h"           // Logging mechanisms
#include "lwip/sockets.h"      // LwIP stack

#define XOR_SWAP(a,b) \
 do                   \
 {                    \
   a ^= b;            \
   b ^= a;            \
   a ^= b;            \
 } while (0)

/* ---------------------------------- Configuration ---------------------------------- */

#define RX_BUFF_SIZE     255
#define UDP_SERVER_PORT 2392

/* --------------------------- Global & static definitions --------------------------- */

// Source file's tag
static char *TAG = "udp_server";

/* ------------------------------------- Declarations --------------------------------- */

// Global variables
extern TaskHandle_t main_handler;

/* ---------------------------------------- Code -------------------------------------- */

/**
 * @brief Udp server task.
 * 
 * @param pvParameters 
 */
void udp_server_task(void *pvParameters){

    // Wait for main to block on xTaskNotifyTake()
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Server duties
    while (true) {

        /* Create strcture describing BSD socket's address
         *     .sin_addr.s_addr : address of the socket (setting to 0 picks random
         *                        addres possessed by the server. ESP has only one
         *                        IP available at the time)
         *     .sin_family : protocoly type (AF_INET = IPv4)
         *     .sin_port : port number for th socket (setting to 0 picks random port)
         * 
         * @note : htonl() and htons() functions revert IP and PORT byte order to 
         * suite network order.
         */
        struct sockaddr_in destAddr;
        destAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(UDP_SERVER_PORT);

        // Create socket
        int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        } else
            ESP_LOGI(TAG, "Socket created");

        // Bind created socket to the IP address struct (destAddr)
        int err = bind(sock, (struct sockaddr *)&destAddr, sizeof(destAddr));
        if (err < 0) {
            ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        } else
            ESP_LOGI(TAG, "Socket binded");

        // Now, when socket is created and configured, we can start receiving data
        while(true) {

            ESP_LOGI(TAG, "Waiting for data");
            
            // Wait for message via UDP
            struct sockaddr_in sourceAddr;
            char rx_buffer[RX_BUFF_SIZE];
            socklen_t socklen = sizeof(sourceAddr);
            int len = recvfrom(
                sock,                            // Socket descriptor to read from
                rx_buffer,                       // Data buffer
                RX_BUFF_SIZE - 1,                // Size of the data buffer
                0,                               // Flags
                (struct sockaddr *)&sourceAddr,  // Socket  address that data came from
                &socklen                         // Length of the incoming socket's struct
            );

            // Error occured during receiving
            if (len < 0) {
                ESP_LOGE(TAG, "recvfrom() failed: errno %d", errno);
                break;
            }
            // Data received
            else {

                // Get the sender's ip address as string
                char addr_str[128];
                inet_ntoa_r(((struct sockaddr_in *)&sourceAddr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);

                // Null-terminate received data to it treat like a string
                rx_buffer[len] = 0; 

                // Print received data
                ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
                ESP_LOGI(TAG, "%s", rx_buffer);

                // Reverse order of the data
                char *start = rx_buffer;
                char *end = start + len - 1;
                while( start < end){
                    XOR_SWAP(*start, *end);
                    ++start;
                    --end;
                }

                ESP_LOGI(TAG, "Sending back: %s", rx_buffer);

                // Send reversed data to the client
                int err = sendto(
                    sock,                            // Socket descriptor to write to
                    rx_buffer,                       // Data buffer
                    len,                             // Size of the data buffer
                    0,                               // Flags
                    (struct sockaddr *)&sourceAddr,  // Destination address that data came from
                    sizeof(sourceAddr)               // Length of the destination socket's struct
                );
                if (err < 0) {
                    ESP_LOGE(TAG, "Error occured during sending: errno %d", errno);
                    break;
                }
            }
        }

        // If socket haven't been close yet, close now
        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }

    // Notify main to disconec from WiFi AP
    xTaskNotifyGive(main_handler);

    // Delete task itself
    vTaskDelete(NULL);
}