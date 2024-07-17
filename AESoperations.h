#ifndef AES_OPERATIONS
#define AES_OPERATIONS
#include <stdint.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "driverlib.h"
#include "rng.h"
#include "constants.h"

void generateRandomKey(uint16_t *cipherKey);

void encryptAndStoreInFRAM(const uint8_t *matrix, uint8_t *encryptedMatrixFRAM,
                           uint16_t *cipherKey);

void decryptAndStoreInSRAM(const uint8_t *encryptedMatrixFRAM,
                           uint8_t *decryptedMatrixSRAM, uint16_t *cipherKey);

encryptedMatrix AES256_encryptMatrix_ECB(uint8_t *matrix,
                                         uint8_t *encryptedMatrixFRAM,
                                         size_t matrixRows, size_t matrixCols);

void AES256_decryptMatrix_ECB(uint8_t *encryptedMatrixFRAM,
                              uint8_t *decryptedMatrixSRAM,
                              size_t encryptedSize, size_t matrixSize);

#endif
