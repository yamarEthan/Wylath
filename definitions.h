#ifndef DEFINITIONS_H
#define DEFINITIONS_H

typedef unsigned long long U64;

#define NAME "Wylath"
#define BRD_SQ_NUM 64

#define START_POSITION "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

typedef enum {
    A1, B1, C1, D1, E1, F1, G1, H1,     //00, 01, 02, 03, 04, 05, 06, 07,
    A2, B2, C2, D2, E2, F2, G2, H2,     //08, 09, 10, 11, 12, 13, 14, 15,
    A3, B3, C3, D3, E3, F3, G3, H3,     //16, 17, 18, 19, 20, 21, 22, 23,
    A4, B4, C4, D4, E4, F4, G4, H4,     //24, 25, 26, 27, 28, 29, 30, 31,
    A5, B5, C5, D5, E5, F5, G5, H5,     //32, 33, 34, 35, 36, 37, 38, 39,
    A6, B6, C6, D6, E6, F6, G6, H6,     //40, 41, 42, 43, 44, 45, 46, 47,
    A7, B7, C7, D7, E7, F7, G7, H7,     //48, 49, 50, 51, 52, 53, 54, 55,
    A8, B8, C8, D8, E8, F8, G8, H8,     //56, 57, 58, 59, 60, 61, 62, 63,
    NO_SQUARE                           //64
} Square; //board squares

//  H8, G8, F8, E8, D8, C8, B8, A8, ..., H3, G3, F3, E3, D3, C3, B3, A3, H2, G2, F2, E2, D2, C2, B2, A2, H1, G1, F1, E1, D1, C1, B1, A1     SQUARE
//  63, 62, 61, 60, 59, 58, 57, 57, ..., 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 09, 08, 07, 06, 05, 04, 03, 02, 01, 00     BIT

typedef enum {P, N, B, R, Q, K, p, n, b, r, q, k} Piece; //pieces; uppercase for white pieces, lowercase for black pieces

typedef enum {white, black, both} Side; //side to move; also used to reference the occupancy bitboards

typedef enum {wk = 1, wq = 2, bk = 4, bq = 8} CastlingRights; //castling rights

//remember that a bitboard is just a 64 bit number
#define set_bit(bitboard, square) ((bitboard) |= (1ULL << (square))) //sets the bit at bitboard bit "square" (zero-based indexing) to 1
#define get_bit(bitboard, square) ((bitboard) & (1ULL << (square))) //returns 1 if there is a 1 at that bit in the bitboard; else 0
#define pop_bit(bitboard, square) ((bitboard) &= ~(1ULL << (square))) //~ inverts the mask and &= sets the one at square to 0; ignores other bits
#define count_bits(bitboard) __builtin_popcountll(bitboard) //returns how manys 1s are in a bitboard
#define get_lsb_index(bitboard) __builtin_ctzll(bitboard)

#define NOT_A_FILE 18374403900871474942ULL //bitboard where file A is all 0s and the rest of the bits are 1s
#define NOT_H_FILE 9187201950435737471ULL //file H is all 0s, rest are 1s; used for pawn attacks mask
#define NOT_AB_FILE 18229723555195321596ULL //used for knight
#define NOT_GH_FILE 4557430888798830399ULL //attacks mask

#endif

//now that the attack tables are all set, we must now deal with move generations and such. The first thing to do is check if a square is attacked and by who.
//I think according to the chess wiki we make a function that returns true or false whether a square is attacked and which side is attacking