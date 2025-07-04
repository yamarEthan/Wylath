#ifndef DEFINITIONS_H
#define DEFINITIONS_H

typedef unsigned long long U64;

typedef __uint16_t Move;

#define NAME "Wylath"

#define START_POSITION "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define TWO_EN_PASSANT "2r3k1/1q1nbppp/r3p3/3pP3/pPpP4/P1Q2N2/2RN1PPP/2R4K b - b3 0 23" //this position has two black pawns able to capture with enpassant
#define MAX_MOVES "R6R/3Q4/1Q4Q1/4Q3/2Q4Q/Q4Q2/pp1Q4/kBNN1KB1 w - - 0 1" //position with 218 legal moves; not the max possible but it's up there

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

typedef struct { //in the future, instead of storing the moves as just int, we may encode it as specifically 32-bit numbers for extra information
    Move moves[256]; //the record of most number of legal moves in one position is 218, so 256 is a good enough amount to store all moves for any position
    int count; //keep track of how many moves are in the list
} MoveList;

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

#define encode_move(source, target, flags) ((Move) (((source) & 0x3F) | (((target) & 0x3F) << 6) | ((flags) << 12)))
//the first 6 bits is to encode the original square, the next 6 bits the square it's heading to, and the last 4 bits for any flags such as castling/promotion
#define get_source(move) ((move) & 0x3F)
#define get_target(move) (((move) >> 6) & 0x3F)
#define get_flag(move) ((move) >> 12)

//FLAGS WITH 4-BITS (taken from chess programming wiki):
#define QUIET_FLAG 0
#define DOUBLE_PUSH_FLAG 1 //double pawn push
#define KING_CASTLE_FLAG 2
#define QUEEN_CASTLE_FLAG 3
#define CAPTURE_FLAG 4
#define EP_CAPTURE_FLAG 5 //en passant capture
//skip 6 and 7 to split between promo flags and other flags
#define KNIGHT_PROMO_FLAG 8
#define BISHOP_PROMO_FLAG 9
#define ROOK_PROMO_FLAG 10
#define QUEEN_PROMO_FLAG 11
#define NP_CAPTURE_FLAG 12 //knight promotion and capture
#define BP_CAPTURE_FLAG 13
#define RP_CAPTURE_FLAG 14
#define QP_CAPTURE_FLAG 15 

//"0x" is just the prefix to show that it's a hexademical number; 0x3F == 63 (3 * 16^1 + F(15))
//to convert hexadecmial to binary, just turn every digit into it's 4-bit representation; 0x3F = 0011 1111
//the purpose of the 0x3F is to make sure the source and target variables will only have their first 6 bits contribute (0-63)
//so if you happen to give a number higher than 63, the "& 0x3F" will truncate it to a number within 0-63.

#endif
