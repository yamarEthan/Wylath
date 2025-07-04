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

int make_move(Move move) { //make sure to update occupancy bitboard, piece bitboard, squareToPiece table, side, enPassant, castling, moves, and ply
    int source = get_source(move);
    int target = get_target(move);
    int flag = get_flag(move);
    int piece = squareToPiece[source];
    int opponent = side ^ 1; //XOR operator
    int capturedPiece = -1; int promotedPiece = -1;
    int rookFrom, rookTo, rookColor;

    //save_board(); write later

    pop_bit(pieceBitboards[piece], source); pop_bit(occupancyBitboards[side], source);
    set_bit(pieceBitboards[piece], target); set_bit(occupancyBitboards[side], target);
    squareToPiece[source] = -1;

    //if king or rook moves, update castling rights
    if(piece == K) {castlingRights &= ~(wk | wq);}
    if(piece == k) {castlingRights &= ~(bk | bq);}
    if(piece == R) { if(source == H1) {castlingRights &= ~wk;} else if(source == A1) {castlingRights &= ~wq;} }
    if(piece == r) { if(source == H8) {castlingRights &= ~bk;} else if(source == A8) {castlingRights &= ~bq;} }

    switch(flag) {
        case QUIET_FLAG:
            squareToPiece[target] = piece;
            break;
        case DOUBLE_PUSH_FLAG:
            squareToPiece[target] = piece;
            enPassant = (side == white) ? target - 8 : target + 8;
            break;
        case KING_CASTLE_FLAG:
            squareToPiece[target] = piece;
            rookTo = (side == white) ? F1 : F8; rookFrom = (side == white) ? H1 : H8; rookColor = (side == white) ? R : r;
            pop_bit(pieceBitboards[rookColor], rookFrom); pop_bit(occupancyBitboards[side], rookFrom);
            set_bit(pieceBitboards[rookColor], rookTo); set_bit(occupancyBitboards[side], rookTo);
            squareToPiece[rookTo] = rookColor; squareToPiece[rookFrom] = -1;          
            break;
        case QUEEN_CASTLE_FLAG:
            squareToPiece[target] = piece;
            rookTo = (side == white) ? D1 : D8; rookFrom = (side == white) ? A1 : A8; rookColor = (side == white) ? R : r;
            pop_bit(pieceBitboards[rookColor], rookFrom); pop_bit(occupancyBitboards[side], rookFrom);
            set_bit(pieceBitboards[rookColor], rookTo); set_bit(occupancyBitboards[side], rookTo);
            squareToPiece[rookTo] = rookColor; squareToPiece[rookFrom] = -1;
            break;
        case CAPTURE_FLAG:
            capturedPiece = squareToPiece[target];
            if (capturedPiece == R) { if (target == H1) {castlingRights &= ~wk;} else if (target == A1) {castlingRights &= ~wq;} }
            if (capturedPiece == r) { if (target == H8) {castlingRights &= ~bk;} else if (target == A8) {castlingRights &= ~bq;} }
            pop_bit(pieceBitboards[capturedPiece], target); pop_bit(occupancyBitboards[opponent], target);
            squareToPiece[target] = piece;
            break;
        case EP_CAPTURE_FLAG:
            int pawnSquare = (side == white) ? target - 8 : target + 8; //gets location of pawn being captured
            pop_bit(pieceBitboards[(piece + 6) % 12], pawnSquare); pop_bit(occupancyBitboards[opponent], pawnSquare); //(0 + 6) % 12 = 6 (black pawn); (6 + 6) % 12 = 0 (white pawn)
            squareToPiece[pawnSquare] = -1; squareToPiece[target] = piece;
            break;
        case KNIGHT_PROMO_FLAG: case BISHOP_PROMO_FLAG: case ROOK_PROMO_FLAG: case QUEEN_PROMO_FLAG:
            promotedPiece = (side == white) ? flag - 7 : flag - 1; //if white, goes to 1-4 (N, B, R, Q); if black, goes to 7-10 (n, b, r, q)
            pop_bit(pieceBitboards[piece], target);
            set_bit(pieceBitboards[promotedPiece], target); //already set occupancy above switch case
            squareToPiece[target] = promotedPiece;
            break;
        case NP_CAPTURE_FLAG: case BP_CAPTURE_FLAG: case RP_CAPTURE_FLAG: case QP_CAPTURE_FLAG:
            capturedPiece = squareToPiece[target];
            if (capturedPiece == R) { if (target == H1) {castlingRights &= ~wk;} else if (target == A1) {castlingRights &= ~wq;} }
            if (capturedPiece == r) { if (target == H8) {castlingRights &= ~bk;} else if (target == A8) {castlingRights &= ~bq;} }
            pop_bit(pieceBitboards[piece], target); pop_bit(pieceBitboards[capturedPiece], target); pop_bit(occupancyBitboards[opponent], target);

            promotedPiece = (side == white) ? flag - 11 : flag - 5;
            set_bit(pieceBitboards[promotedPiece], target); //already set occupancy
            squareToPiece[target] = promotedPiece;
            break;
        default:
            printf("\nError in make_move(): wrong flag\n");
            break;
    }

    occupancyBitboards[both] = occupancyBitboards[white] | occupancyBitboards[black];
    if(piece == P || piece == p || flag == CAPTURE_FLAG || flag == EP_CAPTURE_FLAG || flag >= NP_CAPTURE_FLAG) {ply = 0;} else {ply++;}
    if(side == black) {fullMoves++;}
    if(flag != DOUBLE_PUSH_FLAG) {enPassant = NO_SQUARE;} //if no double pawn push, reset enPassant variable

    side ^= 1;
    return 1;
}

void generate_pawn_moves(MoveList *moveList, int side) { //the add_move function already increases count so no need to worry about that in here
    U64 pawnBitboard = (side == white) ? pieceBitboards[P] : pieceBitboards[p];
    int source, target;
    if(side == white) {
        while(pawnBitboard) {
            source = get_lsb_index(pawnBitboard);
            target = source + 8;
            if(!(target > H8) && !get_bit(occupancyBitboards[both], target)) { //if the target does not go past the board (H8 = 63) and the target square is empty,
                if(source >= A7 && source <= H7) { //if it's on the 7th rank, we can promote
                    add_move(moveList, encode_move(source, target, QUEEN_PROMO_FLAG)); add_move(moveList, encode_move(source, target, ROOK_PROMO_FLAG));
                    add_move(moveList, encode_move(source, target, BISHOP_PROMO_FLAG)); add_move(moveList, encode_move(source, target, KNIGHT_PROMO_FLAG));        
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
                    add_move(moveList, encode_move(source, target, QP_CAPTURE_FLAG)); add_move(moveList, encode_move(source, target, RP_CAPTURE_FLAG));
                    add_move(moveList, encode_move(source, target, BP_CAPTURE_FLAG)); add_move(moveList, encode_move(source, target, NP_CAPTURE_FLAG));
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
                    add_move(moveList, encode_move(source, target, QP_CAPTURE_FLAG)); add_move(moveList, encode_move(source, target, RP_CAPTURE_FLAG));
                    add_move(moveList, encode_move(source, target, BP_CAPTURE_FLAG)); add_move(moveList, encode_move(source, target, NP_CAPTURE_FLAG));
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
                    add_move(moveList, encode_move(source, target, QP_CAPTURE_FLAG)); add_move(moveList, encode_move(source, target, RP_CAPTURE_FLAG));
                    add_move(moveList, encode_move(source, target, BP_CAPTURE_FLAG)); add_move(moveList, encode_move(source, target, NP_CAPTURE_FLAG));
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