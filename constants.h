#ifndef CONSTANTS_H__
#define CONSTANTS_H__
#define I_R 28
#define I_C 28
#define W_C 5
#define BLOCK_SIZE 16
#define TILE_SIZE 16
#define BLOCK_ROWS 4
#define BLOCK_COLS 4
#define ROUND_UP_TO_MULTIPLE_OF_4(x) ((x) + ((x) % 4 ? (4 - (x) % 4) : 0))

#define LENGTH 32 // Length of random data to generate, multiple of RNG_KEYLEN
typedef struct
{
    uint8_t *matrix;
    size_t matrixRows;
    size_t matrixCols;
    size_t encryptedSize;
} encryptedMatrix;
#endif
