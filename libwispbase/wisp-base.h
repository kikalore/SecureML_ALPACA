/**
 * @file wisp-base.h
 *
 * The interface for the standard WISP library, including all definitions
 *  required to communicate with an RFID reader and use onboard peripherals.
 *
 * @author Aaron Parks, Saman Naderiparizi
 */

#ifndef WISP_BASE_H_
#define WISP_BASE_H_

//#include <msp430.h>
#include "libwispbase/globals.h" // Get these outta here (breaks encapsulation barrier)
#include "libwispbase/spi.h"
#include "libwispbase/uart.h"
#include "libwispbase/accel.h"
#include "libwispbase/fram.h"
#include "libwispbase/rfid.h"
#include "libwispbase/wispGuts.h"
#include "libwispbase/timer.h"
#include "libwispbase/rand.h"

void WISP_init(void);
void WISP_getDataBuffers(WISP_dataStructInterface_t* clientStruct);

#endif /* WISP_BASE_H_ */
