#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

// Configuration Macros
#define HOST_IP_ADDR "192.168.1.100"  // Target Server IP
#define PORT         4210              // Target Server Port
#define TX_INTERVAL  2000              // Transmission interval in ms

static const char *TAG = "udp_client";
static const char *payload = "Telemetry payload from ESP-IDF";

static void udp_client_task(void *pvParameters) {
    char rx_buffer[128];
    char host_ip[] = HOST_IP_ADDR;
    int addr_family = AF_INET;
    int ip_protocol = IPPROTO_IP;

    while (1) {
        // Configure target address structure
        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(host_ip);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT);

        // 1. Create Socket
        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            continue;
        }

        // 2. Configure Socket to be Non-Blocking for bidirectional loop
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 50000; // 50ms timeout
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        ESP_LOGI(TAG, "Socket created, sending to %s:%d", host_ip, PORT);

        TickType_t last_send_time = xTaskGetTickCount();

        while (1) {
            // 3. Periodic Send
            if ((xTaskGetTickCount() - last_send_time) >= pdMS_TO_TICKS(TX_INTERVAL)) {
                last_send_time = xTaskGetTickCount();
                
                int err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
                if (err < 0) {
                    ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                    break; // Break loop to recreate socket
                }
                ESP_LOGI(TAG, ">>> Message sent");
            }

            // 4. Non-blocking Receive Check
            struct sockaddr_storage source_addr; 
            socklen_t socklen = sizeof(source_addr);
            
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

            if (len < 0) {
                // If it timed out, just loop back and continue execution
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    vTaskDelay(pdMS_TO_TICKS(10)); // Prevent CPU hogging
                    continue;
                }
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                break;
            } else {
                // Data received successfully
                rx_buffer[len] = 0; // Null-terminate
                ESP_LOGI(TAG, "<<< Received %d bytes: %s", len, rx_buffer);
            }
        }

        // Clean up socket on failure before retrying
        if (sock != -1) {
            ESP_LOGE(TAG, "Shutting down socket and recreating...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}

// Call this function inside your app_main() after Wi-Fi is connected
void start_udp_client(void) {
    xTaskCreate(udp_client_task, "udp_client", 4096, NULL, 5, NULL);
}