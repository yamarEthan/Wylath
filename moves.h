#ifndef ATTACKS_H
#define ATTACKS_H

#include "definitions.h"

//right now we are only generating moves, so we are modifying one MoveList called pseudoMoves while only copying the piece bitboards to find what moves we can make
void generate_moves(MoveList *moveList);

//therefore we only pass the bitboards itself and not the address
void generate_pawn_moves(MoveList *moveList, U64 pawnBitboard, int side);

void generate_knight_moves(MoveList *moveList, U64 knightBitboard);

void generate_bishop_moves(MoveList *moveList, U64 bishopBitboard);

void generate_rook_moves(MoveList *moveList, U64 rookBitboard);

void generate_queen_moves(MoveList *moveList, U64 queenBitboard);

void generate_king_moves(MoveList *moveList, U64 kingBitboard);

#endif