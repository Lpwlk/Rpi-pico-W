#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "pico/cyw43_arch.h"
// For hardware drive 
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/dma.h"


void printhelp(void) {
    puts("\nCommands:\n");
    puts("c0, ...\t: Select ADC channel n");
    puts("s\t: Sample few");
    puts("S\t: Sample many\n");
}

char serial_handler(void) {
    // printf("\n Type cmd to continue");
    char c = getchar();
    // printf(" >>> %c", c);
    return c;
}

void blink(int n) {
    int halfT = 10 * 25;
    for (int i = 0; i<n; i++) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1); sleep_ms(halfT);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0); sleep_ms(halfT);
    }
}
#define CLK_DIV_ADC 480 //960
#define CAPTURE_DEPTH 1000
uint8_t capture_buf[CAPTURE_DEPTH];

void dma_capture(int depth) {
    // printf("\n------------------\nArming DMA\n");
    // Set up the DMA to start transferring data as soon as it appears in FIFO
    uint dma_chan = dma_claim_unused_channel(true);
    dma_channel_config cfg = dma_channel_get_default_config(dma_chan);
    // Reading from constant address, writing to incrementing byte addresses
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_8);
    channel_config_set_read_increment(&cfg, false);
    channel_config_set_write_increment(&cfg, true);
    // Pace transfers based on availability of ADC samples
    channel_config_set_dreq(&cfg, DREQ_ADC);
    adc_run(true);
    dma_channel_configure(dma_chan, &cfg,
        capture_buf,    // dst
        &adc_hw->fifo,  // src
        depth,  // transfer count
        true            // start immediately
    );

    // printf("Starting capture\n");
    
    // Once DMA finishes, stop any new conversions from starting, and clean up
    // the FIFO in case the ADC was still mid-conversion.
    dma_channel_wait_for_finish_blocking(dma_chan);
    // printf("Capture finished\n");
    adc_run(false);
    adc_fifo_drain();
    // Print samples to stdout so you can display them in pyplot, excel, matlab
    for (int i = 0; i < CAPTURE_DEPTH; ++i) {
        printf("%-3d", capture_buf[i]);
        printf(";");
    }
}


int main() {
    stdio_init_all();           // inits standard SDK env for Flash on Rpi PicoW
    if (cyw43_arch_init()) {    // inits cyw43 wifi SoC architecture
        printf("Wi-Fi init failed");
        return -1;
    }
    uint8_t i = 0; 
    while(i<4) {                // inits GPIO for analogue use: hi-Z, no pulls, disable digital input buffer.
        adc_gpio_init(26+i);
        i+=1;
    }
    adc_set_temp_sensor_enabled(true);
    adc_init();                 // inits ADC mux unit for GPIO output measurments
    adc_fifo_setup(
        true,    // Write each completed conversion to the sample FIFO
        true,    // Enable DMA data request (DREQ)
        1,       // DREQ (and IRQ) asserted when at least 1 sample present
        false,   // We won't see the ERR bit because of 8 bit reads; disable.
        true     // Shift each sample to 8 bits when pushing to FIFO
    );
    // Divisor of 0 -> full speed. Free-running capture with the divider is
    // equivalent to pressing the ADC_CS_START_ONCE button once per `div + 1`
    // cycles (div not necessarily an integer). Each conversion takes 96
    // cycles, so in general you want a divider of 0 (hold down the button
    // continuously) or > 95 (take samples less frequently than 96 cycle
    // intervals). This is all timed by the 48 MHz ADC clock.
    adc_set_clkdiv(0);
    adc_select_input(0);
    blink(3);

    while (1) {
        char c = serial_handler();
        blink(1);
        switch(c) {
            case 's': {
                dma_capture(100);
                break;
            }
            case 'S': {
                for (int i=0;i<5;i++) {
                    dma_capture(1000);
                }
                break;
            }
            case '\n':
                break;
            case '\r':
                break;
            case 'h':
                // printhelp();
                break;
            // case 'c':
            //     c = getchar();
            //     printf("%c\n", c);
            //     if (c < '0' || c > '7') {
            //         printf("Unknown input channel\n");
            //         printhelp();
            //     } else {
            //         adc_select_input(c - '0');
            //         printf("Switched to channel %c\n", c);
            //     }
            //     break;
            default:
                // printf("\nUnrecognised command: %c\n", c);
                // printhelp();
                break;
        }
    }
}

// int main() {
//     stdio_init_all();
//     cyw43_arch_init();
//     uint32_t hexa = 0x0;
//     while (1) {
//         while(hexa <= 0xFFFF) {
//             hexa += 1;
//             printf("64-bit hex counter : %x  (%i ms elapsed since boot)\n", hexa, to_ms_since_boot(get_absolute_time()));
//         }
//         hexa = 0;
//     }
// }