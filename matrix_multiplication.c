#include "driverlib.h"
#include <stdio.h>
#include <math.h>
#include "matrix_multiplication.h"
#include "AESoperations.h"

extern uint8_t cipherkey[];

void Basic_Multiplication(uint8_t I[I_R][I_C], uint8_t W[I_C][W_C],
                          uint8_t O[I_R][W_C])
{
    unsigned int i, j, k;
    for (i = 0; i < I_R; ++i)
    {
        for (j = 0; j < W_C; ++j)
        {
            O[i][j] = 0;
            for (k = 0; k < I_C; ++k)
            {
                O[i][j] += I[i][k] * W[k][j];
            }
        }
    }
}

void Print_Matrix(uint8_t *matrix, int rows, int cols)
{
    int i, j;
    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < cols; j++)
        {
            printf("%d ", matrix[i * cols + j]);
        }
        printf("\n");
    }
}

void Initialiaze_Matrix(uint8_t *matrix, int rows, int cols)
{
    int i, j, val = 0;
    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < cols; j++)
        {
            matrix[i * cols + j] = val++;
        }
    }
}

bool Matrix_Equality(uint8_t *matrix1, uint8_t *matrix2, int rows, int cols)
{
    int i, j;
    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < cols; j++)
        {
            if (matrix1[i * cols + j] != matrix2[i * cols + j])
            {
                return false;
            }
        }
    }
    return true;
}

void Tiled_Multiplication(uint8_t I[I_R][I_C], uint8_t W[I_C][W_C],
                          uint8_t O[I_R][W_C])
{
    int i, j, k, ii, jj, kk;

    // Outer loops iterate over tiles
    for (ii = 0; ii < I_R; ii += TILE_SIZE)
    {
        for (jj = 0; jj < W_C; jj += TILE_SIZE)
        {
            for (kk = 0; kk < I_C; kk += TILE_SIZE)
            {
                // Inner loops perform multiplication for the current tile
                for (i = ii; i < ii + TILE_SIZE && i < I_R; ++i)
                {
                    for (j = jj; j < jj + TILE_SIZE && j < W_C; ++j)
                    {
                        for (k = kk; k < kk + TILE_SIZE && k < I_C; ++k)
                        {
                            O[i][j] += I[i][k] * W[k][j];
                        }
                    }
                }
            }
        }
    }
}

encryptedMatrix Tiled_Decryption_Multiplication(encryptedMatrix I_encrypted, encryptedMatrix W_encrypted,
                      uint8_t *O)
{

    // Temporary storage for decrypted tiles
    uint8_t I_decrypted_tile[TILE_SIZE];
    uint8_t W_decrypted_tile[TILE_SIZE];
    uint8_t I_encrypted_tile[TILE_SIZE];
    uint8_t W_encrypted_tile[TILE_SIZE];
    encryptedMatrix O_encrypted;
    uint8_t OTile[TILE_SIZE] = { 0 };
    //To store each row of the output matrix-->for example we can save into FRAM each row instead of the single tiled result (which are just partial results)
    uint8_t output_row[ROUND_UP_TO_MULTIPLE_OF_4(W_C)];
    int blockRow, blockCol, row, col, w_row, w_col, i, j, k;

    memset(O, 0, I_encrypted.matrixRows * W_encrypted.matrixCols * sizeof(uint8_t));

    // Always cycle over matrices that have rows and columns multiple of BLOCK_ROWS and BLOCK_COLS, so no paddiing is needed
    for ( row = 0; row < I_encrypted.matrixRows; row += BLOCK_ROWS)
    {
        for ( col = 0; col < I_encrypted.matrixCols; col += BLOCK_COLS)
        {
            // printf("Start index of tile of INPUT matrix is: (%d, %d)\n", row,
            //        col);

            // Copy current I_tile into I_encrypted_tile and decrypt it
            for (blockRow = 0; blockRow < BLOCK_ROWS; ++blockRow)
            {
                for (blockCol = 0; blockCol < BLOCK_COLS; ++blockCol)
                {
                    size_t sourceRow = row + blockRow;
                    size_t sourceCol = col + blockCol;
                    if (sourceRow < I_encrypted.matrixRows
                            && sourceCol < I_encrypted.matrixCols)
                    {
                        I_encrypted_tile[blockRow * BLOCK_COLS + blockCol] =
                                I_encrypted.matrix[sourceRow
                                        * I_encrypted.matrixCols + sourceCol];
                    }
                }
            }

            // printf("Encrypted tile I:\n");
            // Print_Matrix(I_encrypted_tile, BLOCK_ROWS, BLOCK_COLS);
            AES256_decryptMatrix_ECB(I_encrypted_tile, I_decrypted_tile,
                                     BLOCK_SIZE, BLOCK_SIZE);
            // printf("Decrypted tile I:\n");
            // Print_Matrix(I_decrypted_tile, BLOCK_ROWS, BLOCK_COLS);

            for (w_row = 0; w_row < W_encrypted.matrixRows; w_row += BLOCK_ROWS) {
                for (w_col = 0; w_col < W_encrypted.matrixCols; w_col += BLOCK_COLS) {
                    memset(OTile, 0, sizeof(OTile));
                    //printf("Start index of tile of WEIGHT matrix is: (%d, %d)\n", w_row, w_col);

                    // Copy current tile into W_encrypted_tile
                    memset(W_encrypted_tile, 0, sizeof(W_encrypted_tile));
                    for (blockRow = 0; blockRow < BLOCK_ROWS; ++blockRow) {
                        for (blockCol = 0; blockCol < BLOCK_COLS; ++blockCol) {
                            size_t sourceRow = w_row + blockRow;
                            size_t sourceCol = w_col + blockCol;
                            if (sourceRow < W_encrypted.matrixRows && sourceCol < W_encrypted.matrixCols) {
                                W_encrypted_tile[blockRow * BLOCK_COLS + blockCol] = W_encrypted.matrix[sourceRow * W_encrypted.matrixCols + sourceCol];
                            }
                        }
                    }
                    //printf("Encrypted tile W:\n");
                    //Print_Matrix(W_encrypted_tile, BLOCK_ROWS, BLOCK_COLS);
                    AES256_decryptMatrix_ECB(W_encrypted_tile, W_decrypted_tile, BLOCK_SIZE, sizeof(W_decrypted_tile));
                    // printf("Decrypted tile W:\n");
                    // Print_Matrix(W_decrypted_tile, BLOCK_ROWS, BLOCK_COLS);


                    // Perform multiplication and store result in OTile
                    for (i = 0; i < BLOCK_ROWS; ++i) {
                        for (j = 0; j < BLOCK_COLS; ++j) {
                            for (k = 0; k < BLOCK_COLS; ++k) {
                                OTile[i * BLOCK_COLS + j] += I_decrypted_tile[i * BLOCK_COLS + k] * W_decrypted_tile[k * BLOCK_COLS + j];
                            }
                        }
                    }
                    // Accumulate OTile into the output matrix O
                    for (i = 0; i < BLOCK_ROWS; ++i) {
                        for (j = 0; j < BLOCK_COLS; ++j) {
                            size_t outputRow = row + i;
                            size_t outputCol = w_col + j;
                            if (outputRow <  I_encrypted.matrixRows && outputCol < W_encrypted.matrixCols) {
                                O[outputRow * W_encrypted.matrixCols + outputCol] += OTile[i * BLOCK_COLS + j];
                            }
                        }
                    }
                }
                 // Save the output row into FRAM when it's ready. 
                for (i = 0; i < BLOCK_ROWS; ++i)
                {
                    size_t outputRow = row + i;
                    if (outputRow < I_encrypted.matrixRows)
                    {
                        memcpy(output_row, &O[outputRow * W_encrypted.matrixCols], W_encrypted.matrixCols * sizeof(uint8_t));
                        printf("Output row %d: ", outputRow);
                        for (j = 0; j < W_encrypted.matrixCols; ++j)
                        {
                            printf("%d ", output_row[j]);
                        }
                        printf("\n");
                    }
                }
            }
        }
    }
    printf("Output matrix is:\n");
    Print_Matrix(O, I_encrypted.matrixRows, W_encrypted.matrixCols);
    O_encrypted = AES256_encryptMatrix_ECB(O, O_encrypted.matrix, I_encrypted.matrixRows, W_encrypted.matrixCols);
    return O_encrypted;
}

