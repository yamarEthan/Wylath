#include <stdio.h>
#include "definitions.h"
#include "display.h"
#include "attacks.h"

//This is for finding the magic numbers used in magic bitboards
//Since this code will not be used after the numbers have been found, it will not be compiled with the rest of the code

U64 bishopBlockerMasks[64];
U64 rookBlockerMasks[64];

U64 bishopAttacksTable[64][512]; //the most number of squares a bishop can see is 9, so there is 2^9=512 possible permutations of blockers
U64 rookAttacksTable[64][4096]; //2^12=4096



int main() {
    for(int i = 0; i < 64; i++) {
        bishopBlockerMasks[i] = bishop_blockers_mask(i);
        rookBlockerMasks[i] = rook_blockers_mask(i);
    }
}