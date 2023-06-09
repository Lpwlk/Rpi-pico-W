// #include <stdio.h>
// #include "pico/stdlib.h"
// #include "pico/cyw43_arch.h"

// int main() {
//     stdio_init_all();
//     while (true) {
        
//         printf("LED on\n");
//         cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
//         sleep_ms(2500);
//         printf("LED off\n");
//         cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
//         sleep_ms(2500);

//     }
//     return 0;
// }


// #include "pico/stdlib.h"
// #include "pico/cyw43_arch.h"

// int main() {
//     stdio_init_all();
//     if (cyw43_arch_init()) {
//         printf("Wi-Fi init failed");
//         return -1;
//     }
//     while (true) {
//         // cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
//         printf("ON");
//         // sleep_ms(250);
//         // cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
//         printf("OFF");
//         // sleep_ms(250);
//     }
// }



// #include <stdio.h>
// #include "pico/stdlib.h"
// #include "hardware/adc.h"

// /* Choose 'C' for Celsius or 'F' for Fahrenheit. */
// #define TEMPERATURE_UNITS 'C'

// /* References for this implementation:
//  * raspberry-pi-pico-c-sdk.pdf, Section '4.1.1. hardware_adc'
//  * pico-examples/adc/adc_console/adc_console.c */
// float read_onboard_temperature(const char unit) {
    
//     /* 12-bit conversion, assume max value == ADC_VREF == 3.3 V */
//     const float conversionFactor = 3.3f / (1 << 12);

//     float adc = (float)adc_read() * conversionFactor;
//     float tempC = 27.0f - (adc - 0.706f) / 0.001721f;

//     if (unit == 'C') {
//         return tempC;
//     } else if (unit == 'F') {
//         return tempC * 9 / 5 + 32;
//     }

//     return -1.0f;
// }

// int main() {
//     stdio_init_all();

//     /* Initialize hardware AD converter, enable onboard temperature sensor and
//      *   select its channel (do this once for efficiency, but beware that this
//      *   is a global operation). */
//     adc_init();
//     adc_set_temp_sensor_enabled(true);
//     adc_select_input(4);

//     while (true) {
//         float temperature = read_onboard_temperature(TEMPERATURE_UNITS);
//         // printf("Onboard temperature = %.02f %c\n", temperature, TEMPERATURE_UNITS);
//         printf("%.02f;", temperature);
//         sleep_ms(10);
//     }

//     return 0;
// }



// #include <stdio.h> 
// #include "pico/stdlib.h"
// #include "pico/cyw43_arch.h"

// // char ssid[] = "Livebox-de-ouf";
// // char pass[] = "Motdepasse123";

// char ssid[] = "iPhone";
// char pass[] = "shareconnection";

// int main() {
//     stdio_init_all();

//     // if (cyw43_arch_init_with_country(CYW43_COUNTRY_FRANCE)) {
//     //     printf("failed to initialise\n");
//     //     return 1;
//     // }

//     if (cyw43_arch_init()) {
//         printf("Wi-Fi init failed");
//         return -1;
//     }
//     printf("initialised\n");
//     cyw43_arch_enable_sta_mode();

//     if (cyw43_arch_wifi_connect_timeout_ms(ssid, pass, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
//         printf("failed to connect\n");
//         printf("%x", cyw43_arch_wifi_connect_timeout_ms(ssid, pass, CYW43_AUTH_WPA2_AES_PSK, 10000));
//         return 1;
//     }
//     printf("connected\n");
//     while (true) {
//         cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
//         sleep_ms(250);
//         cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
//         sleep_ms(250);
//     }
// }


/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

static int scan_result(void *env, const cyw43_ev_scan_result_t *result) {
    if (result) {
        printf("ssid: %-32s rssi: %4d chan: %3d mac: %02x:%02x:%02x:%02x:%02x:%02x sec: %u\n",
            result->ssid, result->rssi, result->channel,
            result->bssid[0], result->bssid[1], result->bssid[2], result->bssid[3], result->bssid[4], result->bssid[5],
            result->auth_mode);
    }
    return 0;
}

#include "hardware/vreg.h"
#include "hardware/clocks.h"

int main() {
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();

    absolute_time_t scan_time = nil_time;
    bool scan_in_progress = false;
    while(true) {
        if (absolute_time_diff_us(get_absolute_time(), scan_time) < 0) {
            if (!scan_in_progress) {
                cyw43_wifi_scan_options_t scan_options = {0};
                int err = cyw43_wifi_scan(&cyw43_state, &scan_options, NULL, scan_result);
                if (err == 0) {
                    printf("\nPerforming wifi scan\n");
                    scan_in_progress = true;
                } else {
                    printf("Failed to start scan: %d\n", err);
                    scan_time = make_timeout_time_ms(10000); // wait 10s and scan again
                }
            } else if (!cyw43_wifi_scan_active(&cyw43_state)) {
                scan_time = make_timeout_time_ms(10000); // wait 10s and scan again
                scan_in_progress = false; 
            }
        }
        // the following #ifdef is only here so this same example can be used in multiple modes;
        // you do not need it in your code
#if PICO_CYW43_ARCH_POLL
        // if you are using pico_cyw43_arch_poll, then you must poll periodically from your
        // main loop (not from a timer) to check for Wi-Fi driver or lwIP work that needs to be done.
        cyw43_arch_poll();
        // you can poll as often as you like, however if you have nothing else to do you can
        // choose to sleep until either a specified time, or cyw43_arch_poll() has work to do:
        cyw43_arch_wait_for_work_until(scan_time);
#else
        // if you are not using pico_cyw43_arch_poll, then WiFI driver and lwIP work
        // is done via interrupt in the background. This sleep is just an example of some (blocking)
        // work you might be doing.
        sleep_ms(1000);
#endif
    }

    cyw43_arch_deinit();
    return 0;
}
