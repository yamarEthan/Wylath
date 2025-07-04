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

    init_leapers_attacks(); //initialize pawn, knight, and king attacks for every square
    init_sliders_attacks(1); //initialize bishop attacks
    init_sliders_attacks(0); //initialize rook attacks

    print_board();

    generate_moves(&pseudoMoves);
    make_move(5835); //e2e4
    print_board();

    generate_moves(&pseudoMoves);
    make_move(6387);
    print_board();

    generate_moves(&pseudoMoves);
    print_movelist(&pseudoMoves);
}


//pseudo move generation is basically finished. We may want to update the print_move function to also print info of a flag it has

//other than that, we move on to making the make move function. We aren't actually choosing a move yet, just that after a move has been chosen, we make that move,
//updating all game state variables and bitboards and such.

//the first step is to make a function that copies the current board state as well as be able to restore the board state with this copy.
//this will be useful if a move made is illegal and for our search algorithms after having explored a certain depth