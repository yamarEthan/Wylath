#include <stdio.h>
#include "definitions.h"
#include "display.h"


void print_bitboard(U64 bitboard) {
    printf("----------------------\n");
    for (int rank = 7; rank >= 0; rank--) {
        for(int file = 0; file < 8; file++) {
                int square = rank * 8 + file;

                if(!file) {printf("  %d ", rank + 1);}
                printf(" %d", get_bit(bitboard, square) ? 1 : 0);
        }
        printf("\n");
    }
    printf("\n     a b c d e f g h\n");
    printf("----------------------\n");

}