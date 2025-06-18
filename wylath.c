#include <stdio.h>
#include "definitions.h"
#include "display.h"
#include "attacks.h"

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
}
