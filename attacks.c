#include <stdio.h>
#include "definitions.h"
#include "attacks.h"

U64 pawnAttacksTable[2][64]; //contains the attack table for pawns in both sides and for each square [side][square]
U64 knightAttacksTable[64];
U64 kingAttacksTable[64];

U64 bishopBlockersMask[64];
U64 rookBlockersMask[64];

U64 bishopAttacksTable[64][512];
U64 rookAttacksTable[64][4096];

U64 pawn_attacks_mask(int square, int side) { //have yet to initialize pawnAttacksTable using this function for all squares and sides
    U64 bitboard = 0ULL; //empty bitboard
    U64 attackBitboard = 0ULL;
    set_bit(bitboard, square);

    if(!side) { //if white
        attackBitboard |= (bitboard << 7) & (NOT_H_FILE); //if the shifted bit doesn't end on the H file, accept it; if it is on H file, that means the attack square
        attackBitboard |= (bitboard << 9) & (NOT_A_FILE); //moved up two ranks, which is wrong

    } else { //if black
        attackBitboard |= (bitboard >> 7) & (NOT_A_FILE); //same logic
        attackBitboard |= (bitboard >> 9) & (NOT_H_FILE);
    }

    return attackBitboard; //returns only where the pawn can attack
}

U64 knight_attacks_mask(int square) {
    U64 bitboard = 0ULL;
    U64 attackBitboard = 0ULL;
    set_bit(bitboard, square);

    attackBitboard |= (bitboard << 17) & NOT_A_FILE; //same logic as pawn attack masks
    attackBitboard |= (bitboard << 15) & NOT_H_FILE;

    attackBitboard |= (bitboard << 10) & NOT_AB_FILE; 
    attackBitboard |= (bitboard << 6)  & NOT_GH_FILE;

    attackBitboard |= (bitboard >> 6)  & NOT_AB_FILE; 
    attackBitboard |= (bitboard >> 10) & NOT_GH_FILE; 

    attackBitboard |= (bitboard >> 15) & NOT_A_FILE;
    attackBitboard |= (bitboard >> 17) & NOT_H_FILE;

    return attackBitboard;
}

U64 king_attacks_mask(int square) {
    U64 bitboard = 0ULL;
    U64 attackBitboard = 0ULL;
    set_bit(bitboard, square);

    attackBitboard |= (bitboard >> 9) & NOT_H_FILE;
    attackBitboard |= (bitboard >> 8); //might have issues here
    attackBitboard |= (bitboard >> 7) & NOT_A_FILE;
    attackBitboard |= (bitboard >> 1) & NOT_H_FILE;
    attackBitboard |= (bitboard << 1) & NOT_A_FILE;
    attackBitboard |= (bitboard << 7) & NOT_H_FILE;
    attackBitboard |= (bitboard << 8);
    attackBitboard |= (bitboard << 9) & NOT_A_FILE;

    return attackBitboard;
}
U64 bishop_blockers_mask(int square) { //generates all squares a bishop sees minus the border squares
    U64 blockerMask = 0ULL;

    int rank, file;
    int targetRank = square / 8;
    int targetFile = square % 8;

    for(rank = targetRank + 1, file = targetFile + 1; rank < 7 && file < 7; rank++, file++) {blockerMask |= (1ULL << ((rank * 8) + file));}
    for(rank = targetRank + 1, file = targetFile - 1; rank < 7 && file > 0; rank++, file--) {blockerMask |= (1ULL << ((rank * 8) + file));}
    for(rank = targetRank - 1, file = targetFile + 1; rank > 0 && file < 7; rank--, file++) {blockerMask |= (1ULL << ((rank * 8) + file));}
    for(rank = targetRank - 1, file = targetFile - 1; rank > 0 && file > 0; rank--, file--) {blockerMask |= (1ULL << ((rank * 8) + file));}

    return blockerMask;
}

U64 rook_blockers_mask(int square) { //same as above but for rooks
    U64 blockerMask = 0ULL;
    int rank, file;
    int targetRank = square / 8;
    int targetFile = square % 8;

    for(file = targetFile + 1; file < 7; file++) {blockerMask |= (1ULL << ((targetRank * 8) + file));}
    for(file = targetFile - 1; file > 0; file--) {blockerMask |= (1ULL << ((targetRank * 8) + file));}
    for(rank = targetRank + 1; rank < 7; rank++) {blockerMask |= (1ULL << ((rank * 8) + targetFile));}
    for(rank = targetRank - 1; rank > 0; rank--) {blockerMask |= (1ULL << ((rank * 8) + targetFile));}

    return blockerMask;
}

U64 get_blockers_bitboard(int index, int bitsInMask, U64 blockerMask) {
    /*returns one of the possible blockerBitboard combinations given its index, number of bits, and the blockersMask
        For example, let's look at a rook on A1. It sees 12 squares for a total of 4096 possible blockerBoards. At index 0, there is no pieces blocking
        its view. At index 4095, there's a piece at every single square it looks at

        bitsInMask can be found using count bits

        blockersMask is the blockers mask for a given square and piece type
    */
    U64 blockerBitboard = 0ULL;

    for(int i = 0; i < bitsInMask; i++) {
        int square = get_lsb_index(blockerMask);
        pop_bit(blockerMask, square);
        
        if(index & (1 << i)) { //if bit i is a 1 in index, we set bit (square) as a 1 in the blockersBitboard
            blockerBitboard |= (1ULL << square);
        }
    }
    
    return blockerBitboard;
}

U64 bishop_attacks_mask(int square, U64 blockerBitboard) { //given a bitboard of blockers, generate the possible moves
    U64 attackBitboard = 0ULL;
    int rank, file;
    int targetRank = square / 8;
    int targetFile = square % 8;

    for(rank = targetRank + 1, file = targetFile + 1; rank < 8 && file < 8; rank++, file++) { //given a square, set every positive 9th bit; SE direction
        attackBitboard |= (1ULL << ((rank * 8) + file));
        if((1ULL << ((rank * 8) + file)) & blockerBitboard) break; //if that square is occupied in blockersBitboard, return non-zero and break
    }

    for(rank = targetRank + 1, file = targetFile - 1; rank < 8 && file >= 0; rank++, file--) { //SW direction
        attackBitboard |= (1ULL << ((rank * 8) + file));
        if((1ULL << ((rank * 8) + file)) & blockerBitboard) break;
    }

    for(rank = targetRank - 1, file = targetFile + 1; rank >= 0 && file < 8; rank--, file++) { //NE
        attackBitboard |= (1ULL << ((rank * 8) + file));
        if((1ULL << ((rank * 8) + file)) & blockerBitboard) break;
    }

    for(rank = targetRank - 1, file = targetFile - 1; rank >= 0 && file >= 0; rank--, file--) { //NW direction
        attackBitboard |= (1ULL << ((rank * 8) + file));
        if((1ULL << ((rank * 8) + file)) & blockerBitboard) break;
    }

    return attackBitboard;
}

U64 rook_attacks_mask(int square, U64 blockerBitboard) { //have to fix this
    U64 attackBitboard = 0ULL;

    //imagine a rook on A1
    //it targets the entire rank and file it's at; only two for loops? no because we want to check for blockers in each direction starting from the square
    int rank, file;
    int targetRank = square / 8;
    int targetFile = square % 8;

    //again from black's perspective
    for(file = targetFile + 1; file < 8; file++) { //heading east
        attackBitboard |= (1ULL << ((targetRank * 8) + file));
        if((1ULL << ((targetRank * 8) + file)) & blockerBitboard) break;
    }

    for(file = targetFile - 1; file >= 0; file--) { //west
        attackBitboard |= (1ULL << ((targetRank * 8) + file));
        if((1ULL << ((targetRank * 8) + file)) & blockerBitboard) break;
    }

    for(rank = targetRank + 1; rank < 8; rank++) { //south
        attackBitboard |= (1ULL << ((rank * 8) + targetFile));
        if((1ULL << ((rank * 8) + targetFile)) & blockerBitboard) break;
    }

    for(rank = targetRank - 1; rank >= 0; rank--) { //north
        attackBitboard |= (1ULL << ((rank * 8) + targetFile));
        if((1ULL << ((rank * 8) + targetFile)) & blockerBitboard) break;
    }

    return attackBitboard;
}

U64 get_bishop_attacks(int square, U64 blockersBitboard) {
    blockersBitboard &= bishopBlockersMask[square];
    blockersBitboard *= BishopMagicNumbers[square];
    blockersBitboard >>= (64 - bishopBitsSeen[square]);

    return bishopAttacksTable[square][blockersBitboard];
}

U64 get_rook_attacks(int square, U64 blockersBitboard) {
    blockersBitboard &= rookBlockersMask[square];
    blockersBitboard *= RookMagicNumbers[square];
    blockersBitboard >>= (64 - rookBitsSeen[square]);

    return bishopAttacksTable[square][blockersBitboard];
}

U64 get_queen_attacks(int square, U64 blockersBitboard) { //only function need for queen attacks lol
    U64 queenAttacks = 0ULL;
    U64 bishopBlockersBitboard = blockersBitboard;
    U64 rookBlockersBitboard = blockersBitboard;

    bishopBlockersBitboard &= bishopBlockersMask[square];
    bishopBlockersBitboard *= BishopMagicNumbers[square];
    bishopBlockersBitboard >>= (64 - bishopBitsSeen[square]);

    queenAttacks = bishopBlockersBitboard;

    rookBlockersBitboard &= rookBlockersMask[square];
    rookBlockersBitboard *= RookMagicNumbers[square];
    rookBlockersBitboard >>= (64 - rookBitsSeen[square]);

    queenAttacks |= rookBlockersBitboard;

    return queenAttacks;
}

void init_leapers_attacks() {
    for(int square = 0; square < 64; square++) {
        pawnAttacksTable[0][square] = pawn_attacks_mask(square, white);
        pawnAttacksTable[1][square] = pawn_attacks_mask(square, black);
        knightAttacksTable[square] = knight_attacks_mask(square);
        kingAttacksTable[square] = king_attacks_mask(square);
    }
}

void init_sliders_attacks(int bishop) { //let's say we initialize the bishop attacks
    for(int square = 0; square < 64; square++) { //for every square,
        bishopBlockersMask[square] = bishop_blockers_mask(square); //set the blocker mask of that square; the bitboard where every square the bishop sees is a 1 except
        rookBlockersMask[square] = rook_blockers_mask(square); //if it's an edge square
        
        U64 blockersMask = bishop ? bishopBlockersMask[square] : rookBlockersMask[square]; //get that blocker mask
        int bitsSeen = count_bits(blockersMask); //get how many 1s are in that mask
        int blockersIndices = 1 << bitsSeen; //gets you 2^bitsSeen, aka the total possible permutation of blockers bitboards

        for(int i = 0; i < blockersIndices; i++) { //for every possible permutation of blockers bitboards
            if(bishop) {
                U64 blockersBitboard = get_blockers_bitboard(i, bitsSeen, blockersMask); //get one specific blockers bitboard
                int magicIndex = (blockersIndices * BishopMagicNumbers[square]) >> (64 - bishopBitsSeen[square]); //get the magicIndex using magic numbers we found
                bishopAttacksTable[square][magicIndex] = bishop_attacks_mask(square, blockersBitboard); //store the corresponding attack bitboard in the array
            } else {
                U64 blockersBitboard = get_blockers_bitboard(i, bitsSeen, blockersMask);
                int magicIndex = (blockersIndices * RookMagicNumbers[square]) >> (64 - rookBitsSeen[square]);
                rookAttacksTable[square][magicIndex] = rook_attacks_mask(square, blockersBitboard);
            }
        } //repeat this for all permutations and all squares
    } //the magic numbers is essentially a hashing function to make sure no collision occurs in each square's array
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

int bishopBitsSeen[64] = { //given a bishop on a square, this tells you how many squares it looks at
    6, 5, 5, 5, 5, 5, 5, 6, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    5, 5, 7, 7, 7, 7, 5, 5, 
    5, 5, 7, 9, 9, 7, 5, 5, 
    5, 5, 7, 9, 9, 7, 5, 5, 
    5, 5, 7, 7, 7, 7, 5, 5, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    6, 5, 5, 5, 5, 5, 5, 6
};

int rookBitsSeen[64] = { //for rooks
    12, 11, 11, 11, 11, 11, 11, 12, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    12, 11, 11, 11, 11, 11, 11, 12
};

U64 BishopMagicNumbers[64] = {
    0x20204004822000bULL, //square 1 (a1)
    0x40b00002009038ULL, //square 2 (a2)
    0x105840004100200ULL, //and so on
    0x8204260401200024ULL,
    0x2202001500200100ULL,
    0x1000081218200010ULL,
    0x8082240200a0000ULL,
    0x808004013100ULL,
    0x4300a1030048210ULL,
    0x11001800201bULL,
    0x6020816400820000ULL,
    0x8504100400970040ULL,
    0x8040811040402001ULL,
    0x9080430011000a0ULL,
    0xc1200120810ULL,
    0x20900224044409ULL,
    0x242000828850012ULL,
    0x6105000019240310ULL,
    0x400020300110040cULL,
    0x8082104000400ULL,
    0x480282002044ULL,
    0x40840100108ULL,
    0x40480024a0104ULL,
    0x9401040020080280ULL,
    0x11080208824a00ULL,
    0x403044010100020ULL,
    0x4002402002024410ULL,
    0x4040700102a00ULL,
    0x2000830301040ULL,
    0x921420060401028ULL,
    0x402484022a00ULL,
    0x581a90020c054ULL,
    0x2104110080100ULL,
    0x202106000a18882ULL,
    0x8040100100030ULL,
    0xa040400480020ULL,
    0x504040040010ULL,
    0x2060090102202ULL,
    0x2088211141210ULL,
    0x1a0110d0010a1ULL,
    0x2808101b1800200cULL,
    0x440a40d04800ULL,
    0x2002040024080800ULL,
    0x4014890210200ULL,
    0x8c02501000010ULL,
    0x80840c0152804ULL,
    0x4a100091000061ULL,
    0x588281000c480020ULL,
    0x841a09008100008ULL,
    0x422800264023ULL,
    0x4018100202064080ULL,
    0x4050200010440000ULL,
    0x200848849003824ULL,
    0x1000004610e20050ULL,
    0x1084038428104ULL,
    0xa04201006cc08ULL,
    0x100000c004104020ULL,
    0x161040084800ULL,
    0x104404002898226ULL,
    0x5002001000030408ULL,
    0x4411003008080440ULL,
    0x84010240d048030ULL,
    0x1000084500142280ULL,
    0x840440020824ULL,
};

U64 RookMagicNumbers[64] = {
    0xa820000100120040ULL,
    0x1602088020004068ULL,
    0xe00000000400400ULL,
    0xca00000820100040ULL,
    0x1200030100020c01ULL,
    0x8828004240c08000ULL,
    0x40260a0000c0000ULL,
    0xa00000018040000ULL,
    0x84010000000018ULL,
    0x206006002200000ULL,
    0x2041000808400108ULL,
    0x4020881000040402ULL,
    0x8612002001010000ULL,
    0x608200004100200ULL,
    0x1424000003020800ULL,
    0x104000002000000ULL,
    0xa28280400000011ULL,
    0x4004040000030002ULL,
    0x8082120080004410ULL,
    0x1120200464001c0ULL,
    0xb2020014500dULL,
    0x2100920207800000ULL,
    0xc2800005c0830ULL,
    0x3c3040008010000ULL,
    0x408802808a80002ULL,
    0x2040200200800240ULL,
    0x1200501240000ULL,
    0x20204200120800ULL,
    0x108010100081200ULL,
    0xc864010300100004ULL,
    0x4481205000000902ULL,
    0x250621004400100ULL,
    0x21108801000c480ULL,
    0x110402082080001ULL,
    0x41204a0082000002ULL,
    0x2001001005002200ULL,
    0xc000201202000b80ULL,
    0x82001040a8000000ULL,
    0x2100250218008000ULL,
    0x84001a4020010101ULL,
    0xc29a0c080080060ULL,
    0x54890c0100032ULL,
    0x110202200820020ULL,
    0x2042005008020000ULL,
    0x100804021100000ULL,
    0x4008102010180ULL,
    0x1048850022084000ULL,
    0x2461802892400000ULL,
    0x10c10920805000ULL,
    0x204011040a0212ULL,
    0xa0103884022000ULL,
    0x3040806100100ULL,
    0x4004010820822006ULL,
    0x5202044084190401ULL,
    0x1002215804001ULL,
    0x204210040861000ULL,
    0x688801041042001ULL,
    0x448401061060000ULL,
    0x20911208041242ULL,
    0x8011040810821020ULL,
    0x1322049814010c40ULL,
    0x2400a10202440400ULL,
    0x2001002a00448000ULL,
    0x2811421004008ULL,
};