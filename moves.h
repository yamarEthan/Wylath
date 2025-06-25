#ifndef MOVES_H
#define MOVES_H

#include "definitions.h"

extern MoveList pseudoMoves;

void add_move(MoveList *moveList, Move move);
//right now we are only generating moves, so we are modifying one MoveList called pseudoMoves while only copying the piece bitboards to find what moves we can make
void generate_moves(MoveList *moveList);

//therefore we only pass the bitboards itself and not the address
void generate_pawn_moves(MoveList *moveList, int side);

void generate_knight_moves(MoveList *moveList, int side);

void generate_bishop_moves(MoveList *moveList, int side);

void generate_rook_moves(MoveList *moveList, int side);

void generate_queen_moves(MoveList *moveList, int side);

void generate_king_moves(MoveList *moveList, int side);

void print_move(Move move);

void print_movelist(MoveList *moveList);
#endif