#include <stdio.h>
#include <stdlib.h>  // for random()
#include <string.h>  // for memset()
#include "definitions.h"
#include "attacks.h"

//The functions here were taken from https://www.chessprogramming.org/Looking_for_Magics and slightly modified
//In the future, I want to replace so of the magic numbers I found with the numbers here https://www.chessprogramming.org/Best_Magics_so_far as they are more efficient

//Since this code will not be used after the magic numbers have been found, it will not be compiled with the rest of the code

const int bishopBitsSeen[64] = { //given a bishop on a square, this tells you how many squares it looks at
    6, 5, 5, 5, 5, 5, 5, 6, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    5, 5, 7, 7, 7, 7, 5, 5, 
    5, 5, 7, 9, 9, 7, 5, 5, 
    5, 5, 7, 9, 9, 7, 5, 5, 
    5, 5, 7, 7, 7, 7, 5, 5, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    6, 5, 5, 5, 5, 5, 5, 6
};

const int rookBitsSeen[64] = { //for rooks
    12, 11, 11, 11, 11, 11, 11, 12, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    12, 11, 11, 11, 11, 11, 11, 12
};

U64 random_u64() {
  U64 u1, u2, u3, u4;
  u1 = (U64)(random()) & 0xFFFF; u2 = (U64)(random()) & 0xFFFF;
  u3 = (U64)(random()) & 0xFFFF; u4 = (U64)(random()) & 0xFFFF;
  return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}

U64 random_u64_fewbits() {
  return random_u64() & random_u64() & random_u64();
}

U64 find_magic(int square, int bitsSeen, int bishop) { //for every square

  U64 blockersBitboard[4096]; //we create an array of all possible permutations of blockers, with 4096 being the largest amount from a rook in a corner square
  U64 attacksTable[4096]; //with the blocker mask and the blocker bitboard, we have the corresponding attack bitboard
  U64 usedAttacksTable[4096]; //

  U64 blockersMask = bishop ? bishop_blockers_mask(square) : rook_blockers_mask(square);

  int blockersIndices = 1 << bitsSeen;

  for(int i = 0; i < blockersIndices; i++) {
    blockersBitboard[i] = get_blockers_bitboard(i, bitsSeen, blockersMask); //why always the max number of bits seen????
    attacksTable[i] = bishop ? bishop_attacks_mask(square, blockersBitboard[i]) : rook_attacks_mask(square, blockersBitboard[i]);
  }

  for(int k = 0; k < 100000000; k++) {
    U64 magicNumber = random_u64_fewbits();
    if(count_bits((blockersMask * magicNumber) & 0xFF00000000000000ULL) < 6) continue; //in bbc, the ULL is removed

    memset(usedAttacksTable, 0ULL, sizeof(usedAttacksTable)); //initializes all entries in the array as 0ULL

    int index, fail;
    for(index = 0, fail = 0; !fail && index < blockersIndices; index++) {
      int magicIndex = (int)((blockersBitboard[index] * magicNumber) >> (64 - bitsSeen));

      if(usedAttacksTable[magicIndex] == 0ULL) {
        usedAttacksTable[magicIndex] = attacksTable[index];
      } else if(usedAttacksTable[magicIndex] != attacksTable[index]) {
        fail = 1;
      }
    }

    if(!fail) return magicNumber;
  }

  printf("***MAGIC NUMBER FAILS***\n");
  return 0ULL;
}

int main() {
  printf("const uint64 RMagic[64] = {\n");
  for(int square = 0; square < 64; square++)
    printf("  0x%llxULL,\n", find_magic(square, bishopBitsSeen[square], 0));
  printf("};\n\n");

  printf("const uint64 BMagic[64] = {\n");
  for(int square = 0; square < 64; square++)
    printf("  0x%llxULL,\n", find_magic(square, rookBitsSeen[square], 1));
  printf("};\n\n");

  return 0;
}