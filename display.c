#include <stdio.h>
#include "definitions.h"
#include "display.h"

U64 pieceBitboards[12]; //each piece type will have their own bitboard; order is P, N, B, R, Q, K, p, n, b, r, q, k
U64 occupancyBitboards[3]; //contains the location of all pieces of white, black, and both

int side; //side to move
int enPassant = NO_SQUARE; //en Passant square
int castlingRights; //castling rights

U64 positionKey; //current set up of Board

int fullMoves; //number of full plys played in total in the game

int ply; //half move; white/black plays a move but not the other side; resets after a pawn push or piece capture; used for 50-move draw rule

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

char pieceToChar[12] = "PNBRQKpnbrqk"; //useful for printing the board
char *unicode_pieces[12] = { //may have issues if the computer it's running on doesn't support these, so switch to the chars above if that happens in print_board()
    "\u265F", "\u265E", "\u265D", "\u265C", "\u265B", "\u265A",  // the black pieces unicode gives looks like white pieces, so i switched their representation
    "\u2659", "\u2658", "\u2657", "\u2656", "\u2655", "\u2654"   // vice versa
};

/*FEN has 6 fields:
    1. Piece Placement; lowercase for black, uppercase for white
    2. Active Color (b/w/-)
    3. Castling Rights (KQkq) in this order and remove letters accordingly if that right is loss; - if no castling anymore
    4. En Passant Target Square: if a pawn moves twice, the square it skips is immediately put here even if no pawn can capture it
    5. Halfmove Clock: resets to 0 after a pawn move or piece capture; useful for the 50-move draw rule
    6. Fullmove Number: increments everytime black plays a move

    Example: Starting Position FEN is: "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
*/

//given a fen string: sets the piece and occupancies bitboards, side, enPassant, castling rights, and fullmove and ply counter
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

    //set up occupancies bitboards
    for(int i = P; i <= K; i++) {
        occupancyBitboards[white] |= pieceBitboards[i];
    }
    for(int i = p; i <= k; i++) {
        occupancyBitboards[black] |= pieceBitboards[i];
    }

    occupancyBitboards[both] |= occupancyBitboards[white];
    occupancyBitboards[both] |= occupancyBitboards[black];
    
    //skipping fullmove number for now as there may be cases where it goes all the way to four digit counts
}

void print_bitboard(U64 bitboard) {
    printf("----------------------\n");
    for(int rank = 7; rank >= 0; rank--) {
        for(int file = 0; file < 8; file++) {
                int square = (rank * 8) + file;

                if(!file) {printf("  %d ", rank + 1);}
                //printf(" %d", get_bit(bitboard, square) ? 1 : 0); original print bit but I want to add some color to the printing of the 1s and 0s
                int bit = get_bit(bitboard, square) ? 1 : 0;
                if(bit) {printf(" " BRIGHT_GREEN "1" RESET);} else {printf(RESET " 0");}
        }
        printf("\n");
    }
    printf("\n     a b c d e f g h\n");
    printf("Unsigned Decimal Number Form: %llu\n", bitboard);
    printf("----------------------\n");
}

void print_board() { //update later to also print game state variables
    printf("----------------------\n");
    for(int rank = 7; rank >= 0; rank--) {
        for(int file = 0; file < 8; file++) {
            int square = (rank * 8) + file;
            
            if(!file) {printf("  %d ", rank + 1);}
            int piece = -1;
            for(int i = P; i <= k; i++) {
                if(get_bit(pieceBitboards[i], square)) {
                    piece = i;
                    printf(" %s", unicode_pieces[piece]);
                    //printf(" %c", pieceToChar[piece]); //switch to this if unicode pieces do not print right
                    break;                    
                }
            }
            if(piece == -1) {printf(" -");}
            }
        printf("\n");
    }
    printf("\n     a b c d e f g h\n----------------------\n");
}

/*
  8  0 0 0 0 0 0 0 0
  7  0 0 0 0 0 0 0 0
  6  0 0 0 0 0 0 0 0
  5  0 0 0 0 0 0 0 0
  4  0 0 0 0 0 0 0 0
  3  0 0 0 0 0 0 0 0
  2  0 0 0 0 0 0 0 0
  1  0 0 0 0 0 0 0 0

     a b c d e f g h
*/