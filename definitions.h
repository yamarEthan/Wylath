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

typedef struct { //in the future, instead of storing the moves as just int, we may encode it as specifically 32-bit numbers for extra information
    int moves[256]; //the record of most number of legal moves in one position is 218, so 256 is a good enough amount to store all moves for any position
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

#endif

//now that the attack tables are all set, we must now deal with move generations and such. The first thing to do is check if a square is attacked and by who.
//I think according to the chess wiki we make a function that returns true or false whether a square is attacked and which side is attacking

//now we must generate moves. attacks are easy and are initialized at the start of the program and then stored to be able to be easily accessed, but move generation
//is constantly being updated to take account of board state and legal moves. we need to generate what moves are possible given the current board state and side
//to move.

//more steps to generation: we divide it to two parts: move generation and move making.
//  move generation only generates what the pieces can do; it makes pseduolegal moves
// move making confirms the move by making sure it is a legal move; i.e. it does not leave the king in check

//from what I've seen online, all the possible moves are stored in a move list. C doesn't have lists though, so maybe just an array?

//for move generation, the order of the moves in the movelist does not matter at the moment so just stuff them all in there.
//it's only DURING the searching and evaluating that the order of the moves in the list matters as if the first move is good, the rest is ignored, saving time.

//the standard way to do move list is a struct with an array of size [256] and a count of how many moves are in that array

//after generating the moves, we also need to encode the moves for extra information. Online, it states to stores moves as a 32-bit integer, with every 4
//or so bits storing information, such as castling rights and enpassant. This is probably just a struct and a quick rewrite to MoveList struct.