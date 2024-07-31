/*
 * PF_sim.c
 *
 *  Created on: 18 Mar 2021
 *      Author: eren
 */
#include <msp430.h>
#include "mem.h"
#include <stdlib.h>
#include <stdio.h>
/**
 * main.c
 */
#define min_pf 10

__fram int timer_count = 65000;
__fram unsigned int count = 0;
//__fram unsigned int volatile random_num;  //read temperature value
//__fram int mod5_rand;

static void Reset()
{
    PMMCTL0 = PMMPW | PMMSWBOR; //PMMCTL0 =  0x0008; //Power-On Reset(POR)
}

static void rtc_init()
{
    PJSEL0 = BIT4 | BIT5;                   // Initialize LFXT pins
    CSCTL0_H = CSKEY_H;                     // Unlock CS registers
    CSCTL4 &= ~LFXTOFF;                     // Enable LFXT
    do
    {
        CSCTL5 &= ~LFXTOFFG;                  // Clear LFXT fault flag
        SFRIFG1 &= ~OFIFG;
    }
    while (SFRIFG1 & OFIFG);              // Test oscillator fault flag
    CSCTL0_H = 0;                           // Lock CS registers

    // Setup RTC Timer
    RTCCTL0_H = RTCKEY_H;                   // Unlock RTC

    RTCCTL0_L = RTCTEVIE_L;                 // RTC event interrupt enable
    RTCCTL13 = RTCSSEL_2 | RTCTEV_0 | RTCHOLD; // Counter Mode, RTC1PS, 8-bit ovf
    RTCPS0CTL = RT0PSDIV1;                  // ACLK, /8
    RTCPS1CTL = RT1SSEL1 | RT1PSDIV0 | RT1PSDIV1; // out from RT0PS, /16

    RTCCTL13 &= ~(RTCHOLD);                 // Start RTC
}

static void Timer_A0_set()
{

    TA0CCR0 = timer_count;                 //max 65535
    TA0CTL = TASSEL__ACLK | MC_1; //set the max period for 16bit timer operation
    TA0CCTL0 = CCIE;  //enable compare reg 0
    _BIS_SR(GIE); //ENABLE GLOBAL INTERRRUPTS

}
void PF_sim_start()
{
//    P3DIR ^= 0xFF;
//    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
//    PM5CTL0 &= ~LOCKLPM5;
    rtc_init();
    Timer_A0_set();
}

// Timer A0 interrupt service routine
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer_A(void)
{
    TA0CCTL0 &= ~CCIFG; // Clean interrupt flag
    P1OUT ^= BIT0;
//        random_num=RTCPS;
//        mod5_rand= random_num % 51;
//        random_num = min_pf + mod5_rand*8;
//        P3OUT ^=0x02;
//        P3OUT ^=0x02;
    printf("Timer expired\n");
    Reset();

}
