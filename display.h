#ifndef DISPLAY_H
#define DISPLAY_H

#include "definitions.h"

//colors to mess around with printing
#define RESET   "\x1b[0m"
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define BRIGHT_RED     "\x1b[91m"
#define BRIGHT_GREEN   "\x1b[92m"
#define BRIGHT_YELLOW  "\x1b[93m"
#define BRIGHT_BLUE    "\x1b[94m"
#define BRIGHT_MAGENTA "\x1b[95m"
#define BRIGHT_CYAN    "\x1b[96m"
#define BRIGHT_WHITE   "\x1b[97m"

extern U64 pieceBitboards[12]; //each piece type will have their own bitboard; order is P, N, B, R, Q, K, p, n, b, r, q, k
extern U64 occupancyBitboards[3]; //contains the location of all pieces of white, black, and both

extern int side; //side to move
extern int enPassant; //en Passant square
extern int castlingRights; //castling rights

extern U64 positionKey; //current set up of Board

extern int fullMoves; //number of full plys played in total in the game

extern int ply; //half move; white/black plays a move but not the other side; resets after a pawn push or piece capture; used for 50-move draw rule

extern int charToPiece[]; //useful for parsing a FEN string
extern char pieceToChar[12];
extern char *unicode_pieces[12];

void parse_fen(char *fen);
void print_bitboard(U64 bitboard); //print a given bitboard in chessboard manner
void print_board(); //takes all piece bitboards and puts in place the pieces


#endif