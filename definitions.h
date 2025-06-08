#ifndef DEFINITIONS_H
#define DEFINITIONS_H

typedef unsigned long long U64;

#define NAME "Wylath"
#define BRD_SQ_NUM 64

#define START_POSITION "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

typedef enum {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    NO_SQUARE
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