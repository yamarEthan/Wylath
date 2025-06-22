#include <stdio.h>
#include "definitions.h"
#include "attacks.h"
#include "display.h"

U64 pawnAttackTable[2][64]; //contains the attack table for pawns in both sides and for each square [side][square]
U64 knightAttackTable[64];
U64 kingAttackTable[64];

U64 bishopBlockerMask[64];
U64 rookBlockerMask[64];

U64 bishopAttackTable[64][512];
U64 rookAttackTable[64][4096];

U64 pawn_attack_mask(int square, int side) { //have yet to deal with initial two square move of pawns and en passant
    U64 bitboard = 0ULL; //empty bitboard
    U64 attackBitboard = 0ULL;
    set_bit(bitboard, square);

    if(!side) { //if white (because white is 0)
        attackBitboard |= (bitboard << 7) & (NOT_H_FILE); //if the shifted bit doesn't end on the H file, accept it; if it is on H file, that means the attack square
        attackBitboard |= (bitboard << 9) & (NOT_A_FILE); //moved up two ranks, which is wrong

    } else { //if black
        attackBitboard |= (bitboard >> 7) & (NOT_A_FILE); //same logic
        attackBitboard |= (bitboard >> 9) & (NOT_H_FILE);
    }

    return attackBitboard; //returns only where the pawn can attack
}

U64 knight_attack_mask(int square) {
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

U64 king_attack_mask(int square) {
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
U64 bishop_blocker_mask(int square) { //generates all squares a bishop sees minus the border squares
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

U64 rook_blocker_mask(int square) { //same as above but for rooks
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

U64 get_blocker_bitboard(int index, int bitsInMask, U64 blockerMask) {
    /*returns one of the possible blockerBitboard combinations given its index, number of bits, and the blockerMask
        For example, let's look at a rook on A1. It sees 12 squares for a total of 4096 possible blockerBoards. At index 0, there is no pieces blocking
        its view. At index 4095, there's a piece at every single square it looks at

        bitsInMask can be found using count bits

        blockerMask is the blockers mask for a given square and piece type
    */
    U64 blockerBitboard = 0ULL;

    for(int i = 0; i < bitsInMask; i++) {
        int square = get_lsb_index(blockerMask);
        pop_bit(blockerMask, square);
        
        if(index & (1 << i)) { //if bit i is a 1 in index, we set bit (square) as a 1 in the blockerBitboard
            blockerBitboard |= (1ULL << square);
        }
    }
    
    return blockerBitboard;
}

U64 bishop_attack_mask(int square, U64 blockerBitboard) { //given a bitboard of blockers, generate the possible moves
    U64 attackBitboard = 0ULL;
    int rank, file;
    int targetRank = square / 8;
    int targetFile = square % 8;

    for(rank = targetRank + 1, file = targetFile + 1; rank < 8 && file < 8; rank++, file++) { //given a square, set every positive 9th bit; SE direction
        attackBitboard |= (1ULL << ((rank * 8) + file));
        if((1ULL << ((rank * 8) + file)) & blockerBitboard) break; //if that square is occupied in blockerBitboard, return non-zero and break
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

U64 rook_attack_mask(int square, U64 blockerBitboard) {
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

U64 get_bishop_attacks(int square, U64 blockerBitboard) {
    blockerBitboard &= bishopBlockerMask[square];
    blockerBitboard *= BishopMagicNumbers[square];
    blockerBitboard >>= (64 - bishopBitsSeen[square]);

    return bishopAttackTable[square][blockerBitboard];
}

U64 get_rook_attacks(int square, U64 blockerBitboard) {
    blockerBitboard &= rookBlockerMask[square];
    blockerBitboard *= RookMagicNumbers[square];
    blockerBitboard >>= (64 - rookBitsSeen[square]);

    return rookAttackTable[square][blockerBitboard];
}

U64 get_queen_attacks(int square, U64 blockerBitboard) { //only function need for queen attacks lol
    U64 queenAttacks = 0ULL;
    U64 bishopBlockerBitboard = blockerBitboard;
    U64 rookBlockerBitboard = blockerBitboard;

    bishopBlockerBitboard &= bishopBlockerMask[square];
    bishopBlockerBitboard *= BishopMagicNumbers[square];
    bishopBlockerBitboard >>= (64 - bishopBitsSeen[square]);

    queenAttacks = bishopBlockerBitboard;

    rookBlockerBitboard &= rookBlockerMask[square];
    rookBlockerBitboard *= RookMagicNumbers[square];
    rookBlockerBitboard >>= (64 - rookBitsSeen[square]);

    queenAttacks |= rookBlockerBitboard;

    return queenAttacks;
}

void init_leapers_attacks() {
    for(int square = 0; square < 64; square++) {
        pawnAttackTable[0][square] = pawn_attack_mask(square, white);
        pawnAttackTable[1][square] = pawn_attack_mask(square, black);
        knightAttackTable[square] = knight_attack_mask(square);
        kingAttackTable[square] = king_attack_mask(square);
    }
}

void init_sliders_attacks(int bishop) { //let's say we initialize the bishop attacks
    for(int square = 0; square < 64; square++) { //for every square,

        if(bishop) {bishopBlockerMask[square] = bishop_blocker_mask(square);
        } else {rookBlockerMask[square] = rook_blocker_mask(square);}

        U64 blockerMask = bishop ? bishopBlockerMask[square] : rookBlockerMask[square]; //get that blocker mask
        int bitsInMask = count_bits(blockerMask); //get how many 1s are in that mask
        int blockerIndices = 1 << bitsInMask; //gets you 2^bitsSeen, aka the total possible permutation of blockers bitboards

        for(int i = 0; i < blockerIndices; i++) { //for every possible permutation of blockers bitboards
            U64 blockerBitboard = get_blocker_bitboard(i, bitsInMask, blockerMask); //get one specific blockers bitboard
            if(bishop) {
                int magicIndex = (blockerBitboard * BishopMagicNumbers[square]) >> (64 - bishopBitsSeen[square]); //get the magicIndex using magic numbers we found
                bishopAttackTable[square][magicIndex] = bishop_attack_mask(square, blockerBitboard); //store the corresponding attack bitboard in the array
            } else {
                int magicIndex = (blockerBitboard * RookMagicNumbers[square]) >> (64 - rookBitsSeen[square]);
                rookAttackTable[square][magicIndex] = rook_attack_mask(square, blockerBitboard);
            }
        } //repeat this for all permutations and all squares
    } //the magic numbers is essentially a hashing function to make sure no collision occurs in each square's array
}

int is_square_attacked(int square, int side) { //checks if the square is attacked by the given side
    //we start with pawns
    //say we're checking if the square is attacked by white pawns. how do we do that?
    //so if side attacking is white, we check if the pawnAttackTable of a black pawn in that square is matched with a white pawn there
    //for example, a black pawn on e5 is attacking d4 and f4, SO if a white pawn is on either of those squares, the e5 square is being attacked by white

    if((side == white) && (pawnAttackTable[black][square] & pieceBitboards[P])) {return 1;}
    if((side == black) && (pawnAttackTable[white][square] & pieceBitboards[p])) {return 1;}

    //now we do knights; now we only need the square for the attack 
    if(knightAttackTable[square] & ((side == white) ? pieceBitboards[N] : pieceBitboards[n])) {return 1;}
    if(kingAttackTable[square] & ((side == white) ? pieceBitboards[K] : pieceBitboards[k])) {return 1;}

    //now we do sliding pieces; to get the bishop/rook attacks, we need a blocker bitboard, but we didn't ask for it above.
    //so instead, we feed it the occupancies bitboard of both black and white just so that we account for every piece that may be in the way
    if(get_bishop_attacks(square, occupancyBitboards[both]) & ((side == white) ? pieceBitboards[B] : pieceBitboards[b])) {return 1;}
    if(get_rook_attacks(square, occupancyBitboards[both]) & ((side == white) ? pieceBitboards[R] : pieceBitboards[r])) {return 1;}
    //we don't need to get queen attacks as checking both bishop and rooks does the same thing

    return 0; //return 0 if the square is not attacked by the given side
}

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
    0x420c80100408202ULL,
    0x1204311202260108ULL,
    0x2008208102030000ULL,
    0x24081001000caULL,
    0x488484041002110ULL,
    0x1a080c2c010018ULL,
    0x20a02a2400084ULL,
    0x440404400a01000ULL,
    0x8931041080080ULL,
    0x200484108221ULL,
    0x80460802188000ULL,
    0x4000090401080092ULL,
    0x4000011040a00004ULL,
    0x20011048040504ULL,
    0x2008008401084000ULL,
    0x102422a101a02ULL,
    0x2040801082420404ULL,
    0x8104900210440100ULL,
    0x202101012820109ULL,
    0x248090401409004ULL,
    0x44820404a00020ULL,
    0x40808110100100ULL,
    0x480a80100882000ULL,
    0x184820208a011010ULL,
    0x110400206085200ULL,
    0x1050010104201ULL,
    0x4008480070008010ULL,
    0x8440040018410120ULL,
    0x41010000104000ULL,
    0x4010004080241000ULL,
    0x1244082061040ULL,
    0x51060000288441ULL,
    0x2215410a05820ULL,
    0x6000941020a0c220ULL,
    0xf2080100020201ULL,
    0x8010020081180080ULL,
    0x940012060060080ULL,
    0x620008284290800ULL,
    0x8468100140900ULL,
    0x418400aa01802100ULL,
    0x4000882440015002ULL,
    0x420220a11081ULL,
    0x401a26030000804ULL,
    0x2184208000084ULL,
    0xa430820a0410c201ULL,
    0x640053805080180ULL,
    0x4a04010a44100601ULL,
    0x10014901001021ULL,
    0x422411031300100ULL,
    0x824222110280000ULL,
    0x8800020a0b340300ULL,
    0xa8000441109088ULL,
    0x404000861010208ULL,
    0x40112002042200ULL,
    0x2141006480b00a0ULL,
    0x2210108081004411ULL,
    0x2010804070100803ULL,
    0x7a0011010090ac31ULL,
    0x18005100880400ULL,
    0x8010001081084805ULL,
    0x400200021202020aULL,
    0x4100342100a0221ULL,
    0x404408801010204ULL,
    0x6360041408104012ULL,
};

U64 RookMagicNumbers[64] = {
    0x8080008118604002ULL,
    0x4040100040002002ULL,
    0x80100018e00380ULL,
    0x100041002200900ULL,
    0x200020008100420ULL,
    0x4100040002880100ULL,
    0x80008002000100ULL,
    0x8100014028820300ULL,
    0x860802080004008ULL,
    0x112004081020024ULL,
    0x1042002010408200ULL,
    0x410010000b0020ULL,
    0x20800800800400ULL,
    0x4808026000400ULL,
    0x820800100800200ULL,
    0xd43000a00a04900ULL,
    0x4080818000400068ULL,
    0x20818040002005ULL,
    0xa0010018410020ULL,
    0x8010004008040041ULL,
    0x28008008800400ULL,
    0x809010002080400ULL,
    0x1040240048311230ULL,
    0x88020000d28425ULL,
    0x1480004440002010ULL,
    0x2020400440201000ULL,
    0x2000200080100080ULL,
    0x1400280280300080ULL,
    0x4028002500181100ULL,
    0x8040040080800200ULL,
    0x800020400108108ULL,
    0x3041120004408cULL,
    0x80804008800020ULL,
    0x4010002000400040ULL,
    0x2000100080802000ULL,
    0x8300810804801000ULL,
    0x8011001205000800ULL,
    0x810800601800400ULL,
    0x4301083214000150ULL,
    0x204026458e001401ULL,
    0x40204000808000ULL,
    0x8001008040010020ULL,
    0x8410820820420010ULL,
    0x1003001000090020ULL,
    0x804040008008080ULL,
    0x12000810020004ULL,
    0x1000100200040208ULL,
    0x430000a044020001ULL,
    0x280009023410300ULL,
    0xe0100040002240ULL,
    0x200100401700ULL,
    0x2244100408008080ULL,
    0x8000400801980ULL,
    0x2000810040200ULL,
    0x8010100228810400ULL,
    0x2000009044210200ULL,
    0x4080008040102101ULL,
    0x40002080411d01ULL,
    0x2005524060000901ULL,
    0x502001008400422ULL,
    0x489a000810200402ULL,
    0x1004400080a13ULL,
    0x4000011008020084ULL,
    0x26002114058042ULL,
};