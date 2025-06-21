#ifndef ATTACKS_H
#define ATTACKS_H

#include "definitions.h"

extern U64 pawnAttackTable[2][64]; //contains the attack table for pawns in both sides and for each square [side][square]
extern U64 knightAttackTable[64];
extern U64 kingAttackTable[64];

extern U64 bishopBlockerMask[64];
extern U64 rookBlockerMask[64];

extern U64 bishopAttackTable[64][512];
extern U64 rookAttackTable[64][4096];

extern int bishopBitsSeen[64];
extern int rookBitsSeen[64];
extern U64 BishopMagicNumbers[64];
extern U64 RookMagicNumbers[64];

U64 pawn_attack_mask(int square, int side); //given a specific pawn on a certain square, return a bitboard where the 1s are the squares it is attacking
U64 knight_attack_mask(int square);
U64 king_attack_mask(int square);

U64 bishop_blocker_mask(int square);
U64 rook_blocker_mask(int square);
U64 bishop_attack_mask(int square, U64 blockerBitboard); //used to generate all attacks mask in all squares with all permutations of blockers
U64 rook_attack_mask(int square, U64 blockerBitboard);

U64 get_blocker_bitboard(int index, int bitsInMask, U64 blockerMask); //gets a specific blockers bitboard given its index, how many bits in mask, and the blockers mask

void init_leapers_attacks(); //stores all the attacks of the king, pawns, and knights
void init_sliders_attacks(int bishop); //stores all the blockers masks and attacks mask in arrays and sets up the atttacks table array with magic indices

U64 get_bishop_attacks(int square, U64 blockerBitboard); //this function just grabs the attack bitboard from an array, using its magic index, rather than
U64 get_rook_attacks(int square, U64 blockerBitboard); //generating it like i nthe attack_mask functions
U64 get_queen_attacks(int square, U64 blockerBitboard); //just a union of bishop and rook attacks

static inline int is_square_attacked(int square, int side);

#endif