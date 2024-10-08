/* --COPYRIGHT--,FRAM-Utilities
 * Copyright (c) 2015, Texas Instruments Incorporated
 * All rights reserved.
 *
 * This source code is part of FRAM Utilities for MSP430 FRAM Microcontrollers.
 * Visit http://www.ti.com/tool/msp-fram-utilities for software information and
 * download.
 * --/COPYRIGHT--*/
#ifndef RNG_H_
#define RNG_H_

//******************************************************************************
//
//! \addtogroup rng_api
//! @{
//
//******************************************************************************

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

//******************************************************************************
//
//! \brief Key length for CTR-DRBG methodology used to generate random bytes.
//! This value also defines the minimum length of bytes that can be generated by
//! the rng_generateBytes function.
//
//******************************************************************************
#define RNG_KEYLEN              16

//*****************************************************************************
//
//! \brief  Generate random bytes and store to destination array.
//!
//! Generates the requested number of random bytes using the CTR-DRBG
//! methodology and the AES-128 block cipher algorithm according to Section
//! 9.3.1 and 10.2.1.5.1 of NIST SP 800-90Ar1. The length parameter must be a
//! multiple of RNG_KEYLEN, if it is not the length will be rounded down
//! to the closest multiple and that many bytes will be generated and returned.
//!
//! Note: Reseed, prediction resistance and additional inputs are not
//! supported.
//! Note: The security strength is fixed at 128-bit.
//!
//! \param dst Pointer to the destination array to store generated bytes.
//! \param length Number of bytes requested by the user, must be a multiple of
//!               RNG_KEYLEN bytes.
//!
//! \return Length of random bytes that were generated.
//
//*****************************************************************************
extern uint8_t rng_generateBytes(uint8_t *dst, uint8_t length);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

//******************************************************************************
//
// Close the Doxygen group.
//! @}
//
//******************************************************************************

#endif // RNG_H_
