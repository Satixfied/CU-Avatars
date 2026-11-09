#include "ManualSwitchStamp.h"
#include <cassert>
#include <cstdio>
int main(){
 ManualSwitchStamp signal;
 assert(signal.query(10,20,0)==0); // No intent: arbitrary replacement must clean up.
 signal.publish(10,20,1000);auto first=signal.query(10,20,1000);assert(first!=0);
 assert(signal.query(10,20,4000)==first);assert(signal.query(10,20,4001)==0);
 assert(signal.query(20,30,1001)==0); // Hospital replacement cannot reuse old intent.
 assert(signal.query(10,30,1001)==0);assert(signal.query(20,10,1001)==0);
 signal.publish(20,30,1100);auto second=signal.query(20,30,1100);assert(second&&second!=first);
 assert(signal.query(10,20,1100)==0); // Superseded switch no longer authorizes a handoff.
 signal.publish(30,30,1200);assert(signal.query(30,30,1200)!=0); // Engine may retain handle.
 signal.publish(30,40,0xfffffff0u);assert(signal.query(30,40,0x10u)!=0);
 assert(signal.query(30,40,0x1000u)==0); // GetTickCount wrap still expires normally.
 puts("PASS: only exact recent manual switches authorize preset handoff; expiry, supersession and tick wrap.");
}
