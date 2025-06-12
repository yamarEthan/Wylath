#include <stdio.h>
#include "definitions.h"
#include "attacks.h"
#include "display.h"

U64 pawnAttacksTable[2][64]; //contains the attack table for pawns in both sides and for each square [side][square]

U64 pawn_attacks_mask(int square, int side) { //have yet to initialize pawnAttacksTable using this function for all squares and sides
    U64 bitboard = 0ULL; //empty bitboard
    U64 attackBitboard = 0ULL;
    set_bit(bitboard, square);

    if(!side) { //if white
        attackBitboard |= (bitboard << 7) & (NOT_H_FILE); //if the shifted bit doesn't end on the H file, accept it; if it is on H file, that means the attack square
        attackBitboard |= (bitboard << 9) & (NOT_A_FILE); //moved up two ranks, which is wrong

    } else { //if black
        attackBitboard |= (bitboard >> 7) & (NOT_A_FILE); //same logic
        attackBitboard |= (bitboard >> 9) & (NOT_H_FILE);
    }

    return attackBitboard; //returns only where the pawn can attack
}

U64 knight_attacks_mask(int square) {
    U64 bitboard = 0ULL;
    U64 attackBitboard = 0ULL;
    set_bit(bitboard, square);

    attackBitboard |= (bitboard << 17) & NOT_A_FILE; //same logic as pawn attack masks
    attackBitboard |= (bitboard << 15) & NOT_H_FILE;

    attackBitboard |= (bitboard << 10) & NOT_AB_FILE; 
    attackBitboard |= (bitboard << 6)  & NOT_GH_FILE;

    attackBitboard |= (bitboard >> 6)  & NOT_AB_FILE; 
    attackBitboard |= (bitboard >> 10) & NOT_GH_FILE; 

    attackBitboard |= (bitboard >> 15) & NOT_A_FILE;
    attackBitboard |= (bitboard >> 17) & NOT_H_FILE;

    return attackBitboard;
}

U64 king_attacks_mask(int square) {
    U64 bitboard = 0ULL;
    U64 attackBitboard = 0ULL;
    set_bit(bitboard, square);

    attackBitboard |= (bitboard >> 9) & NOT_H_FILE;
    attackBitboard |= (bitboard >> 8); //might have issues here
    attackBitboard |= (bitboard >> 7) & NOT_A_FILE;
    attackBitboard |= (bitboard >> 1) & NOT_H_FILE;
    attackBitboard |= (bitboard << 1) & NOT_A_FILE;
    attackBitboard |= (bitboard << 7) & NOT_H_FILE;
    attackBitboard |= (bitboard << 8);
    attackBitboard |= (bitboard << 9) & NOT_A_FILE;

    return attackBitboard;
}
/*
    A1, B1, C1, D1, E1, F1, G1, H1,     00, 01, 02, 03, 04, 05, 06, 07,
    A2, B2, C2, D2, E2, F2, G2, H2,     08, 09, 10, 11, 12, 13, 14, 15,
    A3, B3, C3, D3, E3, F3, G3, H3,     16, 17, 18, 19, 20, 21, 22, 23,
    A4, B4, C4, D4, E4, F4, G4, H4,     24, 25, 26, 27, 28, 29, 30, 31,
    A5, B5, C5, D5, E5, F5, G5, H5,     32, 33, 34, 35, 36, 37, 38, 39,
    A6, B6, C6, D6, E6, F6, G6, H6,     40, 41, 42, 43, 44, 45, 46, 47,
    A7, B7, C7, D7, E7, F7, G7, H7,     48, 49, 50, 51, 52, 53, 54, 55,
    A8, B8, C8, D8, E8, F8, G8, H8,     56, 57, 58, 59, 60, 61, 62, 63,
    NO_SQUARE                           64

    H8, G8, F8, E8, D8, C8, B8, A8, ..., H3, G3, F3, E3, D3, C3, B3, A3, H2, G2, F2, E2, D2, C2, B2, A2, H1, G1, F1, E1, D1, C1, B1, A1     SQUARE
    63, 62, 61, 60, 59, 58, 57, 57, ..., 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 09, 08, 07, 06, 05, 04, 03, 02, 01, 00     BIT
    
    the << operator shifts bits to the left heading to MSB
    the >> shifts bit to the right heading to LSB
*/