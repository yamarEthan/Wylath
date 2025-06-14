#include <stdio.h>
#include "definitions.h"
#include "display.h"


void print_bitboard(U64 bitboard) {
    printf("----------------------\n");
    for (int rank = 7; rank >= 0; rank--) {
        for(int file = 0; file < 8; file++) {
                int square = (rank * 8) + file;

                if(!file) {printf("  %d ", rank + 1);}
                //printf(" %d", get_bit(bitboard, square) ? 1 : 0); original print bit but I want to add some color to the printing of the 1s and 0s
                int bit = get_bit(bitboard, square) ? 1 : 0;
                if(bit) {printf(" " BRIGHT_GREEN "1" RESET);} else {printf(" 0");}
        }
        printf("\n");
    }
    printf("\n     a b c d e f g h\n");
    printf("Unsigned Decimal Number Form: %llu\n", bitboard);
    printf("----------------------\n");
}


/*
  8  0 0 0 0 0 0 0 0
  7  0 0 0 0 0 0 0 0
  6  0 0 0 0 0 0 0 0
  5  0 0 0 0 0 0 0 0
  4  0 0 0 0 0 0 0 0
  3  0 0 0 0 0 0 0 0
  2  0 0 0 1 0 0 0 0
  1  0 0 0 0 0 0 0 0

     a b c d e f g h
*/