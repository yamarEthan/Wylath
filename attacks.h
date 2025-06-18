#ifndef ATTACKS_H
#define ATTACKS_H

#include "definitions.h"

extern U64 pawnAttacksTable[2][64]; //contains the attack table for pawns in both sides and for each square [side][square]
extern U64 knightAttacksTable[64];
extern U64 kingAttacksTable[64];

extern U64 bishopBlockersMask[64];
extern U64 rookBlockersMask[64];

extern U64 bishopAttacksTable[64][512];
extern U64 rookAttacksTable[64][4096];

extern int bishopBitsSeen[64];
extern int rookBitsSeen[64];
extern U64 BishopMagicNumbers[64];
extern U64 RookMagicNumbers[64];

U64 pawn_attacks_mask(int square, int side); //given a specific pawn on a certain square, return a bitboard where the 1s are the squares it is attacking
U64 knight_attacks_mask(int square);
U64 king_attacks_mask(int square);

U64 bishop_blockers_mask(int square);
U64 rook_blockers_mask(int square);
U64 get_blockers_bitboard(int index, int bitsInMask, U64 blockersMask); //gets a specific blockers bitboard given its index, how many bits in mask, and the blockers mask
U64 bishop_attacks_mask(int square, U64 blockersBitboard); //used to generate all attacks mask in all squares with all permutations of blockers
U64 rook_attacks_mask(int square, U64 blockersBitboard);

void init_leapers_attacks(); //stores all the attacks of the king, pawns, and knights
void init_sliders_attacks(int bishop); //stores all the blockers masks and attacks mask in arrays and sets up the atttacks table array with magic indices

U64 get_bishop_attacks(int square, U64 blockersBitboard); //get a specific attack mask given the current board
U64 get_rook_attacks(int square, U64 blockersBitboard);
U64 get_queen_attacks(int square, U64 blockersBitboard); //just a union of bishop and rook attacks

#endif