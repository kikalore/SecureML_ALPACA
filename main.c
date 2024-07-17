#include "driverlib.h"
#include <stdio.h>
#include <stdlib.h>
#include "matrix_multiplication.h"
#include "AESoperations.h"

#pragma PERSISTENT(I_encrypted_ECB)
encryptedMatrix I_encrypted_ECB;
#pragma PERSISTENT(W_encrypted_ECB)
encryptedMatrix W_encrypted_ECB;
#pragma PERSISTENT(O_encrypted_ECB)
encryptedMatrix O_encrypted_ECB;
#pragma PERSISTENT(isFRAMInitialized)
bool isFRAMInitialized = 0; // Flag to check if FRAM is initialized
#pragma PERSISTENT(cipherkey)
uint16_t cipherkey[LENGTH] = { 0 };

int main(void)
{
    WDT_A_hold(WDT_A_BASE);

    generateRandomKey(cipherkey);

//TESTING TILED MULTIPLICATION
    printf("*******TESTING TILED MULTIPLICATION*******\n");
    uint8_t I[I_R][I_C] = { 0 };
    uint8_t W[I_C][W_C] = { 0 };
    uint8_t O[I_R][W_C] = { 0 };
    Initialiaze_Matrix(I, I_R, I_C);
    Initialiaze_Matrix(W, I_C, W_C);
    Basic_Multiplication(I, W, O);
    printf("Result of multiplication is:\n");
    Print_Matrix(O, I_R, W_C);


    printf("*******TESTING NEW TILED TECHNIQUE*******\n");
    printf("Input matrix plaintext is:\n");
    Print_Matrix(I, I_R, I_C);
    I_encrypted_ECB = AES256_encryptMatrix_ECB(I, I_encrypted_ECB.matrix, I_R, I_C);
    printf("Weight matrix plaintext is:\n");
    Print_Matrix(W, I_C, W_C);
    W_encrypted_ECB = AES256_encryptMatrix_ECB(W, I_encrypted_ECB.matrix, I_C, W_C);
    uint8_t Otest[ROUND_UP_TO_MULTIPLE_OF_4(I_R)][ROUND_UP_TO_MULTIPLE_OF_4(W_C)];

    O_encrypted_ECB = Tiled_Decryption_Multiplication(I_encrypted_ECB, W_encrypted_ECB, Otest);
    printf("Correct output matrix is:\n");
    Print_Matrix(O, I_R, W_C);

    printf("Output matrix encrypted is:\n");
    Print_Matrix(O_encrypted_ECB.matrix, O_encrypted_ECB.matrixRows, O_encrypted_ECB.matrixCols);
    decryptAndStoreInSRAM(O_encrypted_ECB.matrix, Otest, cipherkey);


    free(I_encrypted_ECB.matrix);
    free(W_encrypted_ECB.matrix);
    free(O_encrypted_ECB.matrix);
    return 0;
}
