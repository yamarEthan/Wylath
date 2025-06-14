#ifndef ATTACKS_H
#define ATTACKS_H

#include "definitions.h"

U64 pawn_attacks_mask(int square, int side); //given a specific pawn on a certain square, return a bitboard where the 1s are the squares it is attacking

U64 knight_attacks_mask(int square);

U64 king_attacks_mask(int square);

U64 bishop_blockers_mask(int square);

U64 rook_blockers_mask(int square);

U64 get_blockers_bitboard(int index, int bitsInMask, U64 blockersMask);

U64 bishop_attacks_mask(int square, U64 blockersBitboard);

U64 rook_attacks_mask(int square, U64 blockersBitboard);

#endif