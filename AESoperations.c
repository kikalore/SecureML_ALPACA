#include "driverlib.h"
#include "AESoperations.h"

extern uint8_t cipherkey[];

void generateRandomKey(uint16_t *cipherKey)
{
    // Configure MPU with INFO memory as RW
    MPUCTL0 = MPUPW;
    MPUSAM |= MPUSEGIRE | MPUSEGIWE;
    MPUCTL0 = MPUPW | MPUENA;
    uint8_t len;
    len = rng_generateBytes(cipherKey, LENGTH);
    //check length correctness
    if (len != LENGTH)
    {
        printf("Error generating bytes\n");
        __no_operation();
    }
    else
    {
        printf("Key of %d bytes generated successfully\n", len);
        printf("Generated key is: ");
        int i;
        for (i = 0; i < LENGTH; ++i)
        {
            printf("%d", cipherKey[i]);
        }
        printf("\n");
    }
}

void encryptAndStoreInFRAM(const uint8_t *matrix,
                           uint8_t *encryptedMatrixFRAMFRAM,
                           uint16_t *cipherKey)
{
    AES256_setCipherKey(AES256_BASE, cipherKey, AES256_KEYLENGTH_128BIT);
    AES256_encryptData(AES256_BASE, matrix, encryptedMatrixFRAMFRAM);
}

void decryptAndStoreInSRAM(const uint8_t *encryptedMatrixFRAM,
                           uint8_t *decryptedMatrixSRAM, uint16_t *cipherKey)
{
    AES256_setDecipherKey(AES256_BASE, cipherKey, AES256_KEYLENGTH_128BIT);
    AES256_decryptData(AES256_BASE, encryptedMatrixFRAM, decryptedMatrixSRAM);
}

void AES256_decryptMatrix_ECB(uint8_t *encryptedMatrixFRAM,
                              uint8_t *decryptedMatrixSRAM,
                              size_t encryptedSize, size_t matrixSize)
{
    size_t i, j;
    uint8_t block[BLOCK_SIZE];
    uint8_t decryptedBlock[BLOCK_SIZE];
    size_t paddedEncryptedSize = encryptedSize;

    //print tile to decrypt
    // printf("Encrypted input:\t");
    // for (i = 0; i < encryptedSize; i++)
    // {
    //     printf("%d ", encryptedMatrixFRAM[i]);
    // }
    // printf("\n");

    // Iterate over the encrypted matrix in 16-byte blocks
    for (i = 0; i < encryptedSize; i += BLOCK_SIZE)
    {
        // Copy the next 16 bytes into the block
        if (i + BLOCK_SIZE <= encryptedSize)
        {
            memcpy(block, encryptedMatrixFRAM + i, BLOCK_SIZE);
            // printf("Block to decrypt:\t");
            // for (j = 0; j < BLOCK_SIZE; j++)
            // {
            //     printf("%d ", block[j]);
            // }
            // printf("\n");
        }

        // Decrypt the block
        decryptAndStoreInSRAM(block, decryptedBlock, cipherkey);
        // printf("Decrypted block:\t");
        // for (j = 0; j < BLOCK_SIZE; j++)
        // {
        //     printf("%d ", decryptedBlock[j]);
        // }
        // printf("\n");
        // Copy the decrypted block into the decrypted matrix
        // Be careful to not overwrite the buffer if it's not a multiple of 16
        size_t copySize =
                (i + BLOCK_SIZE > matrixSize) ? matrixSize - i : BLOCK_SIZE;
        memcpy(decryptedMatrixSRAM + i, decryptedBlock, copySize);
    }
}

encryptedMatrix AES256_encryptMatrix_ECB(uint8_t *matrix,
                                         uint8_t *encryptedMatrixFRAM,
                                         size_t matrixRows, size_t matrixCols)
{
    encryptedMatrix encryptedMatrix;
    encryptedMatrix.matrixRows = ROUND_UP_TO_MULTIPLE_OF_4(matrixRows);
    encryptedMatrix.matrixCols = ROUND_UP_TO_MULTIPLE_OF_4(matrixCols);
    encryptedMatrix.encryptedSize = encryptedMatrix.matrixRows * encryptedMatrix.matrixCols;
    printf("Encrypted matrix size: %d\n", encryptedMatrix.encryptedSize);
    encryptedMatrix.matrix = (uint8_t*) malloc(
            encryptedMatrix.matrixRows * encryptedMatrix.matrixCols);
    if (encryptedMatrix.matrix == NULL)
    {
        printf("Memory allocation failed\n");
        return encryptedMatrix;
    }

    uint8_t block[BLOCK_SIZE];
    size_t row, col;
    uint8_t encryptedBlock[BLOCK_SIZE];

    for (row = 0; row < matrixRows; row += BLOCK_ROWS)
    {
        for (col = 0; col < matrixCols; col += BLOCK_COLS)
        {
            size_t blockRow, blockCol;

            // Copy 4x4 block (or remaining elements) into the block
            for (blockRow = 0; blockRow < BLOCK_ROWS; ++blockRow)
            {
                for (blockCol = 0; blockCol < BLOCK_COLS; ++blockCol)
                {
                    size_t sourceRow = row + blockRow;
                    size_t sourceCol = col + blockCol;

                    if (sourceRow < matrixRows && sourceCol < matrixCols)
                    {
                        block[blockRow * BLOCK_COLS + blockCol] =
                                matrix[sourceRow * matrixCols + sourceCol];
                    }
                    else
                    {
                        block[blockRow * BLOCK_COLS + blockCol] = 0;
                    }
                }
            }
            // Process the block-> encrypt it
            encryptAndStoreInFRAM(block, encryptedBlock, cipherkey);
            //copy block into encrypted matrix
            for (blockRow = 0; blockRow < BLOCK_ROWS; ++blockRow)
            {
                for (blockCol = 0; blockCol < BLOCK_COLS; ++blockCol)
                {
                    size_t destRow = row + blockRow;
                    size_t destCol = col + blockCol;
                    encryptedMatrix.matrix[destRow * encryptedMatrix.matrixCols
                            + destCol] = encryptedBlock[blockRow * BLOCK_COLS
                            + blockCol];
                }
            }
            // Print the block
            // printf("Block starting at (%d, %d):\n", row, col);
            // for (blockRow = 0; blockRow < BLOCK_ROWS; ++blockRow)
            // {
            //     for (blockCol = 0; blockCol < BLOCK_COLS; ++blockCol)
            //     {
            //         printf("%d ", block[blockRow * BLOCK_COLS + blockCol]);
            //     }
            //     //print encrypted block
            //     printf("\t\t");
            //     for (blockCol = 0; blockCol < BLOCK_COLS; ++blockCol)
            //     {
            //         printf("%d ",
            //                encryptedBlock[blockRow * BLOCK_COLS + blockCol]);
            //     }
            //     printf("\n");
            // }
            // printf("\n");
        }
    }
    // printf("Encrypted matrix:\n");
    // Print_Matrix(encryptedMatrix.matrix, encryptedMatrix.matrixRows,
    //              encryptedMatrix.matrixCols);
    return encryptedMatrix;
}
