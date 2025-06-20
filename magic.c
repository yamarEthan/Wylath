#include <stdio.h>
#include <stdlib.h>  // for random()
#include <string.h>  // for memset()
#include "definitions.h"
#include "attacks.h"

//The functions here were taken from https://www.chessprogramming.org/Looking_for_Magics and slightly modified
//In the future, I want to replace so of the magic numbers I found with the numbers here https://www.chessprogramming.org/Best_Magics_so_far as they are more efficient

//Since this code will not be used after the magic numbers have been found, it will not be compiled with the rest of the code
//to recompile it's just magic.c and attacks.c

U64 random_u64() {
	U64 u1, u2, u3, u4;
	u1 = (U64)(random()) & 0xFFFF;
	u2 = (U64)(random()) & 0xFFFF;
	u3 = (U64)(random()) & 0xFFFF;
	u4 = (U64)(random()) & 0xFFFF;
	return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}

U64 random_u64_fewbits() {
	return random_u64() & random_u64() & random_u64();
}

U64 find_magic(int square, int bitsSeen, int bishop) { //for every square

	U64 blockerBitboard[4096]; //we create an array of all possible permutations of blockers, with 4096 being the largest amount from a rook in a corner square
	U64 attackTable[4096]; //with the blocker mask and the blocker bitboard, we have the corresponding attack bitboard
	U64 usedAttackTable[4096]; //

	U64 blockerMask = bishop ? bishop_blocker_mask(square) : rook_blocker_mask(square);

	int bitsInMask = count_bits(blockerMask);
	int blockerIndices = 1 << bitsInMask; //total possible combinations of blockers given how many bits in the mask

	for(int i = 0; i < blockerIndices; i++) {
		blockerBitboard[i] = get_blocker_bitboard(i, bitsInMask, blockerMask);
		attackTable[i] = bishop ? bishop_attack_mask(square, blockerBitboard[i]) : rook_attack_mask(square, blockerBitboard[i]);
	}

	for(int k = 0; k < 100000000; k++) { //arbitrary high number of guesses
		U64 magicNumber = random_u64_fewbits();
		if(count_bits((blockerMask * magicNumber) & 0xFF00000000000000ULL) < 6) {continue;} //in bbc, the ULL is removed

		memset(usedAttackTable, 0ULL, sizeof(usedAttackTable)); //initializes all entries in the array as 0ULL

		int index, fail;
		for(index = 0, fail = 0; !fail && index < blockerIndices; index++) {
			int magicIndex = (int)((blockerBitboard[index] * magicNumber) >> (64 - bitsSeen));

			if(usedAttackTable[magicIndex] == 0ULL) {usedAttackTable[magicIndex] = attackTable[index];
			} else if(usedAttackTable[magicIndex] != attackTable[index]) {fail = 1;}
		}

		if(!fail) {return magicNumber;}
	}

	printf("***MAGIC NUMBER FAILS***\n");
	return 0ULL;
}

int main() {
	printf("extern U64 BishopMagicNumbers[64] = {\n");
	for(int square = 0; square < 64; square++) {printf("  0x%llxULL,\n", find_magic(square, bishopBitsSeen[square], 1));}
	printf("};\n\n");

	printf("extern U64 RookMagicNumbers[64] = {\n");
	for(int square = 0; square < 64; square++) {printf("  0x%llxULL,\n", find_magic(square, rookBitsSeen[square], 0));}
	printf("};\n\n");

	return 0;
}