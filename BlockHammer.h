#ifndef __ROWBLOCKER_H
#define __ROWBLOCKER_H

#include <stdlib.h>
#include<iostream>
#include <limits.h>
#include <time.h>
#include<vector>
#include<iterator>

#define  arsize 1000

using namespace std;

namespace ramulator
{

    class BlockHammer{
        private:

        long cntr;
        long CurrTime, epc, epoch;          // TIME
        bool c;                             // EPOCH
        int Nrh, Nbl, Tcbf,Trw, Tdelay;

        long recent[arsize], timeStamp[arsize];       // HISTORY BUFFER

        long hashKey[2][4];                 // ROWBLOCKER
        int bloomFilter0[arsize];           // RB
        int bloomFilter1[arsize];

        float RHLI[8][16];                  // AT
        int ATC0[8][16];                    // ATTACK THROTTLER
        int ATC1[8][16];
        bool ATR0[8][16];                    // ATTACK THROTTLER RECENT
        bool ATR1[8][16];

        /*      ROW BLOCKER     */

        void hashseed();                // BLACK LIST
        long hashfunc(int,int,long);
        void bloomClear();
        void bloomAdd(long);
        bool RowBlockerBL(long);

        void historyAdd(long, long);    // HISTORY BUFFER
        void historyClear(int);
        bool RowBlockerHB(long, long);

        /*      ATTACK THROTTLER     */
        void atcAdd(int, int);
        void atcClear();
        void RHLIcalc(int, int);
        bool atcLim(int, int);
        bool AttackThrottler(int, int);

        public:

        void test(int, long);
        bool rowBlocker(long, long, int, int);

        void eUpdate(long);
        bool BHcheck(long, long, int, int);

        BlockHammer()
        {
            cntr = 0;
            Nrh = 3200; Nbl = 400;
            Tdelay = 8; epoch = 32000;
            Tcbf = 64000; Trw = 64000;   // in ms, need to sync

            srand(time(NULL));
            epc = 0;
            
            c = false; 
                hashseed();
                bloomClear(); 
                atcClear();
            c = true; 
                hashseed();
                bloomClear(); 
                atcClear();

            c = false;
        }        
        
    };
}

#endif /* __ROWBLOCKER_H */
