#include "MFRC522.h"
#include "cy_http_client_api.h"
#include "cy_nw_helper.h"
#include "cy_result.h"
#include "cy_retarget_io.h"
#include "cy_secure_sockets.h"
#include "cy_wcm.h"
#include "cy_wcm_error.h"
#include "cyabs_rtos.h"
#include "cybsp.h"
#include "cyhal.h"
#include "cyhal_spi.h"
#include <inttypes.h>
#include <string.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

void spi() {
  cyhal_spi_t spi;
  {
    cy_rslt_t result = cyhal_spi_init(&spi, P9_0, P9_1, P9_2, P9_3, NULL, 8,
                                      CYHAL_SPI_MODE_00_MSB, false);
    if (result != CY_RSLT_SUCCESS) {
      printf("SPI initialization failed! Error code: 0x%08" PRIx32 "\n",
             (uint32_t)result);
      CY_ASSERT(0);
    }
  }

  {
    cy_rslt_t result = cyhal_spi_set_frequency(&spi, 8000000);
    if (result != CY_RSLT_SUCCESS) {
      printf("SPI set frequency failed! Error code: 0x%08" PRIx32 "\n",
             (uint32_t)result);
      CY_ASSERT(0);
    }
  }

  {
    cy_rslt_t result = cyhal_spi_send(&spi, 0x80 | 0x37 << 1);
    if (result != CY_RSLT_SUCCESS) {
      printf("SPI send failed! Error code: 0x%08" PRIx32 "\n",
             (uint32_t)result);
      CY_ASSERT(0);
    }
    uint32_t value = 0;
    result = cyhal_spi_recv(&spi, &value);
    if (result != CY_RSLT_SUCCESS) {
      printf("SPI receive failed! Error code: 0x%08" PRIx32 "\n",
             (uint32_t)result);
      CY_ASSERT(0);
    }
    printf("Value: %08lx\n", value);
  }

  cyhal_spi_free(&spi);
}

void mfrc(void) {
  mfrc522_t mfrc;
  MFRC522_Init(&mfrc, P9_0, P9_1, P9_2, P9_3, P9_4);
  printf("Version: %02x\n", PCD_ReadRegister(&mfrc, VersionReg));

  while (1) {
    bool result;
    PCD_Init(&mfrc);
    do {
      result = PICC_IsNewCardPresent(&mfrc);
    } while (result != true);
    do {
      result = PICC_ReadCardSerial(&mfrc);
    } while (result != true);

    printf("New Card: %02x %02x %02x %02x\n", mfrc.uid.uidByte[0],
           mfrc.uid.uidByte[1], mfrc.uid.uidByte[2], mfrc.uid.uidByte[3]);
  }
}

void http_task_init() {
  cy_wcm_config_t wifi_config = {.interface = CY_WCM_INTERFACE_TYPE_STA};
  cy_rslt_t result = cy_wcm_init(&wifi_config);

  if (result != CY_RSLT_SUCCESS) {
    printf("Wi-Fi Connection Manager initialization failed! Error code: "
           "0x%08" PRIx32 "\n",
           (uint32_t)result);
    CY_ASSERT(0);
  }
  printf("Wi-Fi Connection Manager initialized.\r\n");
}

void http_task_connect() {

  cy_wcm_connect_params_t wifi_conn_param;
  cy_wcm_ip_address_t ip_address;

  memset(&wifi_conn_param, 0, sizeof(cy_wcm_connect_params_t));
  memcpy(wifi_conn_param.ap_credentials.SSID, WIFI_SSID, sizeof(WIFI_SSID));
  memcpy(wifi_conn_param.ap_credentials.password, WIFI_PASSWORD,
         sizeof(WIFI_PASSWORD));
  wifi_conn_param.ap_credentials.security = CY_WCM_SECURITY_WPA2_AES_PSK;

  printf("Connecting to Wi-Fi Network: %s\n", WIFI_SSID);

  uint32_t conn_retries = 0;
  for (; conn_retries < 10; conn_retries++) {
    cy_rslt_t result = cy_wcm_connect_ap(&wifi_conn_param, &ip_address);

    if (result == CY_RSLT_SUCCESS) {
      printf("Successfully connected to Wi-Fi network '%s'.\n",
             wifi_conn_param.ap_credentials.SSID);
      printf("IP Address Assigned: %08lx\n", ip_address.ip.v4);
      break;
    }

    printf("Connection to Wi-Fi network failed with error code %d."
           "Retrying in %d ms...\n",
           (int)result, 1000);

    cy_rtos_delay_milliseconds(1000);
  }

  if (conn_retries == 10) {
    printf("Exceeded maximum Wi-Fi connection attempts\n");
    CY_ASSERT(0);
  }
}

void http_task(void *arg) {
  mfrc();
  return;

  http_task_init();
  http_task_connect();

  {
    cy_rslt_t result = cy_http_client_init();
    if (result != CY_RSLT_SUCCESS) {
      printf("\n Failed to initialize HTTP client! Error code: 0x%08" PRIx32
             "\n",
             (uint32_t)result);
      CY_ASSERT(0);
    }
  }

  cy_http_client_t client_handle;
  {
    cy_awsport_server_info_t server_info = {
        .host_name = "smi-server.stefan-hackenberg.de", .port = 80};
    cy_rslt_t result =
        cy_http_client_create(NULL, &server_info, NULL, NULL, &client_handle);
    if (result != CY_RSLT_SUCCESS) {
      printf("\n Failed to initialize HTTP client! Error code: 0x%08" PRIx32
             "\n",
             (uint32_t)result);
      CY_ASSERT(0);
    }
  }

  {
    cy_rslt_t result = cy_http_client_connect(client_handle, 1000, 1000);
    if (result != CY_RSLT_SUCCESS) {
      printf("\n Failed to connect to HTTP server! Error code: 0x%08" PRIx32
             "\n",
             (uint32_t)result);
      CY_ASSERT(0);
    }
  }

  uint8_t buffer[2048];

  {
    cy_http_client_request_header_t request = {.method =
                                                   CY_HTTP_CLIENT_METHOD_GET,
                                               .headers_len = 0,
                                               .buffer = buffer,
                                               .buffer_len = sizeof(buffer),
                                               .range_start = 0,
                                               .range_end = -1,
                                               .resource_path = "/hello"};

    cy_rslt_t result =
        cy_http_client_write_header(client_handle, &request, NULL, 0);
    if (result != CY_RSLT_SUCCESS) {
      printf("\n Failed to write HTTP header! Error code: 0x%08" PRIx32 "\n",
             (uint32_t)result);
      CY_ASSERT(0);
    }

    cy_http_client_response_t response;
    result = cy_http_client_send(client_handle, &request, NULL, 0, &response);
    if (result != CY_RSLT_SUCCESS) {
      printf("\n Failed to send HTTP request! Error code: 0x%08" PRIx32 "\n",
             (uint32_t)result);
      CY_ASSERT(0);
    }
    printf("Response: %d, %.*s\n", response.status_code, response.body_len,
           response.body);
  }
}
