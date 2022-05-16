#include"BlockHammer.h"
#include<iostream>

using namespace std;
using namespace ramulator;

namespace ramulator{

  //------------------------------ ROW BLOCKER --------------------------------
    //------------------- PRIVATE ELEMENTS --------------------------------


    //  ------    ROWBLOCKER HISTORY BUFFER ELEMENTS    -----

    void BlockHammer::historyAdd(long add, long time){      // adds access count to history buffer
        int j;
        for (j = 0; j < arsize; j++){
            if(timeStamp[j]== -1){
                break;}
        }
        
        for(int i = j; i>0 ; --i){
            recent[i]=recent[i-1];
            timeStamp[i]=timeStamp[i-1];
        }
        recent[0]=add;
        timeStamp[0]=time;
        return;
    }

    void BlockHammer::historyClear(int index){              // clears recent access at every new epoch 
        for (int i = index; i < arsize; i++)
        {
            if(timeStamp[i] == -1){
                break;
            }

            recent[i] = -1;
            timeStamp[i] = -1;
        }
        
        return;
    }

    bool BlockHammer::RowBlockerHB(long add, long time){    // checks history buffer
        //return true;
        for(int i=0 ; i<arsize ; ++i){
            if(recent[i]==add){
                if( time-timeStamp[i]<Tdelay){
                    return true;
                }
            }
        }
        return false;
    }

    //  X X X X    HB END    X X X X
    //  ------    ROWBLOCKER BLACK LIST ELEMENTS    -----

    void BlockHammer::hashseed(){                   // genrates hashkeys for each epoch

        long temp;
        long t1, t2;

        if (c){                                     // checks active epoch
            for (int i = 0; i < 4; i++){
                t1 = rand();
                t2 = rand();
                temp = (t1<<31) + t2;
                hashKey[1][i] = temp;
            }
        }

        else {
            for (int i = 0; i < 4; i++){
                t1 = rand();
                t2 = rand();
                temp = (t1<<31) + t2;
                hashKey[0][i] = temp;
            }
        }

        return;
    }

    long BlockHammer::hashfunc(int m, int n, long add){     // XOR add with key to genrate hashed values

        long x = (add^hashKey[m][n])%arsize;
        return x;
    }

    void BlockHammer::bloomClear(){                         // Clears a Bloom filter at every epoch

        if(c){
            for (int i = 0; i < arsize; i++){
                bloomFilter1[i]=0;
            }
        }
            
        else{
            for (int i = 0; i < arsize; i++){
                bloomFilter0[i]=0;
            }
        }
    }

    void BlockHammer::bloomAdd(long add){               // Adds access count to bloom filters 

        long t;
    
        for (int i = 0; i < 4; i++){
            t = hashfunc(1,i,add);
            bloomFilter1[t]++;
        }
                
        for (int i = 0; i < 4; i++){
            t = hashfunc(0,i,add);
            bloomFilter0[t]++;
        }

        return;
    }

    //------------------- PUBLIC ELEMENTS -------------------------

    bool BlockHammer::RowBlockerBL(long add){       // Checks weather an address is blacklisted or not
        int nk = Nbl;
        if (c){   
            int t;
            for (int i = 0; i < 4; i++)
            {
                t = hashfunc(1,i,add);
                if(bloomFilter1[t]<nk){
                    nk = bloomFilter1[t];    
                }
            }
        }
        else{
            long t;
            for (int i = 0; i < 4; i++)
            {
                t = hashfunc(0,i,add);
                if(bloomFilter0[t]<nk){
                    nk = bloomFilter1[t];
                }
            }
        }
        return (nk >= Nbl);
    }

    //  X X X X    BL END    X X X X

    bool BlockHammer::rowBlocker(long add, long time, int cid, int bnk)     // called at row ACT and blocks blacklisted accesses
    {
        if(RowBlockerBL(add)){
            atcAdd(cid,bnk);
            if(RowBlockerHB(add, time)){
                return true;
            }
            else{    
                historyAdd(add,time);
                bloomAdd(add);
                return false;
            }

        }
        else {
            bloomAdd(add);
            return false; 
        }
    }

  //---------------------------- ATTACK THROTTLER ------------------------------
    //------------------- PRIVATE ELEMENTS ------------------------

    void BlockHammer::atcClear()                    // Clears a counter set
    {
        if(c)
        {
            for (int i = 0; i < 8; i++){
                for (int j = 0; j < 16; j++){
                    ATC1[i][j]=0;
                    ATR1[i][j]=1;
                }
            }
        }
        else
        {
            for (int i = 0; i < 8; i++){
                for (int j = 0; j < 16; j++){
                    ATC0[i][j]=0;
                    ATR0[i][j]=1;
            	}
            }
        }

        return;
    }

    void BlockHammer::atcAdd(int m, int n)      // Adds access for thread bank pair
    {   
        ATC0[m][n]++;
        ATC1[m][n]++;
        return;   
    }

    void BlockHammer::RHLIcalc(int m, int n)    // Claculate RHLI values
    {
        if (c){
            RHLI[m][n] = ATC1[m][n];
        }
        else{
            RHLI[m][n] = ATC0[m][n];
        }
        RHLI[m][n] /= (Nrh-Nbl);

        return;
    }
    
    //------------ ATTACK THROTTLER LIMITER ----------

	bool BlockHammer::atcLim(int m, int n)
	{
	
		// return true;					// uncomment for passive
		
		if(c){
			ATR1[m][n] = !ATR1[m][n];
			return ATR1[m][n];
		}
		else{
			ATR0[m][n] = !ATR1[m][n];
			return ATR0[m][n];
		}
	}
	
	

	// X X X X X X X ATTACK THROTTLER LIMITER X X X X X X X
    
    //------------------- PUBLIC ELEMENTS -------------------------

    bool BlockHammer::AttackThrottler(int coreID, int bank)     // ristricts bank access for certain threads based on RHLI
    {
        // return false;                 // uncomment for passive

        if (bank<0)
        {
            return false;
        }
        
        RHLIcalc(coreID, bank);
        //cout<<RHLI[coreID][bank]<<endl;
        if(RHLI[coreID][bank] > 0.9){
            //cout<<"high"<<endl;
            return true;
        }
        else if(RHLI[coreID][bank] > 0.7){
            //cout<<"mid"<<endl;
        	return atcLim(coreID, bank);
        }

        //cout<<"Low"<<endl;
        return false;
    }

  /*------------------------------ BLOCK HAMMER --------------------------------
    common, Attack Throttler & Row Blocker
        public
  */

    void BlockHammer::eUpdate(long clk)            // Sycronises and updates values from the RAM clock
    {
        CurrTime = clk;
        if((clk - epc) >= epoch)
        {
            epc = clk;
            bloomClear();
            hashseed();
            atcClear();
            c = (c)?false:true;
            
        }
        return;
    }
    bool BlockHammer::BHcheck(long add, long time, int cid, int bnk) {  // The function the does checks in controller for Row Hammer
        // return false;   // Deactivate
        
        if (AttackThrottler(cid, bnk))
        {
            cntr++;
            cout<<cntr<<endl;
            
            return true;
        }
        
        if (rowBlocker(add, time, cid, bnk))
        {
            return true;
        }
        return false;
        
    }   

}