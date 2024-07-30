#include <msp430.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <libalpaca/alpaca.h>
#include <libmsp/mem.h>
#include <libmsp/watchdog.h>
#include <libPF/PF_sim.h>
#include "matrix_multiplication.h"
#include "AESoperations.h"
#include "constants.h"

// Global variables (task-shared)
GLOBAL_SB(uint8_t, I[I_R][I_C]);
GLOBAL_SB(uint8_t, W[I_C][W_C]);
GLOBAL_SB(uint8_t, O[I_R][W_C]);
__nv int ii, jj, kk, i, j, k;
__nv uint8_t execution_counter = 0;
GLOBAL_SB(uint16_t cipherkey[LENGTH]) = { 0 };

#define MEM_SIZE 0x4
__nv uint8_t *data_src[MEM_SIZE];
__nv uint8_t *data_dest[MEM_SIZE];
__nv unsigned int data_size[MEM_SIZE];
__nv uint8_t exacution_counter = 0;
void clear_isDirty()
{
}

//Declaration of tasks
void init();
void task_init();
void tile_process();
void aggregate_results();
//void task_end();

//Defintion of tasks' order
TASK(1, task_init)
TASK(2, tile_process)
TASK(3, aggregate_results)
//TASK(4, task_end)

static void init_hw()
{
    msp_watchdog_disable();
    __enable_interrupt();
    PM5CTL0 &= ~LOCKLPM5; // Disable the GPIO power-on default high-impedance mode
    P1DIR |= BIT0;             // Set P1.0 to output direction
    P1OUT |= BIT0;             // Set the LED on initially

}

// INIT FUNCTION->executes first on EVERY reboot, to reinitialize hw and interrupt handlers
void init()
{
    init_hw();
    PF_sim_start();
    printf("Init function successful\n");
}

// Initialization Task-> executed ONLY when the device boots for the first time (ENTRY_TASK)
void task_init()
{
    printf("Executing entry task\n");

    // Initialize matrices I, W, and O
    printf("Init matrix I\n");
    Initialiaze_Matrix(GV(I), I_R, I_C);

    printf("Init matrix W\n");
    Initialiaze_Matrix(GV(W), I_C, W_C);

    // Initialize O to zero
    for (int i = 0; i < I_R; i++)
    {
        for (int j = 0; j < W_C; j++)
        {
            GV(O[i][j]) = 0;
        }
    }
    // Initialize indices
    ii = jj = kk = 0;
    printf("Entry successful\n");
    TRANSITION_TO(tile_process);
}

// Tile Processing Task
void tile_process()
{
    printf("Executing tile_process task\n");

    // Perform multiplication for the current tile
    for (i = ii; i < ii + TILE_SIZE && i < I_R; ++i)
    {
        for (j = jj; j < jj + TILE_SIZE && j < W_C; ++j)
        {
            for (k = kk; k < kk + TILE_SIZE && k < I_C; ++k)
            {
                GV(O[i][j]) += GV(I[i][k]) * GV(W[k][j]);
            }
        }
    }

    // Update tile indices
    kk += TILE_SIZE;
    if (kk >= I_C)
    {
        kk = 0;
        jj += TILE_SIZE;
        if (jj >= W_C)
        {
            jj = 0;
            ii += TILE_SIZE;
            if (ii >= I_R)
            {
                //Print_Matrix(GV(O),I_R,W_C);
                // All tiles processed, transition to the aggregation task
                TRANSITION_TO(aggregate_results);
                return;
            }
        }
    }
    TRANSITION_TO(tile_process);
}
//

// Aggregation Task
void aggregate_results()
{
    printf("Executing aggregate_results task\n");
    TRANSITION_TO(task_init);
}

// void task_end() {
//     execution_counter++;
// 	TRANSITION_TO(task_init);
// }

ENTRY_TASK(task_init)
INIT_FUNC(init)
