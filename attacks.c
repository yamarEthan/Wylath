#include <stdio.h>
#include "definitions.h"
#include "attacks.h"
#include "display.h"

U64 pawnAttacksTable[2][64]; //contains the attack table for pawns in both sides and for each square [side][square]

U64 pawn_attacks_mask(int square, int side) {
    U64 bitboard = 0ULL; //empty bitboard
    U64 attackBitboard = 0ULL;
    set_bit(bitboard, square);

    if(!side) { //if white
        if(bitboard & NOT_A_FILE) attackBitboard |= (bitboard << 7); //if piece is not in A file, shift the bit down one rank and to the left one file
        if(bitboard & NOT_H_FILE) attackBitboard |= (bitboard << 9); //same logic
    } else {
        if(bitboard & NOT_H_FILE) attackBitboard |= (bitboard >> 7); //shifts up and to the right
        if(bitboard & NOT_A_FILE) attackBitboard |= (bitboard >> 9);
    }

    return attackBitboard; //returns only where the pawn can attack
}