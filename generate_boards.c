// adapted from tpt hand evaluator

#include "poker.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern const int primes[];

long IDs[294204];
int BS[15592864];

int numIDs = 1;
int numcards = 0;
int maxBS = 0;
long maxID = 0;

int min(int a, int b) {
    return (a < b ? a : b);
}

long make_id(long IDin, int newcard)
{
    long ID = 0;
    int rankcount[13 + 1];
    int workcards[6];
    int cardnum;
    int getout = 0;

    memset(workcards, 0, sizeof(workcards));
    memset(rankcount, 0, sizeof(rankcount));

    for (cardnum = 0; cardnum < 4; cardnum++)
    {
        workcards[cardnum + 1] = (int) ((IDin >> (8 * cardnum)) & 0xff);
    }

    newcard--;

    // add next card formats card to rrrr00ss
    workcards[0] = (((newcard >> 2) + 1) << 4) + (newcard & 3) + 1;

    for (numcards = 0; workcards[numcards]; numcards++)
    {
        rankcount[(workcards[numcards] >> 4) & 0xf]++;
        if (numcards
            && workcards[0] == workcards[numcards])
        {
            return 0;
        }
    }

    if (numcards > 4) {
        for (int rank = 1; rank < 14; rank++) {
            if (rankcount[rank] > 4) {
                return 0;
            }
        }
    }

#define SWAP(I,J) {if (workcards[I] < workcards[J]) {workcards[I]^=workcards[J]; workcards[J]^=workcards[I]; workcards[I]^=workcards[J];}}

    SWAP(0, 1);
    SWAP(3, 4);
    SWAP(2, 4);
    SWAP(2, 3);
    SWAP(0, 3);
    SWAP(0, 2);
    SWAP(1, 4);
    SWAP(1, 3);
    SWAP(1, 2);

    ID = (long) workcards[0] +
        ((long) workcards[1] << 8) +
        ((long) workcards[2] << 16) +
        ((long) workcards[3] << 24) +
        ((long) workcards[4] << 32);

    return ID;
}

int save_id(long ID)
{
    if (ID == 0) return 0;

    if (ID >= maxID) {
        if (ID > maxID) {
            IDs[numIDs++] = ID;
            maxID = ID;
        }
        return numIDs - 1;
    }

    int low = 0;
    int high = numIDs - 1;
    long testval;
    int holdtest;

    while (high - low > 1) {
        holdtest = (high + low + 1) / 2;
        testval = IDs[holdtest] - ID;
        if (testval > 0) high = holdtest;
        else if (testval < 0) low = holdtest;
        else return holdtest;
    }

    memmove(&IDs[high + 1], &IDs[high], (numIDs - high) * sizeof(IDs[0]));

    IDs[high] = ID;
    numIDs++;
    return high;
}

int do_eval(long IDin)
{
    int cardnum;
    int workcard;
    int rank;
    int suit;

    int workcards[6];
    int holdcards[6];
    int numevalcards = 0;

    memset(workcards, 0, sizeof(workcards));
    memset(holdcards, 0, sizeof(holdcards));

    if (IDin)
    {
        for (cardnum = 0; cardnum < 5; cardnum++)
        {
            holdcards[cardnum] = (int) ((IDin >> (8 * cardnum)) & 0xff);
            if (holdcards[cardnum] == 0) break;
            numevalcards++;
        }

        for (cardnum = 0; cardnum < numevalcards; cardnum++)
        {
            workcard = holdcards[cardnum];

            rank = (workcard >> 4) - 1;
            suit = workcard & 0xf;

            //   +--------+--------+--------+--------+
            //   |xxxbbbbb|bbbbbbbb|cdhsrrrr|xxpppppp|
            //   +--------+--------+--------+--------+
            //   p = prime number of rank (deuce=2,trey=3,four=5,five=7,...,ace=41)
            //   r = rank of card (deuce=0,trey=1,four=2,five=3,...,ace=12)
            //   cdhs = suit of card
            //   b = bit turned on depending on rank of card
            workcards[cardnum] = primes[rank] | (rank << 8) | (1 << (suit + 11)) | (1 << (16 + rank));
        }

        switch (numevalcards) {
            case 3: return eval_board(workcards[0],workcards[1],workcards[2],0,0);
                break;
            case 4: return eval_board(workcards[0],workcards[1],workcards[2],workcards[3],0);
                break;
            case 5: return eval_board(workcards[0],workcards[1],workcards[2],workcards[3],workcards[4]);
                break;
            default:
                printf("    Problem with numcards = %d!!\n", numcards);
                break;
        }
    }

    return 0;
}

int main(int argc, char* argv[])
{
    int IDslot, card = 0, count = 0;
    long ID;

    memset(IDs, 0, sizeof(IDs));
    memset(BS, 0, sizeof(BS));

    int IDnum;
    int holdid;

    printf("\nGetting Card IDs!\n");

    for (IDnum = 0; IDs[IDnum] || IDnum == 0; IDnum++)
    {
        for (card = 1; card < 53; card++)
        {
            ID = make_id(IDs[IDnum], card);
            if (numcards < 5) holdid = save_id(ID);
        }
        printf("\rID - %d", IDnum);
    }

    printf("\nSetting board strengts!\n");

    for (IDnum = 0; IDs[IDnum] || IDnum == 0; IDnum++)
    {
        for (card = 1; card < 53; card++)
        {
            ID = make_id(IDs[IDnum], card);

            if (numcards < 5)
            {
                IDslot = save_id(ID) * 53 + 53;
            }
            else
            {
                IDslot = do_eval(ID);
            }

            maxBS = IDnum * 53 + card + 53;
            BS[maxBS] = IDslot;
        }

        if (numcards == 4 || numcards == 5)
        {
            BS[IDnum * 53 + 53] = do_eval(IDs[IDnum]);
        }

        printf("\rID - %d", IDnum);
    }

    printf("\nNumber IDs = %d\nmaxBS = %d\n", numIDs, maxBS);

    int c0, c1, c2, c3, c4;
    int u0, u1, u2, u3, u4;
    int pairness[6];
    int flushness[6];
    int straightness[6];
    memset(pairness, 0, sizeof(pairness));
    memset(flushness, 0, sizeof(flushness));
    memset(straightness, 0, sizeof(straightness));

    for (c0 = 1; c0 < 49; c0++) {
        u0 = BS[53+c0];
        for (c1 = c0+1; c1 < 50; c1++) {
            u1 = BS[u0+c1];
            for (c2 = c1+1; c2 < 51; c2++) {
                u2 = BS[u1+c2];
                for (c3 = c2+1; c3 < 52; c3++) {
                    u3 = BS[u2+c3];
                    for (c4 = c3+1; c4 < 53; c4++) {
                        u4 = BS[u3+c4];
                        pairness[u4 & 0x7]++;
                        flushness[(u4 >> 3) & 0x7]++;
                        straightness[(u4 >> 6) & 0x7]++;
                        count++;
                    }
                }
            }
        }
    }

    printf("\nTotal boards = %d\n", count);
    for (int i = 0; i < 6; i++) {
        printf("pairness %d: %d\n", i, pairness[i]);
        printf("flushness %d: %d\n", i, flushness[i]);
        printf("straightness %d: %d\n", i, straightness[i]);
    }

    FILE * fout = fopen("Boards.dat", "wb");
    if (!fout) {
        printf("Problem creating the Output File!\n");
        return 1;
    }
    fwrite(BS, sizeof(BS), 1, fout);
    fclose(fout);

    return 0;
}
