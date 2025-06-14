#include <stdio.h>
#include "definitions.h"
#include "display.h"
#include "attacks.h"

//naming schemes:
//variables: camelCase
//functions: lowercase_underline_spaced

U64 pieceBitboards[12]; //each piece type will have their own bitboard; order is P, N, B, R, Q, K, p, n, b, r, q, k

int side; //side to move
int enPassant; //en Passant square
int castlingRights; //castling rights

U64 positionKey; //current set up of Board

int fullMoves; //number of full plys played in total in the game

int ply; //half move; white/black plays a move but not the other side; resets after a pawn push or piece capture; used for 50-move draw rule

/*FEN has 6 fields:
    1. Piece Placement; lowercase for black, uppercase for white
    2. Active Color (b/w/-)
    3. Castling Rights (KQkq) in this order and remove letters accordingly if that right is loss; - if no castling anymore
    4. En Passant Target Square: if a pawn moves twice, the square it skips is immediately put here even if no pawn can capture it
    5. Halfmove Clock: resets to 0 after a pawn move or piece capture; useful for the 50-move draw rule
    6. Fullmove Number: increments everytime black plays a move

    Example: Starting Position FEN is: "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
*/
int charToPiece[] = { //useful for parsing a FEN string
    ['P'] = P,
    ['N'] = N,
    ['B'] = B,
    ['R'] = R,
    ['Q'] = Q,
    ['K'] = K,
    ['p'] = p,
    ['n'] = n,
    ['b'] = b,
    ['r'] = r,
    ['q'] = q,
    ['k'] = k
};

//given a fen string: sets the piece bitboards, side, enPassant, castling rights, and fullmove and ply counter
void parse_fen(char *fen) { //may have issues with unknown characters or overflow
    for(int rank = 7; rank >= 0; rank--) {
        for(int file = 0; file < 8; file++) {
            int square = (rank * 8) + file;
            if((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z')) {
                int piece = charToPiece[(unsigned char)*fen];
                set_bit(pieceBitboards[piece], square);
                fen++;
            }
            if((*fen >= '0' && *fen <= '9')) {
                int blanks = *fen - '0'; //converts char to int
                file += blanks;
                fen++;
            }
            if (*fen == '/') {
                fen++;
            }
        }
    }

    fen++; //move to Active Color field
    (*fen == 'w') ? (side = white) : (side = black);

    fen += 2; //move to Castling Rights field
    while(*fen != ' ') {
        switch(*fen) {
            case 'K': castlingRights |= wk; break;
            case 'Q': castlingRights |= wq; break;
            case 'k': castlingRights |= bk; break;
            case 'q': castlingRights |= bq; break;
            case '-': break;
        }
        fen++;
    }

    fen++; //move to enpassant field
    if(*fen != '-') {
        int file = *fen - 'a';
        fen++;
        int rank = *fen - '0';
        enPassant = ((rank - 1) * 8) + file; //subtract one due to zero based indexing
        //printf("\nEn Passant Square: %d\n", enPassant); //keep for now just in case
    } else {
        enPassant = NO_SQUARE;
    }

    fen += 2; //move to halfmove field
    if(*fen != ' ') {
        if(*(fen + 1) != ' ') {
            if(*(fen + 2) != ' ') {
                ply = 100; //if there's three digits, it's either >= 100, and fifty-move draw rule applies in any case
            } else {
                int tens = *fen++ - '0';
                int units = *fen - '0';
                ply = (tens * 10) + units;
            }
        } else {
            ply = *fen - '0';
        }
    } else {
        ply = 0;
    }

    //skipping fullmove number for now as there may be cases where it goes all the way to four digit counts
}

int main() {
    /*parse_fen(START_POSITION);
    for(int i = 0; i < 12; i++) {
        print_bitboard(pieceBitboards[i]);
    }*/
    

    /*for(int i = 0; i < 64; i++) { //prints all attack masks of black pawns
        printf("\nSquare: %d\n", i);
        U64 Pawn = pawn_attacks_mask(i, black);
        print_bitboard(Pawn);
    } 
    */

    /*for(int i = 0; i < 64; i++) {
        printf("\nSquare: %d\n", i);
        U64 Knight = knight_attacks_mask(i);
        print_bitboard(Knight);
    }
    */

    /*for(int i = 0; i < 64; i++) {
        printf("\nSquare: %d\n", i);
        U64 King = king_attacks_mask(i);
        print_bitboard(King);
    }
    */

    /*for(int i = 0; i < 64; i++) { //prints all attack masks of bishops and rooks
        //printf("\nSquare: %d\n", i);
        //U64 Bishop = bishop_blockers_mask(i);
        U64 Rook = rook_blockers_mask(i);
        //print_bitboard(Rook);
        printf("\n%d\n", count_bits(Rook));
        //printf("\n%d", get_lsb_index(Rook));
        //print_bitboard(Bishop);
    }*/
    
    U64 Rook = rook_blockers_mask(A1);
    for(int i = 4080; i < 4096; i++) {
        U64 permutation = get_blockers_bitboard(i, count_bits(Rook), Rook);
        print_bitboard(permutation);
    }
}
