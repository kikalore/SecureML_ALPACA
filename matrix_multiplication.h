#include "constants.h"
#ifndef MATRIX_MULTIPLICATION
#define MATRIX_MULTIPLICATION

bool Matrix_Equality(uint8_t *matrix1, uint8_t *matrix2, int rows, int cols);
void Print_Matrix(uint8_t *matrix, int rows, int cols);
void Initialiaze_Matrix(uint8_t *matrix, int rows, int cols);

void Basic_Multiplication(uint8_t I[I_R][I_C], uint8_t W[I_C][W_C],
                          uint8_t O[I_R][W_C]);
void Tiled_Multiplication(uint8_t I[I_R][I_C], uint8_t W[I_C][W_C],
                          uint8_t O[I_R][W_C]);

encryptedMatrix Tiled_Decryption_Multiplication(encryptedMatrix I_encrypted, encryptedMatrix W_encrypted, uint8_t *O);
#endif
