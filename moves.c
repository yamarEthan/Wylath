#include <stdio.h>
#include "definitions.h"
#include "attacks.h"
#include "display.h"
#include "moves.h"

MoveList pseudoMoves;

void add_move(MoveList *moveList, Move move) {
    moveList->moves[moveList->count] = move;
    moveList->count++;
}

//remove pawnBitboard parameter and add side parameter for all generate moves functions
void generate_pawn_moves(MoveList *moveList, int side) { //the add_move function already increases count so no need to worry about that in here
    U64 pawnBitboard = (side == white) ? pieceBitboards[P] : pieceBitboards[p];
    int source, target;
    if(side == white) {
        while(pawnBitboard) {
            source = get_lsb_index(pawnBitboard);
            target = source + 8;
            if(!(target > H8) && !get_bit(occupancyBitboards[both], target)) { //if the target does not go past the board (H8 = 63) and the target square is empty,
                if(source >= A7 && source <= H7) { //if it's on the 7th rank
                    add_move(moveList, encode_move(source, target, QUEEN_PROMO_FLAG)); //pawn push one square with promotion
                    add_move(moveList, encode_move(source, target, ROOK_PROMO_FLAG));
                    add_move(moveList, encode_move(source, target, BISHOP_PROMO_FLAG));
                    add_move(moveList, encode_move(source, target, KNIGHT_PROMO_FLAG));        
                } else {
                    add_move(moveList, encode_move(source, target, QUIET_FLAG)); //pawn push one square
                    if((source >= A2 && source <= H2) && !get_bit(occupancyBitboards[both], target + 8)) { //double pawn push
                        add_move(moveList, encode_move(source, target + 8, DOUBLE_PUSH_FLAG)); //when this occurs, we need to set the enpassant variable in a later function
                    }
                }
            }
            //now we deal with attacks and enpassant attack
            U64 attacks = pawnAttackTable[white][source] & occupancyBitboards[black];
            while(attacks) {
                target = get_lsb_index(attacks);
                if(source >= A7 && source <= H7) { //if on 7th rank, capture and promote
                    add_move(moveList, encode_move(source, target, QP_CAPTURE_FLAG));
                    add_move(moveList, encode_move(source, target, RP_CAPTURE_FLAG));
                    add_move(moveList, encode_move(source, target, BP_CAPTURE_FLAG));
                    add_move(moveList, encode_move(source, target, NP_CAPTURE_FLAG));
                } else {
                    add_move(moveList, encode_move(source, target, CAPTURE_FLAG)); //regular capture elsewise
                }
                pop_bit(attacks, target);
            }
            pop_bit(pawnBitboard, source);
        }
    } else { //if black
        while(pawnBitboard) {
            source = get_lsb_index(pawnBitboard);
            target = source - 8;
            if(!(target < A1) && !get_bit(occupancyBitboards[both], target)) {
                if(source >= A2 && source <= H2) {
                    add_move(moveList, encode_move(source, target, QP_CAPTURE_FLAG));
                    add_move(moveList, encode_move(source, target, RP_CAPTURE_FLAG));
                    add_move(moveList, encode_move(source, target, BP_CAPTURE_FLAG));
                    add_move(moveList, encode_move(source, target, NP_CAPTURE_FLAG));
                } else {
                    add_move(moveList, encode_move(source, target, QUIET_FLAG)); //pawn push one square
                    if((source >= A7 && source <= H7) && !get_bit(occupancyBitboards[both], target - 8)) {
                        add_move(moveList, encode_move(source, target - 8, DOUBLE_PUSH_FLAG));
                    }
                }
            }
            U64 attacks = pawnAttackTable[black][source] & occupancyBitboards[white];
            while(attacks) {
                target = get_lsb_index(attacks);
                if(source >= A2 && source <= H2) { //if on 2nd rank, capture and promote
                    add_move(moveList, encode_move(source, target, QP_CAPTURE_FLAG));
                    add_move(moveList, encode_move(source, target, RP_CAPTURE_FLAG));
                    add_move(moveList, encode_move(source, target, BP_CAPTURE_FLAG));
                    add_move(moveList, encode_move(source, target, NP_CAPTURE_FLAG));
                } else {
                    add_move(moveList, encode_move(source, target, CAPTURE_FLAG));
                }
                pop_bit(attacks, target);
            }
            pop_bit(pawnBitboard, source);
        }
    }
    if(enPassant != NO_SQUARE) {
        U64 google = (side == white) ? pawnAttackTable[black][enPassant] & pieceBitboards[P] : pawnAttackTable[white][enPassant] & pieceBitboards[p]; //like the meme "google enPassant" lmao
        while(google) {
            source = get_lsb_index(google);
            add_move(moveList, encode_move(source, enPassant, EP_CAPTURE_FLAG));
            pop_bit(google, source);
        }
    }
}

/* let's imagine this position where there are two white pawns that can capture with enpassant
8  - - - - - - - -
7  - - - - - - - -
6  - - x - - - - -
5  - P p P - - - -
4  - - - - - - - -
3  - - - - - - - -
2  - - - - - - - -
1  - - - - - - - -

so for white, we can see that the target square looks like if a black pawn was there and can take either of the white pawns
so we want to get the black pawn attack table for that square and & it with white's occupancy bitboard
*/

void generate_knight_moves(MoveList *moveList, int side) {
    U64 knightBitboard = (side == white) ? pieceBitboards[N] : pieceBitboards[n]; //get the side's knight pieces bitboard
    U64 opponentOccupancy = (side == white) ? occupancyBitboards[black] : occupancyBitboards[white];
    int source, target;
    while(knightBitboard) { //while there are knights on the board
        source = get_lsb_index(knightBitboard); //get one knight
        U64 targets = knightAttackTable[source]; //get their attack squares
        U64 quietTargets = targets & ~occupancyBitboards[both]; //get the squares that are empty
        U64 captureTargets = targets & opponentOccupancy; //get the squares with enemies on it
        while(quietTargets) { //while there are empty squares to move on
            target = get_lsb_index(quietTargets); //get one empty square
            add_move(moveList, encode_move(source, target, QUIET_FLAG)); //add a quiet knight move to there
            pop_bit(quietTargets, target); //pop bit and move on to the next empty square
        }
        while(captureTargets) { //while there are enemy pieces on those squares
            target = get_lsb_index(captureTargets); //get one square
            add_move(moveList, encode_move(source, target, CAPTURE_FLAG)); //add a capture move
            pop_bit(captureTargets, target); //pop and move to next enemy occupied square
        }
        pop_bit(knightBitboard, source); //pop the knight and move on to the next knight to do
    }
}
/*
8  - - - - - - - -
7  - - - - - - - -
6  - 1 - 1 - - - -
5  1 - - - 1 - - -
4  - - N - - - - -
3  1 - - - 1 - - -
2  - 1 - 1 - - - -
1  - - - - - - - -
*/

void generate_bishop_moves(MoveList *moveList, int side) {
    U64 bishopBitboard = (side == white) ? pieceBitboards[B] : pieceBitboards[b];
    U64 opponentOccupancy = (side == white) ? occupancyBitboards[black] : occupancyBitboards[white];
    int source, target;
    while(bishopBitboard) {
        source = get_lsb_index(bishopBitboard);
        U64 targets = get_bishop_attacks(source, occupancyBitboards[both]);
        U64 quietTargets = targets & ~occupancyBitboards[both];
        U64 captureTargets = targets & opponentOccupancy;
        while(quietTargets) {
            target = get_lsb_index(quietTargets);
            add_move(moveList, encode_move(source, target, QUIET_FLAG));
            pop_bit(quietTargets, target);
        }
        while(captureTargets) {
            target = get_lsb_index(captureTargets);
            add_move(moveList, encode_move(source, target, CAPTURE_FLAG));
            pop_bit(captureTargets, target);
        }
        pop_bit(bishopBitboard, source);
    }
}

void generate_rook_moves(MoveList *moveList, int side) {
    U64 rookBitboard = (side == white) ? pieceBitboards[R] : pieceBitboards[r];
    U64 opponentOccupancy = (side == white) ? occupancyBitboards[black] : occupancyBitboards[white];
    int source, target;
    while(rookBitboard) {
        source = get_lsb_index(rookBitboard);
        U64 targets = get_rook_attacks(source, occupancyBitboards[both]);
        U64 quietTargets = targets & ~occupancyBitboards[both];
        U64 captureTargets = targets & opponentOccupancy;
        while(quietTargets) {
            target = get_lsb_index(quietTargets);
            add_move(moveList, encode_move(source, target, QUIET_FLAG));
            pop_bit(quietTargets, target);
        }
        while(captureTargets) {
            target = get_lsb_index(captureTargets);
            add_move(moveList, encode_move(source, target, CAPTURE_FLAG));
            pop_bit(captureTargets, target);
        }
        pop_bit(rookBitboard, source);
    }
}

void generate_queen_moves(MoveList *moveList, int side) {
    U64 queenBitboard = (side == white) ? pieceBitboards[Q] : pieceBitboards[q];
    U64 opponentOccupancy = (side == white) ? occupancyBitboards[black] : occupancyBitboards[white];
    int source, target;
    while(queenBitboard) {
        source = get_lsb_index(queenBitboard);
        U64 targets = get_queen_attacks(source, occupancyBitboards[both]);
        U64 quietTargets = targets & ~occupancyBitboards[both];
        U64 captureTargets = targets & opponentOccupancy;
        while(quietTargets) {
            target = get_lsb_index(quietTargets);
            add_move(moveList, encode_move(source, target, QUIET_FLAG));
            pop_bit(quietTargets, target);
        }
        while(captureTargets) {
            target = get_lsb_index(captureTargets);
            add_move(moveList, encode_move(source, target, CAPTURE_FLAG));
            pop_bit(captureTargets, target);
        }
        pop_bit(queenBitboard, source);
    }
}

void generate_king_moves(MoveList *moveList, int side) {
    U64 kingBitboard = (side == white) ? pieceBitboards[K] : pieceBitboards[k]; //get the side's knight pieces bitboard
    U64 opponentOccupancy = (side == white) ? occupancyBitboards[black] : occupancyBitboards[white];
    int source, target;
    while(kingBitboard) { //while there are knights on the board
        source = get_lsb_index(kingBitboard); //get one knight
        U64 targets = kingAttackTable[source]; //get their attack squares
        U64 quietTargets = targets & ~occupancyBitboards[both]; //get the squares that are empty
        U64 captureTargets = targets & opponentOccupancy; //get the squares with enemies on it
        while(quietTargets) { //while there are empty squares to move on
            target = get_lsb_index(quietTargets); //get one empty square
            add_move(moveList, encode_move(source, target, QUIET_FLAG)); //add a quiet knight move to there
            pop_bit(quietTargets, target); //pop bit and move on to the next empty square
        }
        while(captureTargets) { //while there are enemy pieces on those squares
            target = get_lsb_index(captureTargets); //get one square
            add_move(moveList, encode_move(source, target, CAPTURE_FLAG)); //add a capture move
            pop_bit(captureTargets, target); //pop and move to next enemy occupied square
        }
        pop_bit(kingBitboard, source); //pop the knight and move on to the next knight to do
    }
}

void generate_moves(MoveList *moveList) {
    moveList -> count = 0;

    if(side == white) {
        //if there are white pawns left, then we generate possible moves; will save computation in later state games
        if(pieceBitboards[P]) {generate_pawn_moves(moveList, white);}
        if(pieceBitboards[N]) {generate_knight_moves(moveList, white);}
        if(pieceBitboards[B]) {generate_bishop_moves(moveList, white);}
        if(pieceBitboards[R]) {generate_rook_moves(moveList, white);}
        if(pieceBitboards[Q]) {generate_queen_moves(moveList, white);}
        generate_king_moves(moveList, white); //game's over if the king is gone lol
    } else {
        //code for black
        if(pieceBitboards[p]) {generate_pawn_moves(moveList, black);}
        if(pieceBitboards[n]) {generate_knight_moves(moveList, black);}
        if(pieceBitboards[b]) {generate_bishop_moves(moveList, black);}
        if(pieceBitboards[r]) {generate_rook_moves(moveList, black);}
        if(pieceBitboards[q]) {generate_queen_moves(moveList, black);}
        generate_king_moves(moveList, black);
    }
}

void print_move(Move move) { //have to update this function later to deal with flags such as promoted piece
    printf("%s to %s", squareToCoords[get_source(move)], squareToCoords[get_target(move)]);
}

void print_movelist(MoveList *moveList) {
    for(int i = 0; i < moveList->count; i++) {
        int move = moveList->moves[i];
        print_move(move);
        printf("\n");
    }
    printf("Number of moves in list: %d\n", moveList->count);
}
/*
#define get_source(move) ((move) & 0x3F)            returns an int from 0-63
#define get_target(move) (((move) >> 6) & 0x3F)     returns an int from 0-63
#define get_flags(move) ((move) >> 12)              returns an int from 0-15
*/