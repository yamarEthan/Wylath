#include <stdio.h>
#include "definitions.h"
#include "attacks.h"

const int bishopBitsSeen[64] = { //given a bishop on a square, this tells you how many squares it looks at
    6, 5, 5, 5, 5, 5, 5, 6, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    5, 5, 7, 7, 7, 7, 5, 5, 
    5, 5, 7, 9, 9, 7, 5, 5, 
    5, 5, 7, 9, 9, 7, 5, 5, 
    5, 5, 7, 7, 7, 7, 5, 5, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    6, 5, 5, 5, 5, 5, 5, 6
};

const int rookBitsSeen[64] = { //for rooks
    12, 11, 11, 11, 11, 11, 11, 12, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    12, 11, 11, 11, 11, 11, 11, 12
};

U64 pawnAttacksTable[2][64]; //contains the attack table for pawns in both sides and for each square [side][square]
U64 knightAttacksTable[64];
U64 kingAttacksTable[64];

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
U64 bishop_blockers_mask(int square) { //generates all squares a bishop sees minus the border squares
    U64 attackBitboard = 0ULL;

    int rank, file;
    int targetRank = square / 8;
    int targetFile = square % 8;

    for(rank = targetRank + 1, file = targetFile + 1; rank < 7 && file < 7; rank++, file++) {attackBitboard |= (1ULL << ((rank * 8) + file));}
    for(rank = targetRank + 1, file = targetFile - 1; rank < 7 && file > 0; rank++, file--) {attackBitboard |= (1ULL << ((rank * 8) + file));}
    for(rank = targetRank - 1, file = targetFile + 1; rank > 0 && file < 7; rank--, file++) {attackBitboard |= (1ULL << ((rank * 8) + file));}
    for(rank = targetRank - 1, file = targetFile - 1; rank > 0 && file > 0; rank--, file--) {attackBitboard |= (1ULL << ((rank * 8) + file));}

    return attackBitboard;
}

U64 rook_blockers_mask(int square) { //same as above but for rooks
    U64 attackBitboard = 0ULL;
    int rank, file;
    int targetRank = square / 8;
    int targetFile = square % 8;

    for(file = targetFile + 1; file < 7; file++) {attackBitboard |= (1ULL << ((targetRank * 8) + file));}
    for(file = targetFile - 1; file > 0; file--) {attackBitboard |= (1ULL << ((targetRank * 8) + file));}
    for(rank = targetRank + 1; rank < 7; rank++) {attackBitboard |= (1ULL << ((rank * 8) + targetFile));}
    for(rank = targetRank - 1; rank > 0; rank--) {attackBitboard |= (1ULL << ((rank * 8) + targetFile));}

    return attackBitboard;
}

U64 get_blockers_bitboard(int index, int bitsInMask, U64 blockersMask) {
    /*returns one of the possible blockerBitboard combinations given its index, number of bits, and the blockersMask
        For example, let's look at a rook on A1. It sees 12 squares for a total of 4096 possible blockerBoards. At index 0, there is no pieces blocking
        its view. At index 4095, there's a piece at every single square it looks at

        bitsInMask can be found using count_bits

        blockersMask is the blockers mask for a given square and piece type
    */
    U64 blockersBitboard = 0ULL;

    for(int i = 0; i < bitsInMask; i++) {
        int square = get_lsb_index(blockersMask);
        pop_bit(blockersMask, square);
        
        if(index & (1 << i)) { //if bit i is a 1 in index, we set bit (square) as a 1 in the blockersBitboard
            blockersBitboard |= (1ULL << square);
        }
    }
    
    return blockersBitboard;
}

U64 bishop_attacks_mask(int square, U64 blockersBitboard) { //given a bitboard of blockers, generate the possible moves
    U64 attackBitboard = 0ULL;
    int rank, file;
    int targetRank = square / 8;
    int targetFile = square % 8;

    for(rank = targetRank + 1, file = targetFile + 1; rank < 8 && file < 8; rank++, file++) { //given a square, set every positive 9th bit; SE direction
        attackBitboard |= (1ULL << ((rank * 8) + file));
        if((1ULL << ((rank * 8) + file)) & blockersBitboard) break; //if that square is occupied in blockersBitboard, return non-zero and break
    }

    for(rank = targetRank + 1, file = targetFile - 1; rank < 8 && file >= 0; rank++, file--) { //SW direction
        attackBitboard |= (1ULL << ((rank * 8) + file));
        if((1ULL << ((rank * 8) + file)) & blockersBitboard) break;
    }

    for(rank = targetRank - 1, file = targetFile + 1; rank >= 0 && file < 8; rank--, file++) { //NE
        attackBitboard |= (1ULL << ((rank * 8) + file));
        if((1ULL << ((rank * 8) + file)) & blockersBitboard) break;
    }

    for(rank = targetRank - 1, file = targetFile - 1; rank >= 0 && file >= 0; rank--, file--) { //NW direction
        attackBitboard |= (1ULL << ((rank * 8) + file));
        if((1ULL << ((rank * 8) + file)) & blockersBitboard) break;
    }

    return attackBitboard;
}

U64 rook_attacks_mask(int square, U64 blockersBitboard) { //have to fix this
    U64 attackBitboard = 0ULL;

    //imagine a rook on A1
    //it targets the entire rank and file it's at; only two for loops? no because we want to check for blockers in each direction starting from the square
    int rank, file;
    int targetRank = square / 8;
    int targetFile = square % 8;

    //again from black's perspective
    for(file = targetFile + 1; file < 8; file++) { //heading east
        attackBitboard |= (1ULL << ((targetRank * 8) + file));
        if((1ULL << ((targetRank * 8) + file)) & blockersBitboard) break;
    }

    for(file = targetFile - 1; file >= 0; file--) { //west
        attackBitboard |= (1ULL << ((targetRank * 8) + file));
        if((1ULL << ((targetRank * 8) + file)) & blockersBitboard) break;
    }

    for(rank = targetRank + 1; rank < 8; rank++) { //south
        attackBitboard |= (1ULL << ((rank * 8) + targetFile));
        if((1ULL << ((rank * 8) + targetFile)) & blockersBitboard) break;
    }

    for(rank = targetRank - 1; rank >= 0; rank--) { //north
        attackBitboard |= (1ULL << ((rank * 8) + targetFile));
        if((1ULL << ((rank * 8) + targetFile)) & blockersBitboard) break;
    }

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

/*
How do I generate the magic bitboard numbers?

I need three things:

1.  The blocker masks: this is similar to what we have been doing for the other pieces, finding all the possible places the piece can go to given a square
    MINUS THE EDGES (ignore A,H file and 1,8 rank). Why? The piece can't go past them anyways so no need to calculate possible blockers there.

2.  The blocker boards: A rook on E4, for example, can see 14 squares. The total possible combination of pieces blocking the rook is 2^14 which is huge.
    By ignoring the border squares, we reduce it to 2^10, much easier. The blocker boards are a subset of the blocker masks as the pieces are always inside
    the square the pieces can see. We don't actually need a function for this we just iterate from 0 to 2^n.
    
3.  The move board: This is the code we have above right now, feeding it the blocker boards to get the move board.

Step 1 is done. Pretty easy.

Step 2 is where it gets hard. 
*/