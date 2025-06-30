#include <stdio.h>
#include "definitions.h"
#include "display.h"
#include "attacks.h"
#include "moves.h"

//naming schemes:
//variables: camelCase
//functions: lowercase_underline_spaced

int main() {
    parse_fen(MAX_MOVES);

    init_leapers_attacks(); //initialize pawn, knight, and king attacks for every square
    init_sliders_attacks(1); //initialize bishop attacks
    init_sliders_attacks(0); //initialize rook attacks

    print_board();

    generate_moves(&pseudoMoves);

    print_movelist(&pseudoMoves);
}