#!/bin/bash -xe

defines="-DCOMPONENT_4343W -DCOMPONENT_APP_CY8CPROTO_062_4343W -DCOMPONENT_CAT1 -DCOMPONENT_CAT1A -DCOMPONENT_CM0P_SLEEP -DCOMPONENT_CM4 -DCOMPONENT_CM4_0 -DCOMPONENT_Debug -DCOMPONENT_FREERTOS -DCOMPONENT_GCC_ARM -DCOMPONENT_HCI_UART -DCOMPONENT_LWIP -DCOMPONENT_MBEDTLS -DCOMPONENT_MURATA_1DX -DCOMPONENT_MW_ABSTRACTION_RTOS -DCOMPONENT_MW_CAT1CM0P -DCOMPONENT_MW_CLIB_SUPPORT -DCOMPONENT_MW_CMSIS -DCOMPONENT_MW_CONNECTIVITY_UTILITIES -DCOMPONENT_MW_CORE_LIB -DCOMPONENT_MW_CORE_MAKE -DCOMPONENT_MW_CY_MBEDTLS_ACCELERATION -DCOMPONENT_MW_DEVICE_DB -DCOMPONENT_MW_FREERTOS -DCOMPONENT_MW_LWIP -DCOMPONENT_MW_LWIP_FREERTOS_INTEGRATION -DCOMPONENT_MW_LWIP_NETWORK_INTERFACE_INTEGRATION -DCOMPONENT_MW_MBEDTLS -DCOMPONENT_MW_MTB_HAL_CAT1 -DCOMPONENT_MW_MTB_PDL_CAT1 -DCOMPONENT_MW_RECIPE_MAKE_CAT1A -DCOMPONENT_MW_RETARGET_IO -DCOMPONENT_MW_SECURE_SOCKETS -DCOMPONENT_MW_SERIAL_FLASH -DCOMPONENT_MW_WHD_BSP_INTEGRATION -DCOMPONENT_MW_WIFI_CONNECTION_MANAGER -DCOMPONENT_MW_WIFI_CORE_FREERTOS_LWIP_MBEDTLS -DCOMPONENT_MW_WIFI_HOST_DRIVER -DCOMPONENT_MW_WPA3_EXTERNAL_SUPPLICANT -DCOMPONENT_PSOC6_02 -DCOMPONENT_SECURE_SOCKETS -DCOMPONENT_SOFTFP -DCOMPONENT_WIFI_INTERFACE_SDIO -DCORE_NAME_CM4_0=1 -DCY8C624ABZI_S2D44 -DCYBSP_WIFI_CAPABLE -DCY_APPNAME_mtb_example_wifi_tcp_client -DCY_RETARGET_IO_CONVERT_LF_TO_CRLF -DCY_RTOS_AWARE -DCY_SUPPORTS_DEVICE_VALIDATION -DCY_TARGET_BOARD=APP_CY8CPROTO_062_4343W -DCY_USING_HAL -DDEBUG -DMBEDTLS_USER_CONFIG_FILE='\"mbedtls_user_config.h\"' -DTARGET_APP_CY8CPROTO_062_4343W"
includes=$(cat build/APP_CY8CPROTO-062-4343W/Debug/inclist.rsp)


bindgen --translate-enum-integer-types --use-core --allowlist-file ".*cy_wcm.h" -o cy_wcm.rs ../mtb_shared/wifi-connection-manager/release-v3.3.0/include/cy_wcm.h -- $defines $includes
bindgen --translate-enum-integer-types --use-core --allowlist-file ".*cy_http_client_api.h" -o cy_http_client_api.rs ../mtb_shared/http-client/latest-v1.X/include/cy_http_client_api.h -- $defines $includes
bindgen --translate-enum-integer-types --use-core --allowlist-file ".*MFRC522.h" -o mfrc522.rs source/MFRC522.h -- $defines $includes

# Add clippy lints to generated files
LINTING="#![allow(non_upper_case_globals, non_camel_case_types, non_snake_case, unused)]"
sed -i "1s/^/${LINTING}\n/" cy_wcm.rs
sed -i "1s/^/${LINTING}\n/" cy_http_client_api.rs
sed -i "1s/^/${LINTING}\n/" mfrc522.rs
