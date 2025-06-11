#ifndef ATTACKS_H
#define ATTACKS_H

#include "definitions.h"

U64 pawn_attacks_mask(int square, int side); //given a specific pawn on a certain square, return a bitboard where the 1s are the squares it is attacking

#endif