#ifndef MOVES_H
#define MOVES_H

#include "definitions.h"

extern MoveList pseudoMoves;

void add_move(MoveList *moveList, Move move);

int make_move(Move move);

void generate_pawn_moves(MoveList *moveList, int side);

void generate_knight_moves(MoveList *moveList, int side);

void generate_bishop_moves(MoveList *moveList, int side);

void generate_rook_moves(MoveList *moveList, int side);

void generate_queen_moves(MoveList *moveList, int side);

void generate_king_moves(MoveList *moveList, int side);

void generate_moves(MoveList *moveList); //generate pseudo moves (doesn't check if a move leaves their king in check)

#endif