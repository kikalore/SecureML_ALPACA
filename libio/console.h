#ifndef LIBIO_CONSOLE_H
#define LIBIO_CONSOLE_H

#include <stdio.h>

void console_init();

#if defined(LIBIO_BACKEND_HWUART)
#include <libio/console_hwuart.h>
#elif defined(LIBIO_BACKEND_SWUART)
#include <libio/console_swuart.h>
#elif  defined(LIBIO_BACKEND_EDB)
#include <libio/console_edb.h>
#else // no console

// All printfs fall back to nop
#define BLOCK_PRINTF_BEGIN()
#define BLOCK_PRINTF(...)
#define BLOCK_PRINTF_END()

#define PRINTF(...)

#define EIF_PRINTF(...)
#define BARE_PRINTF(...)

#endif // no printf

#if VERBOSE > 0

#define INIT_CONSOLE() INIT_CONSOLE_BACKEND()

#define BLOCK_LOG_BEGIN() BLOCK_PRINTF_BEGIN()
#define BLOCK_LOG(...)    BLOCK_PRINTF(__VA_ARGS__)
#define BLOCK_LOG_END()   BLOCK_PRINTF_END()

#if VERBOSE > 1
#define LOG PRINTF
#else
#define LOG(...)
#endif

#if VERBOSE >= 3
#define LOG2 PRINTF
#else // VERBOSE < 2
#define LOG2(...)
#endif // VERBOSE < 2

#else // !VERBOSE*

#define INIT_CONSOLE()

#define LOG(...)
#define LOG2(...)

#define BLOCK_LOG_BEGIN()
#define BLOCK_LOG(...)
#define BLOCK_LOG_END()

#endif // !VERBOSE*

#endif // LIBIO_CONSOLE_H
