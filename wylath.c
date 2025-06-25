#include <stdio.h>
#include "definitions.h"
#include "display.h"
#include "attacks.h"
#include "moves.h"

//naming schemes:
//variables: camelCase
//functions: lowercase_underline_spaced

int main() {

    parse_fen(START_POSITION);
    for(int i = 0; i < 12; i++) {
        print_bitboard(pieceBitboards[i]);
    }

    init_leapers_attacks(); //initialize pawn, knight, and king attacks for every square
    init_sliders_attacks(1); //initialize bishop attacks
    init_sliders_attacks(0); //initialize rook attacks

    print_board();

    generate_moves(&pseudoMoves);

    print_movelist(&pseudoMoves);
    
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