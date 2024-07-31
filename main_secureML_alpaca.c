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
GLOBAL_SB2(uint8_t, I[I_R][I_C]);
GLOBAL_SB2(uint8_t, W[I_C][W_C]);
GLOBAL_SB2(uint8_t, O[I_R][W_C]);
GLOBAL_SB2(encryptedMatrix, I_encrypted_ECB);
GLOBAL_SB2(encryptedMatrix, W_encrypted_ECB);
GLOBAL_SB2(uint8_t, I_encrypted_tile[TILE_SIZE]);
GLOBAL_SB2(uint8_t, I_decrypted_tile[TILE_SIZE]);
GLOBAL_SB2(uint8_t, W_encrypted_tile[TILE_SIZE]);
GLOBAL_SB2(uint8_t, W_decrypted_tile[TILE_SIZE]);
GLOBAL_SB2(uint8_t, Otile[TILE_SIZE])= {0};
GLOBAL_SB2(encryptedMatrix, O_encrypted);
GLOBAL_SB2(uint8_t, row);
GLOBAL_SB2(uint8_t, col);
GLOBAL_SB2(uint8_t, blockRowI);
GLOBAL_SB2(uint8_t, blockColI);
//define cipherKey of 32 bits
__nv uint16_t cipherKey[LENGTH]={0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF};
__nv int ii, jj, kk, i, j, k;
__nv uint8_t execution_counter = 0;

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
void task_encrypt_I();
void get_input_indeces();
void get_input_tile();
void task_decrypt_I_tile();
void task_end();

//Defintion of tasks' order
TASK(1, task_init)
TASK(2, task_encrypt_I)
TASK(3, get_input_indeces)
TASK(4, get_input_tile)
TASK(5, task_decrypt_I_tile)
TASK(6, task_end)

static void init_hw()
{
    WDT_A_hold(WDT_A_BASE);
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
    GV(row) = 0;
    GV(col) = 0;
    GV(blockRowI) = 0;
    GV(blockColI) = 0;
    printf("Entry successful\n");
    TRANSITION_TO(task_encrypt_I);
}

//encrypt Input matrix
void task_encrypt_I()
{
    printf("Executing encrypt_I task\n");
    GV(I_encrypted_ECB) = AES256_encryptMatrix_ECB(GV(I), GV(I_encrypted_ECB.matrix), I_R, I_C);
    TRANSITION_TO(get_input_indeces);
}
//encrypt Weight matrix
// void task_encrypt_W()
// {
//     printf("Executing encrypt_W task\n");
//     GV(W_encrypted_ECB) = AES256_encryptMatrix_ECB(GV(W), GV(W_encrypted_ECB.matrix), I_C, W_C);
//     printf("Encrypted matrix:\n");
//     Print_Matrix(GV(W_encrypted_ECB.matrix), GV(W_encrypted_ECB.matrixRows),
//                  GV(W_encrypted_ECB.matrixCols));
// }

// Extract indeces from input matrix
void get_input_indeces(){
            //first iteration or a new block should be extract
            if (GV(blockColI)==0 && GV(blockRowI)==0) 
            {
                TRANSITION_TO(get_input_tile);
            }

            //update col and row indeces
            GV(col) += BLOCK_COLS;
            printf("in get input indeces col=%d\n", GV(col));
            if (GV(col) >= GV(I_encrypted_ECB.matrixCols))
            {
                GV(col) = 0;
                GV(row) += BLOCK_ROWS;
                if (GV(row) >= GV(I_encrypted_ECB.matrixRows))
                {
                    // All tiles processed, transition to the aggregation task
                    TRANSITION_TO(task_end);
                }
            }
            TRANSITION_TO(get_input_indeces);
    //      }
    // }
}
// Extract input encrypted tile
void get_input_tile(){
    printf("Executing get_input_tile task\n");
    size_t sourceRow = GV(row) + GV(blockRowI);
    size_t sourceCol = GV(col) + GV(blockColI);
    if (sourceRow < GV(I_encrypted_ECB.matrixRows) && sourceCol < GV(I_encrypted_ECB.matrixCols))
    {
        GV(I_encrypted_tile[GV(blockRowI) * BLOCK_COLS + GV(blockColI)]) = GV(I_encrypted_ECB.matrix[sourceRow * GV(I_encrypted_ECB.matrixCols) + sourceCol]);        
    }
    //update blockColI and blockRowI indeces
    GV(blockColI)++;
    if (GV(blockColI) >= BLOCK_COLS)
    {
        GV(blockColI) = 0;
        GV(blockRowI)++;
        if (GV(blockRowI) >= BLOCK_ROWS)
        {
            GV(blockRowI) = 0;
            // Block obtained, transition to the task for decrypt it
            TRANSITION_TO(task_decrypt_I_tile);
        }
        TRANSITION_TO(get_input_tile);
    }
    TRANSITION_TO(get_input_tile);


}
//Decrypt Input tile
void task_decrypt_I_tile()
{
    printf("Executing decrypt_I_tile task\n");
    AES256_decryptMatrix_ECB(GV(I_encrypted_tile), GV(I_decrypted_tile), BLOCK_SIZE,
                             sizeof(GV(I_decrypted_tile)));
    printf("Decrypted matrix:\n");
    Print_Matrix(GV(I_decrypted_tile), BLOCK_ROWS, BLOCK_COLS);
    TRANSITION_TO(task_end);
}
// End Task
void task_end()
{
    printf("Executing end task\n");
    TRANSITION_TO(task_init);
}

ENTRY_TASK(task_init)
INIT_FUNC(init)
