#include <stdio.h>
#include "definitions.h"
#include "display.h"

U64 pieceBitboards[12]; //each piece type will have their own bitboard; order is P, N, B, R, Q, K, p, n, b, r, q, k
U64 occupancyBitboards[3]; //contains the location of all pieces of white, black, and both

int squareToPiece[64]; //keeps track of what piece is in what square; initialized by parse_fen and updated by make_move; -1 if empty, 0-11 for piece type

int side; //side to move
int enPassant = NO_SQUARE; //en Passant square
int castlingRights; //castling rights

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

char *squareToCoords[] = {
    "A1", "B1", "C1", "D1", "E1", "F1", "G1", "H1",
    "A2", "B2", "C2", "D2", "E2", "F2", "G2", "H2",
    "A3", "B3", "C3", "D3", "E3", "F3", "G3", "H3",
    "A4", "B4", "C4", "D4", "E4", "F4", "G4", "H4",
    "A5", "B5", "C5", "D5", "E5", "F5", "G5", "H5",
    "A6", "B6", "C6", "D6", "E6", "F6", "G6", "H6",
    "A7", "B7", "C7", "D7", "E7", "F7", "G7", "H7",
    "A8", "B8", "C8", "D8", "E8", "F8", "G8", "H8",
};

char pieceToChar[12] = "PNBRQKpnbrqk"; //useful for printing the board

char *unicode_pieces[12] = { //may have issues if the computer it's running on doesn't support these, so switch to the chars above if that happens in print_board()
    "\u265F", "\u265E", "\u265D", "\u265C", "\u265B", "\u265A",  // the black pieces unicode gives looks like white pieces, so i switched their representation
    "\u2659", "\u2658", "\u2657", "\u2656", "\u2655", "\u2654"   // vice versa
};


void init_square_to_piece() {
    for(int i = 0; i < 64; i++) {squareToPiece[i] = -1;}

    for(int i = P; i <= k; i++) {
        U64 bitboard = pieceBitboards[i];
        while(bitboard) {
            int square = get_lsb_index(bitboard);
            squareToPiece[square] = i;
            pop_bit(bitboard, square);
        }
    }
}

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
            }
            if((*fen >= '0' && *fen <= '9')) {
                int blanks = *fen - '0'; //converts char to int
                file += blanks - 1;
            }
            fen++;
        }
        if (*fen == '/') {
            fen++;
        }
    }

    init_square_to_piece();

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

void print_board() {
    printf("----------------------\n");
    for(int rank = 7; rank >= 0; rank--) {
        for(int file = 0; file < 8; file++) {
            int square = (rank * 8) + file;
            
            if(!file) {printf("  %d ", rank + 1);}
            int piece = -1;
            for(int i = P; i <= k; i++) {
                if(get_bit(pieceBitboards[i], square)) {
                    piece = i;
                    //printf(" %s", unicode_pieces[piece]);
                    printf(" %c", pieceToChar[piece]); //switch to this if unicode pieces do not print right
                    break;                    
                }
            }
            if(piece == -1) {printf(" -");}
            }
        printf("\n");
    }
    printf("\n     a b c d e f g h\n----------------------\n\n");
    printf("Side to Move: %s\n", !side ? "White" : "Black");
    printf("En Passant: %s\n", (enPassant != NO_SQUARE) ? squareToCoords[enPassant] : "None");
    printf("Castling: %c%c%c%c\n", (castlingRights & wk) ? 'K': '-', (castlingRights & wq) ? 'Q': '-', (castlingRights & bk) ? 'k': '-', (castlingRights & bq) ? 'q': '-');
}

void print_move(Move move) { //have to update this function later to deal with flags such as promoted piece
    printf("%s to %s: ", squareToCoords[get_source(move)], squareToCoords[get_target(move)]);
    for (int i = 15; i >= 0; i--) {printf("%d", (move >> i) & 1);}
    printf(": %u\n", move);
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