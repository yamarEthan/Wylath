#include <stdio.h>
#include "definitions.h"
#include "display.h"

U64 bitboards[12]; //each piece type will have their own bitboard; order is P, N, B, R, Q, K, p, n, b, r, q, k

int side; //side to move
int enPassant = NO_SQUARE; //en Passant square
int castle; //castling rights

U64 positionKey; //current set up of Board

int repetition; //how many moves (full ply) have gone without a capture or pawn push; used for the fifty move rule

int ply; //half move; white/black plays a move but not the other side

/*FEN has 6 fields:
    1. Piece Placement; lowercase for black, uppercase for white
    2. Active Color (b/w/-)
    3. Castling Rights (KQkq) in this order and remove letters accordingly if that right is loss; - if no castling anymore
    4. En Passant Target Square: if a pawn moves twice, the square it skips is immediately put here even if no pawn can capture it
    5. Halfmove Clock: resets to 0 after a pawn move or piece capture; useful for the 50-move draw rule
    6. Fullmove Number: increments everytime black plays a move

    Example: Starting Position FEN is: "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
*/

int main() {
    U64 bitboard = 0LL;
    set_bit(bitboard, A1);
    set_bit(bitboard, A7);
    set_bit(bitboard, C6);
    print_bitboard(bitboard);
    return 0;
}
