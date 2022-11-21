/**
 * Copyright (c) 2021 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * ----------------------------------------------------------------------------------------------------
 * Includes
 * ----------------------------------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "port_common.h"

#include "wizchip_conf.h"
#include "w5x00_spi.h"

#include "dhcp.h"
#include "dns.h"
#include "timer.h"

#include "netif.h"

#include "azure_samples.h"
#include "proto_pi_CMD.h"

#include "hardware/rtc.h"
#include "hardware/gpio.h"
#include "timer.h"
#include "manage_process.h"

#include "pico/multicore.h"
#include "hardware/watchdog.h"

//#include "hardware/watchdog.h"


/**
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */
/* Clock */
#define PLL_SYS_KHZ (133 * 1000)

// The application you wish to use should be uncommented
//
// #define APP_TELEMETRY
// #define APP_C2D
// #define APP_CLI_X509
//#define APP_PROV_X509


//#define APP_PROV_X509

/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */
// The application you wish to use DHCP mode should be uncommented
#define _DHCP
static wiz_NetInfo g_net_info =
    {
        .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56}, // MAC address
        .ip = {192, 168, 11, 2},                     // IP address
        .sn = {255, 255, 255, 0},                    // Subnet Mask
        .gw = {192, 168, 11, 1},                     // Gateway
        .dns = {8, 8, 8, 8},                         // DNS server
#ifdef _DHCP
        .dhcp = NETINFO_DHCP // DHCP enable/disable
#else
        // this example uses static IP
        .dhcp = NETINFO_STATIC
#endif
};

/* Timer */
static uint16_t g_msec_cnt = 0;

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
/* Clock */
static void set_clock_khz(void);

/* Timer callback */
static void repeating_timer_callback(void);


void core1_entry() {
    while(1)
    {
        //g_cnt++;
        //printf("core1 cnt = %d\r\n", g_cnt);
        //sleep_ms(200);
        proto_uart_process();
    }
}
void CheckEthernet(void)
{
    uint8_t temp;
    int8_t networkip_setting = 0;
    switch(NetStatus)
    {
        case 0:
            if (ctlwizchip(CW_GET_PHYLINK, (void *)&temp) == -1)
            {
                printf(" Unknown PHY link status\n");
            }
            if(temp == PHY_LINK_ON)
            {
                NetStatus++;
                printf("network next[%d]\r\n", NetStatus);
            }
            break;
        case 1:
            // dhcp 
            watchdog_update();
#ifdef _DHCP
            // this example uses DHCP
            networkip_setting = wizchip_network_initialize(true, &g_net_info);
#else
            // this example uses static IP
            networkip_setting = wizchip_network_initialize(false, &g_net_info);
#endif
#if DEBUG_UDP_EN
            init_Debug_UDP();
            DebugSend("Debug Start", 11);
#endif
#if AZURE_EN
            if(networkip_setting)
                prov_dev_client_ll_Connect();
            else
                printf(" Check your network setting.\n");
            // Enable the watchdog, requiring the watchdog to be updated every 100ms or the chip will reboot
            // second arg is pause on debug which means the watchdog will pause when stepping through code
#endif
            NetStatus++;
            printf("network ready[%d]\r\n", NetStatus);

            break;
        default:
            break;
    }
}


/**
 * ----------------------------------------------------------------------------------------------------
 * Main
 * ----------------------------------------------------------------------------------------------------
 */
int main()
{
    //-----------------------------------------------------------------------------------
    // Pico board configuration - W5100S, GPIO, Timer Setting
    //-----------------------------------------------------------------------------------
    //int8_t networkip_setting = 0;

    datetime_t t = {
            .year  = 2022,
            .month = 05,
            .day   = 24,
            .dotw  = 5, // 0 is Sunday, so 5 is Friday
            .hour  = 11,
            .min   = 40,
            .sec   = 00
    };

    set_clock_khz();

    stdio_init_all();
    proto_uart_init();

    wizchip_delay_ms(1000 * 2); // wait for 2 seconds

    wizchip_spi_initialize();
    wizchip_cris_initialize();

    wizchip_reset();
    wizchip_initialize();
    wizchip_check();

    wizchip_1ms_timer_initialize(repeating_timer_callback);
    // Start the RTC
    rtc_init();
    rtc_set_datetime(&t);
    init_GPIO_IRQ();
    init_ADC();

    printf("hello pico arhis system \n");
#if 0
#ifdef _DHCP
    // this example uses DHCP
    networkip_setting = wizchip_network_initialize(true, &g_net_info);
#else
    // this example uses static IP
    networkip_setting = wizchip_network_initialize(false, &g_net_info);
#endif
    //W5100s_ping();

    #if DEBUG_UDP_EN
    init_Debug_UDP();
    DebugSend("Debug Start", 11);
    #endif
    #endif
    #if 1   //watchdog init
    sleep_ms(100);
    if(watchdog_caused_reboot()) {
        printf("Rebooted by Watchdog!\n");
        DebugSend("Rebooted by Watchdog!\r\n", strlen("Rebooted by Watchdog!\r\n"));
        //watchdog_reboot(0,0,0);
        //return 0;
    }else {
        printf("Clean boot\n");
        DebugSend("Clean boot\r\n", strlen("Clean boot\r\n"));
    }
    #endif
    Load_Flash_Internal_Data();
    //multicore_launch_core1(core1_entry);
    // bool cancelled = cancel_repeating_timer(&timer);
    // printf("cancelled... %d\n", cancelled);
    #if 0
    if (networkip_setting)
    {
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
// CALL Main Funcion - Azure IoT SDK example funcion
// Select one application.
//-----------------------------------------------------------------------------------
#ifdef APP_TELEMETRY
        iothub_ll_telemetry_sample();
#endif // APP_TELEMETRY
#ifdef APP_C2D
        iothub_ll_c2d_sample();
#endif // APP_C2D
#ifdef APP_CLI_X509
        iothub_ll_client_x509_sample();
#endif // APP_CLI_X509
#ifdef APP_PROV_X509
        prov_dev_client_ll_sample();
#endif // APP_PROV_X509
       //-----------------------------------------------------------------------------------
    }
    else
        printf(" Check your network setting.\n");
#endif
    /* Infinite loop */
    init_TIMER_IRQ();
    printf("start Loop %lld\r\n", time_us_64());
    #if 0
    #if AZURE_EN
    prov_dev_client_ll_Connect();
    // Enable the watchdog, requiring the watchdog to be updated every 100ms or the chip will reboot
    // second arg is pause on debug which means the watchdog will pause when stepping through code
    #endif
    #endif
    #if 1  //watchdog enable
    watchdog_enable(2000, 1);
    watchdog_update();
    #endif
    
    //flash_test();
    for (;;)
    {
        watchdog_update();
        CheckEthernet();
    #if AZURE_EN
        if(NetStatus > 1)
        {
            prov_dev_client_ll_Process();
        }
    #endif
        proto_uart_process();
        manage_process();
    #if 0
#ifdef _DHCP
        if (0 > wizchip_dhcp_run())
        {
            printf(" Stop Example.\n");

            while (1)
                ;
        }
#endif
        wizchip_delay_ms(1000); // wait for 1 second
#endif
    //printf("now :  %lld\r\n", time_us_64());

    //wizchip_delay_ms(1000); // wait for 1 second
    }
}

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
/* Clock */
static void set_clock_khz(void)
{
    // set a system clock frequency in khz
    set_sys_clock_khz(PLL_SYS_KHZ, true);

    // configure the specified clock
    clock_configure(
        clk_peri,
        0,                                                // No glitchless mux
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, // System PLL on AUX mux
        PLL_SYS_KHZ * 1000,                               // Input frequency
        PLL_SYS_KHZ * 1000                                // Output (must be same as no divider)
    );
}

/* Timer callback */
static void repeating_timer_callback(void)
{
    g_msec_cnt++;

#ifdef _DHCP
    if (g_msec_cnt >= 1000 - 1)
    {
        g_msec_cnt = 0;

        wizchip_dhcp_time_handler();
    }
#endif
}
