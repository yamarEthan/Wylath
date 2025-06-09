#ifndef DEFINITIONS_H
#define DEFINITIONS_H

typedef unsigned long long U64;

#define NAME "Wylath"
#define BRD_SQ_NUM 64

#define START_POSITION "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define TEST_POSITION "1R6/8/8/8/8/8/k1K5/8 w - - 123 59"

typedef enum {
    A1, B1, C1, D1, E1, F1, G1, H1, //0, 1, 2, 3, 4, 5, 6, 7,
    A2, B2, C2, D2, E2, F2, G2, H2, //8, 9, 10, 11, 12, 13, 14, 15,
    A3, B3, C3, D3, E3, F3, G3, H3, //16, 17, 18, 19, 20, 21, 22, 23,
    A4, B4, C4, D4, E4, F4, G4, H4, //24, 25, 26, 27, 28, 29, 30, 31,
    A5, B5, C5, D5, E5, F5, G5, H5, //32, 33, 34, 35, 36, 37, 38, 39,
    A6, B6, C6, D6, E6, F6, G6, H6, //40, 41, 42, 43, 44, 45, 46, 47,
    A7, B7, C7, D7, E7, F7, G7, H7, //48, 49, 50, 51, 52, 53, 54, 55,
    A8, B8, C8, D8, E8, F8, G8, H8, //56, 57, 58, 59, 60, 61, 62, 63,
    NO_SQUARE //64
} Square; //board squares

typedef enum {P, N, B, R, Q, K, p, n, b, r, q, k} Piece; //pieces; uppercase for white pieces, lowercase for black pieces

typedef enum {white, black, both} Side; //side to move

typedef enum {wk = 1, wq = 2, bk = 4, bq = 8} CastlingRights; //castling rights

//remember that a bitboard is just a 64 bit number
#define set_bit(bitboard, square) ((bitboard) |= (1ULL << (square))) //sets the bit at bitboard bit "square" (zero-based indexing) to 1
#define get_bit(bitboard, square) ((bitboard) & (1ULL << (square))) //returns 1 if there is a 1 at that bit in the bitboard; else 0
#define pop_bit(bitboard, square) ((bitboard) &= ~(1ULL << (square))) //~ inverts the mask and &= sets the one at square to 0; ignores other bits

#endif



//today, i want to print a board given a FEN string. this means that we parse through the string and update all the bitboards accordingly.
//finally, we fully print the entire board with all the pieces. instead of unique ascii chars, we will use lowercase and uppercase letters for pieces