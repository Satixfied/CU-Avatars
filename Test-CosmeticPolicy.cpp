#include "CosmeticPolicy.h"
#include <cstdio>
#include <cstdlib>
using M=CosmeticPolicy::Mode;using A=CosmeticPolicy::Action;
static void expect(bool ok,const char* message){if(!ok){std::fprintf(stderr,"FAIL: %s\n",message);std::exit(1);}}
int main(){
 CosmeticPolicy p;
 expect(p.step(0,false,false,M::Idle,true)==A::Play,"initial idle starts");
 expect(p.step(16,false,false,M::Idle,true)==A::Wait,"asynchronous start is allowed");
 expect(p.step(5000,false,true,M::Idle,true)==A::Wait,"loop is not periodically restarted");
 expect(p.step(5100,true,true,M::Idle,true)==A::Stop,"gameplay yields the cosmetic slot once");
 for(unsigned t=5116;t<=30000;t+=16){
  expect(p.step(t,true,false,(t%3)?M::Closed:M::Talk,true)==A::Wait,"phone/action remains unmodified despite blink/speech requests");
 }
 expect(p.step(30100,false,false,M::Idle,true)==A::Wait,"put-away transition gets time to finish");
 expect(p.step(31200,false,false,M::Idle,true)==A::Play,"cosmetics resume after a quiet interval");
 expect(p.step(32000,false,false,M::Idle,true)==A::Wait,"unknown interruption does not cause immediate restart");
 expect(p.step(32016,true,false,M::Closed,true)==A::Wait,"another scripted task is not stopped");
 expect(p.step(33200,false,false,M::Idle,true)==A::Wait,"scripted action extends quiet interval");
 expect(p.step(33216,false,false,M::Idle,true)==A::Play,"cosmetics recover after scripted action");
 p.reset();
 expect(p.step(0,false,false,M::Idle,false)==A::Wait,"unloaded dictionary cannot launch");
 expect(p.step(10,false,false,M::Idle,true)==A::Play,"loaded dictionary starts normally");
 expect(p.step(400,false,false,M::Idle,true)==A::Wait,"failed asynchronous start backs off");
 expect(p.step(1599,false,false,M::Idle,true)==A::Wait,"backoff does not pollute gameplay slot");
 expect(p.step(1600,false,false,M::Idle,true)==A::Play,"failed-start retry is bounded");
 p.reset();
 expect(p.step(0xfffffff0u,true,false,M::Idle,true)==A::Wait,"busy near tick wrap yields");
 expect(p.step(100,false,false,M::Idle,true)==A::Wait,"cooldown survives tick wrap");
 expect(p.step(1184,false,false,M::Idle,true)==A::Play,"cooldown expires after tick wrap");
 std::puts("PASS: native-action priority, delayed recovery, async starts, no periodic restart, dictionary readiness and tick wrap");
}
