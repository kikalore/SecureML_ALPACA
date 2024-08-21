#include <stdarg.h>
#include <string.h>
#include <msp430.h>
#include <stdio.h>
#include <constants.h>
#include <libalpaca/alpaca.h>

__nv uint16_t scratch_bak[SCRATCH_SIZE];

/**
 * @brief dirtylist to save src address
 */
__nv uint8_t **data_src_base = &data_src;
/**
 * @brief dirtylist to save dst address
 */
__nv uint8_t **data_dest_base = &data_dest;
/**
 * @brief dirtylist to save size
 */
__nv unsigned *data_size_base = &data_size;

/**
 * @brief var to iterate over dirtylist
 */
__nv volatile unsigned gv_index = 0;
/**
 * @brief len of dirtylist
 */
__nv volatile unsigned num_dirty_gv = 0;

/**
 * @brief double buffered context
 */
__nv context_t context_1 = { 0 };
/**
 * @brief double buffered context
 */
__nv context_t context_0 = { .task = TASK_REF(_entry_task), .needCommit = 0, };
/**
 * @brief current context
 */
__nv context_t *volatile curctx = &context_0;
/**
 * @brief current version which updates at every reboot or transition
 */
__nv volatile unsigned _numBoots = 0;

__nv unsigned int overflow_counter=0;


/**
 * @brief Function to be invoked at the beginning of every task
 */
void task_prologue()
{
    // increment version
    if (_numBoots == 0xFFFF)
    {
        clear_isDirty();
        ++_numBoots;
    }
    ++_numBoots;
    // commit if needed
    if (curctx->needCommit)
    {
        while (gv_index < num_dirty_gv)
        {
            uint8_t *w_data_dest = *(data_dest_base + gv_index);
            uint8_t *w_data_src = *(data_src_base + gv_index);
            unsigned w_data_size = *(data_size_base + gv_index);
            memcpy(w_data_dest, w_data_src, w_data_size);
            ++gv_index;
        }
        num_dirty_gv = 0;
        gv_index = 0;
        curctx->needCommit = 0;
    }
    else
    {
        num_dirty_gv = 0;
    }
}

/**
 * @brief Transfer control to the given task
 * @details Finalize the current task and jump to the given task.
 *          This function does not return.
 *
 */
void transition_to(task_t *next_task)
{
    // double-buffered update to deal with power failure
    context_t *next_ctx;
    next_ctx = (curctx == &context_0 ? &context_1 : &context_0);
    next_ctx->task = next_task;
    next_ctx->needCommit = 1;

    // atomic update of curctx
    curctx = next_ctx;

    // fire task prologue
    task_prologue();
    // jump to next tast
    __asm__ volatile ( // volatile because output operands unused by C
            "mov #0x2400, r1\n"
            "br %[ntask]\n"
            :
            : [ntask] "r" (next_task->func)
    );
}

/**
 * @brief save variable data to dirtylist
 *
 */
void write_to_gbuf(uint8_t *data_src, uint8_t *data_dest, size_t var_size)
{
    // save to dirtylist
    *(data_size_base + num_dirty_gv) = var_size;
    *(data_dest_base + num_dirty_gv) = data_dest;
    *(data_src_base + num_dirty_gv) = data_src;
    // increment count
    num_dirty_gv++;
}

//Setup Timer to measure execution time
//void setup_timerB()
//{
//    //setup ports->P1.0 to an output
//    // P1DIR |= BIT0;
//    // P1OUT &= ~BIT0;
//    //PM5CTL0 &= ~LOCKLPM5;        // Already done in init_hw()
//
//    //setup timer
//    TB0CTL |= TBCLR;               // Reset timer
//    TB0CTL |= TBSSEL__ACLK;        // Clock source ACLK
//    //TB0CTL |= MC__CONTINUOUS;    // Continuous mode
//    TB0CTL |= MC__UP;              // Up mode, count to TB0CCR0
//    TB0CCR0 = 1638;                // Set timer period for 0.05 seconds
//
//    //TB0CTL |= TBIE;              // Setup TB0 overflow IRQ
//    TB0CCTL0 |= CCIE;              // Enable interrupt for CCR0
//    //__enable_interrupt();
//    TB0CTL &= ~TBIFG;
//}
//
//// Interrupt service routine for Timer B CCR0
//#pragma vector = TIMER0_B0_VECTOR
//__interrupt void ISR_TB0_CCR0(void)
//{
//    //P1OUT ^= BIT0;                 // Toggle LED
//    overflow_counter++;            // Increment overflow counter
//}

/** @brief Entry point upon reboot */
int main()
{
//    setvbuf(stdout, NULL, _IONBF, 0);
    _BIS_SR(GIE);
    init();

    // check for update
    task_prologue();

    // jump to curctx
    __asm__ volatile ( // volatile because output operands unused by C
            "br %[nt]\n"
            : /* no outputs */
            : [nt] "r" (curctx->task->func)
    );
    return 0;
}

