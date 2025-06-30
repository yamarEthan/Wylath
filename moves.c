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
        U64 google = (side == white) ? pawnAttackTable[black][enPassant] & pieceBitboards[P] : pawnAttackTable[white][enPassant] & pieceBitboards[p];
        while(google) {
            source = get_lsb_index(google);
            add_move(moveList, encode_move(source, enPassant, EP_CAPTURE_FLAG));
            pop_bit(google, source);
        }
    }
}

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

void generate_queen_moves(MoveList *moveList, int side) { //fix queen moves
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

void generate_king_moves(MoveList *moveList, int side) { //there's only one king per side
    U64 kingBitboard = (side == white) ? pieceBitboards[K] : pieceBitboards[k];
    U64 opponentOccupancy = (side == white) ? occupancyBitboards[black] : occupancyBitboards[white];
    int source, target;
    source = get_lsb_index(kingBitboard);

    U64 targets = kingAttackTable[source];
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
    if(side == white) {
        if(castlingRights & wk) { //think of castlingRights as 4 bits 0000 where each bit on is a castling right
            if(!get_bit(occupancyBitboards[both], F1) && !get_bit(occupancyBitboards[both], G1)) { //if the squares are empty
                if(!is_square_attacked(E1, black) && !is_square_attacked(F1, black) && !is_square_attacked(G1, black)) { //if the squares aren't attacked
                    add_move(moveList, encode_move(E1, G1, KING_CASTLE_FLAG)); //castle
                }
            }
        }
        if(castlingRights & wq) { //repeat with other side
            if(!get_bit(occupancyBitboards[both], D1) && !get_bit(occupancyBitboards[both], C1) && !get_bit(occupancyBitboards[both], B1)) {
                if(!is_square_attacked(D1, black) && !is_square_attacked(C1, black)) { //don't need to check if B1 is attacked as king castles to C1
                    add_move(moveList, encode_move(E1, C1, QUEEN_CASTLE_FLAG));
                }
            }
        }
    } else {
        if(castlingRights & bk) {
            if(!get_bit(occupancyBitboards[both], F8) && !get_bit(occupancyBitboards[both], G8)) {
                if(!is_square_attacked(E8, white) && !is_square_attacked(F8, white) && !is_square_attacked(G8, white)) {
                    add_move(moveList, encode_move(E8, G8, KING_CASTLE_FLAG));
                }
            }
        }
        if(castlingRights & bq) {
            if(!get_bit(occupancyBitboards[both], D8) && !get_bit(occupancyBitboards[both], C8) && !get_bit(occupancyBitboards[both], B8)) {
                if(!is_square_attacked(D8, white) && !is_square_attacked(C8, white)) {
                    add_move(moveList, encode_move(E8, C8, QUEEN_CASTLE_FLAG));
                }
            }
        }
    }
}
/*
typedef enum {
    A1, B1, C1, D1, E1, F1, G1, H1,     //00, 01, 02, 03, 04, 05, 06, 07,
    A2, B2, C2, D2, E2, F2, G2, H2,     //08, 09, 10, 11, 12, 13, 14, 15,
    A3, B3, C3, D3, E3, F3, G3, H3,     //16, 17, 18, 19, 20, 21, 22, 23,
    A4, B4, C4, D4, E4, F4, G4, H4,     //24, 25, 26, 27, 28, 29, 30, 31,
    A5, B5, C5, D5, E5, F5, G5, H5,     //32, 33, 34, 35, 36, 37, 38, 39,
    A6, B6, C6, D6, E6, F6, G6, H6,     //40, 41, 42, 43, 44, 45, 46, 47,
    A7, B7, C7, D7, E7, F7, G7, H7,     //48, 49, 50, 51, 52, 53, 54, 55,
    A8, B8, C8, D8, E8, F8, G8, H8,     //56, 57, 58, 59, 60, 61, 62, 63,
    NO_SQUARE                           //64
} Square; //board squares
*/

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