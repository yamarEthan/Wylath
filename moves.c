#include <stdio.h>
#include "definitions.h"
#include "attacks.h"
#include "display.h"
#include "moves.h"

MoveList pseudoMoves;


void generate_pawn_moves(MoveList *moveList, U64 pawnBitboard, int side) { //we want to update moveList's count as well in each function
    //now how do we generate pawn moves? we need to deal with:

    //pawn push one square: check that the square is empty using occupancyBitboard[3]

    //pawn push two square: check it's at starting rank AND both squares are empty

    //pawn attacks: check that there's enemies on its diagonals
    
    //promotion: check if reaching the end ranks

    //something for enPassant
}

void generate_knight_moves(MoveList *moveList, U64 knightBitboard) {

}

void generate_bishop_moves(MoveList *moveList, U64 bishopBitboard) {

}

void generate_rook_moves(MoveList *moveList, U64 rookBitboard) {

}

void generate_queen_moves(MoveList *moveList, U64 queenBitboard) {

}

void generate_king_moves(MoveList *moveList, U64 kingBitboard) {

}

void generate_moves(MoveList *moveList) {
    moveList -> count = 0;

    if(side == white) {
        //if there are white pawns left, then we generate possible moves; will save computation in later state games
        if(pieceBitboards[P]) {generate_pawn_moves(moveList, pieceBitboards[P], white);}
        if(pieceBitboards[N]) {generate_knight_moves(moveList, pieceBitboards[N]);}
        if(pieceBitboards[B]) {generate_bishop_moves(moveList, pieceBitboards[B]);}
        if(pieceBitboards[R]) {generate_rook_moves(moveList, pieceBitboards[R]);}
        if(pieceBitboards[Q]) {generate_queen_moves(moveList, pieceBitboards[Q]);}
        generate_king_moves(moveList, pieceBitboards[K]); //game's over if the king is gone lol
    } else {
        //code for black
        if(pieceBitboards[p]) {generate_pawn_moves(moveList, pieceBitboards[p], black);}
        if(pieceBitboards[n]) {generate_knight_moves(moveList, pieceBitboards[n]);}
        if(pieceBitboards[b]) {generate_bishop_moves(moveList, pieceBitboards[b]);}
        if(pieceBitboards[r]) {generate_rook_moves(moveList, pieceBitboards[r]);}
        if(pieceBitboards[q]) {generate_queen_moves(moveList, pieceBitboards[q]);}
        generate_king_moves(moveList, pieceBitboards[k]); //game's over if the king is gone lol
    }
}
