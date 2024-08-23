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
extern int overflow_counter;
// Global variables (task-shared)
GLOBAL_SB2(uint8_t, I[I_R*I_C]);
GLOBAL_SB2(uint8_t, W[I_C*W_C]);
GLOBAL_SB2(uint8_t,
           O[ROUND_UP_TO_MULTIPLE_OF_4(I_R)*ROUND_UP_TO_MULTIPLE_OF_4(W_C)]);
GLOBAL_SB2(encryptedMatrix, I_encrypted_ECB);
GLOBAL_SB2(encryptedMatrix, W_encrypted_ECB);
GLOBAL_SB2(encryptedMatrix, O_encrypted_ECB);
GLOBAL_SB2(uint8_t, I_encrypted_tile[TILE_SIZE]);
GLOBAL_SB2(uint8_t, I_decrypted_tile[TILE_SIZE]);
GLOBAL_SB2(uint8_t, W_encrypted_tile[TILE_SIZE]);
GLOBAL_SB2(uint8_t, W_decrypted_tile[TILE_SIZE]);
GLOBAL_SB2(uint8_t, OTile[TILE_SIZE]) =
{   0};
GLOBAL_SB2(uint8_t, row);
GLOBAL_SB2(uint8_t, col);
GLOBAL_SB2(uint8_t, blockRowI);
GLOBAL_SB2(uint8_t, blockColI);
GLOBAL_SB2(uint8_t, blockRowW);
GLOBAL_SB2(uint8_t, blockColW);
GLOBAL_SB2(uint8_t, w_row);
GLOBAL_SB2(uint8_t, w_col);
GLOBAL_SB2(uint8_t, output_row[ROUND_UP_TO_MULTIPLE_OF_4(W_C)]);
GLOBAL_SB2(uint8_t, index) = 0;

//define cipherKey of 32 bits
__nv uint16_t cipherKey[LENGTH] = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8,
                                    0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0x0, 0x1,
                                    0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA,
                                    0xB, 0xC, 0xD, 0xE, 0xF };
__nv uint8_t execution_counter = 0;
int i, j, k;

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
void task_encrypt_W();
void task_getTileI();
void task_getTileW();
void task_decrypt_I_tile();
void task_decrypt_W_tile();
void task_multiplicate_tile();
void task_accumulate_result();
void task_getOuputRow();
void task_encryptOutput();
void task_end();

//Defintion of tasks' order
TASK(1, task_init)
TASK(2, task_encrypt_I)
TASK(3, task_encrypt_W)
TASK(4, task_getTileI)
TASK(5, task_getTileW)
TASK(6, task_decrypt_I_tile)
TASK(7, task_decrypt_W_tile)
TASK(8, task_multiplicate_tile)
TASK(9, task_accumulate_result)
TASK(10, task_getOuputRow)
TASK(11, task_encryptOutput)
TASK(12, task_end)

static void init_hw()
{
    WDT_A_hold(WDT_A_BASE);
    __enable_interrupt();
    __bis_SR_register(GIE);     // Enable global interrupts
    PM5CTL0 &= ~LOCKLPM5; // Disable the GPIO power-on default high-impedance mode
}

// INIT FUNCTION->executes first on EVERY reboot, to reinitialize hw and interrupt handlers
void init()
{
    init_hw();
    PF_sim_start();
//    printf("Start time: %d\n", start_time);
//    printf("Init function successful\n");
}

// Initialization Task-> executed ONLY when the device boots for the first time (ENTRY_TASK)
void task_init()
{
//    printf("Executing entry task\n");
    TA0R = 0;
    overflow_counter = 0;
    // Initialize matrices I, W, and O
    Initialiaze_Matrix(GV(I), I_R, I_C);

    Initialiaze_Matrix(GV(W), I_C, W_C);

    // Set matrix O to zero
    memset(GV(O),
           0,
           GV(I_encrypted_ECB.matrixRows) * GV(W_encrypted_ECB.matrixCols)
                   * sizeof(uint8_t));

    // Initialize indices
    GV(row) = 0;
    GV(col) = 0;
    GV(blockRowI) = 0;
    GV(blockColI) = 0;
    GV(blockRowW) = 0;
    GV(blockColW) = 0;
    GV(w_row) = 0;
    GV(w_col) = 0;
    //printf("Entry successful\n");
    TRANSITION_TO(task_encrypt_I);
}

//Encrypt Input matrix
void task_encrypt_I()
{
//    printf("Executing encrypt_I task\n");
    unsigned int start_time_enc = TA0R;
    unsigned int overflow_enc_start=overflow_counter;
    GV(I_encrypted_ECB) = AES256_encryptMatrix_ECB(
            GV(I), GV(I_encrypted_ECB.matrix), I_R, I_C);
    unsigned int end_time_enc = TA0R;
    unsigned int elapsed_time_enc;
    unsigned int overflow_enc_end=overflow_counter;
    unsigned int overflow_enc=overflow_enc_end-overflow_enc_start;
    //printf("Start time: %d, End time: %d, Overflow_enc: %d\n",start_time_enc, end_time_enc, overflow_enc);
    if(start_time_enc>end_time_enc){
        elapsed_time_enc = (TA0CCR0 - start_time_enc) + end_time_enc;
    }else{
        elapsed_time_enc = end_time_enc - start_time_enc;
    }
    unsigned long total_ticks_enc = ((unsigned long) overflow_enc * TA0CCR0) + elapsed_time_enc;
    float time=(((float) total_ticks_enc / 32768.0) * 1000);
    printf("Time to encrypt: %d ms\n",(int)time);

    TRANSITION_TO(task_encrypt_W);
}
//Encrypt Weight matrix
void task_encrypt_W()
{
//    printf("Executing encrypt_W task\n");
    GV(W_encrypted_ECB) = AES256_encryptMatrix_ECB(
            GV(W), GV(W_encrypted_ECB.matrix), I_C, W_C);
    TRANSITION_TO(task_getTileI);
}

// Extract input encrypted tile
void task_getTileI()
{
    //printf("Executing task_getTileI\n");
    int sourceRow = GV(row) + GV(blockRowI);
    int sourceCol = GV(col) + GV(blockColI);
    if (sourceRow < GV(I_encrypted_ECB.matrixRows)
            && sourceCol < GV(I_encrypted_ECB.matrixCols))
    {
        GV(I_encrypted_tile[GV(blockRowI) * BLOCK_COLS + GV(blockColI)])= GV(I_encrypted_ECB.matrix[sourceRow * GV(I_encrypted_ECB.matrixCols) + sourceCol]);
    }

    //update blockColI and blockRowI indeces
    GV(blockColI)++;
    //printf("blockColI: %d\n", GV(blockColI));
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
        TRANSITION_TO(task_getTileI);
    }
    TRANSITION_TO(task_getTileI);
}
//Decrypt Input tile
void task_decrypt_I_tile()
{
    //printf("Executing task_decrypt_I_tile\n");
    AES256_decryptMatrix_ECB(GV(I_encrypted_tile), GV(I_decrypted_tile),
    BLOCK_SIZE,
                             sizeof(GV(I_decrypted_tile)));
    // printf("Decrypted matrix I:\n");
    // Print_Matrix(GV(I_decrypted_tile), BLOCK_ROWS, BLOCK_COLS);
    TRANSITION_TO(task_getTileW);
}
void task_getTileW()
{
    //printf("Executing task_getTileW\n");
    int sourceRow = GV(w_row) + GV(blockRowW);
    int sourceCol = GV(w_col) + GV(blockColW);
    if (sourceRow < GV(W_encrypted_ECB.matrixRows)
            && sourceCol < GV(W_encrypted_ECB.matrixCols))
    {
        GV(W_encrypted_tile[GV(blockRowW) * BLOCK_COLS + GV(blockColW)])= GV(W_encrypted_ECB.matrix[sourceRow * GV(W_encrypted_ECB.matrixCols) + sourceCol]);
    }

    //update blockColW and blockRowW indeces
    GV(blockColW)++;
    if (GV(blockColW) >= BLOCK_COLS)
    {
        GV(blockColW) = 0;
        GV(blockRowW)++;
        if (GV(blockRowW) >= BLOCK_ROWS)
        {
            GV(blockRowW) = 0;
            // Block obtained, transition to the task for decrypt it
            TRANSITION_TO(task_decrypt_W_tile);
        }
        TRANSITION_TO(task_getTileW);
    }
    TRANSITION_TO(task_getTileW);
}
void task_decrypt_W_tile()
{
    //printf("Executing task_decrypt_W_tile\n");
    AES256_decryptMatrix_ECB(GV(W_encrypted_tile), GV(W_decrypted_tile),
    BLOCK_SIZE,
                             sizeof(GV(W_decrypted_tile)));
    // printf("Decrypted matrix W:\n");
    // Print_Matrix(GV(W_decrypted_tile), BLOCK_ROWS, BLOCK_COLS);
    TRANSITION_TO(task_multiplicate_tile);
}
void task_multiplicate_tile()
{
//    printf("Executing task_multiplicate_tile\n");
    memset(GV(OTile), 0, sizeof(GV(OTile)));
    // printf("I_decrypted_tile:\n");
    // Print_Matrix(GV(I_decrypted_tile), BLOCK_ROWS, BLOCK_COLS);
    // printf("W_decrypted_tile:\n");
    // Print_Matrix(GV(W_decrypted_tile), BLOCK_ROWS, BLOCK_COLS);

    // Perform multiplication and store result in OTile
    for (i = 0; i < BLOCK_ROWS; ++i)
    {
        for (j = 0; j < BLOCK_COLS; ++j)
        {
            for (k = 0; k < BLOCK_COLS; ++k)
            {
                GV(OTile[i * BLOCK_COLS + j])+= GV(I_decrypted_tile[i * BLOCK_COLS + k]) * GV(W_decrypted_tile[k * BLOCK_COLS + j]);
            }
        }
    }
    // printf("OTile:\n");
    // Print_Matrix(GV(OTile), BLOCK_ROWS, BLOCK_COLS);
    TRANSITION_TO(task_accumulate_result);
}
void task_accumulate_result()
{
//    printf("Executing task_accumulate_result\n");
    // Accumulate OTile into the output matrix O
    for (i = 0; i < BLOCK_ROWS; ++i)
    {
        for (j = 0; j < BLOCK_COLS; ++j)
        {
            int outputRow = GV(row) + i;
            int outputCol = GV(w_col) + j;
            if (outputRow
                    < GV(I_encrypted_ECB.matrixRows && outputCol < GV(W_encrypted_ECB.matrixCols)))
            {
                //set corresponding area of O to zero
                GV(O[outputRow * GV(W_encrypted_ECB.matrixCols) + outputCol])= 0;
                GV(O[outputRow * GV(W_encrypted_ECB.matrixCols) + outputCol])+= GV(OTile[i * BLOCK_COLS + j]);
            }
        }
    }
    // printf("Output matrix is:\n");
    // Print_Matrix(GV(O), GV(I_encrypted_ECB.matrixRows), GV(W_encrypted_ECB.matrixCols));
    //update  w_col indeces
    GV(w_col) += BLOCK_COLS;
    if (GV(w_col) >= GV(W_encrypted_ECB.matrixCols))
    {
        GV(w_col) = 0;
        TRANSITION_TO(task_getOuputRow);
    }
    TRANSITION_TO(task_getTileW);
}
void task_getOuputRow()
{
//    printf("Executing task_save_output_row\n");
    int outputrow = GV(row) + GV(index);
    //printf("outputrow: %d, GV(row): %d, GV(index): %d\n", outputrow, GV(row), GV(index));

    if (outputrow < GV(I_encrypted_ECB.matrixRows))
    {
        memcpy(GV(output_row),
               &GV(O[outputrow * GV(W_encrypted_ECB.matrixCols)]),
               GV(W_encrypted_ECB.matrixCols * sizeof(uint8_t)));
    }
    GV(index)++;
    if (GV(index) >= BLOCK_ROWS)
    {
        GV(index) = 0;
        GV(w_row) += BLOCK_ROWS;
        if (GV(w_row) >= GV(W_encrypted_ECB.matrixRows))
        {
            GV(w_row) = 0;
            //update col and row indeces
            GV(col) += BLOCK_COLS;
            if (GV(col) >= GV(I_encrypted_ECB.matrixCols))
            {
                GV(col) = 0;
                GV(row) += BLOCK_ROWS;
                if (GV(row) >= GV(I_encrypted_ECB.matrixRows))
                {
                    //all lines saved
                    TRANSITION_TO(task_encryptOutput);
                }
            }
            TRANSITION_TO(task_getTileI);
        }
        TRANSITION_TO(task_getTileW);
    }
    TRANSITION_TO(task_getOuputRow);
}

void task_encryptOutput()
{
//    printf("Executing encrypt_Output task\n");
    // printf("Plaintext output matrix is:\n");
    // Print_Matrix(GV(O), GV(I_encrypted_ECB.matrixRows),
    //              GV(W_encrypted_ECB.matrixCols));
    GV(O_encrypted_ECB) = AES256_encryptMatrix_ECB(
            GV(O), GV(O_encrypted_ECB.matrix), GV(I_encrypted_ECB.matrixRows),
            GV(W_encrypted_ECB.matrixCols));
    TRANSITION_TO(task_end);
}

// End Task
void task_end()
{
    free(GV(I_encrypted_ECB.matrix));
    free(GV(W_encrypted_ECB.matrix));
    free(GV(O_encrypted_ECB.matrix));

//    printf("Executing end task\n");
//    printf("Number of overflow: %d\n", overflow_counter);

//    unsigned int end_time = TA0R;
//    printf("End time: %d\n", end_time);

//    unsigned int elapsed_time;
//    elapsed_time = end_time - 0;
    //printf("Elapsed time: %u\n", elapsed_time);

    // Calculate total ticks
    unsigned long total_ticks = ((unsigned long) overflow_counter * TA0CCR0)
            + TA0R;
//    printf("Total ticks: %lu\n", total_ticks);

    // Convert ticks to seconds
    float time_in_seconds = ((float) total_ticks / 32768.0);
    printf("Total %d ms\n", (int) (time_in_seconds * 1000));

    TRANSITION_TO(task_init);
}

ENTRY_TASK(task_init)
INIT_FUNC(init)
