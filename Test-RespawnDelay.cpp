#include "RespawnDelay.h"
#include <cstdio>
#include <cstdlib>
using A=RespawnDelay::Action;
static void require(bool ok,const char* text){if(!ok){std::fprintf(stderr,"FAIL: %s\n",text);std::exit(1);}}
int main(){RespawnDelay p;p.begin(100);
 require(p.step(100,false)==A::Wait,"do not instantly replace the dying avatar");
 require(p.step(1000,false)==A::Wait,"do not expose the native protagonist mid-ragdoll");
 require(p.step(10000,false)==A::Wait,"keep avatar through a long WASTED camera");
 require(p.step(500,true)==A::RestoreNative,"restore immediately when naturally black");
 p.begin(100);require(p.step(15099,false)==A::Wait,"keep avatar until fade or bounded timeout");
 require(p.step(15100,false)==A::RestoreNative,"missing natural fade has a finite safety handoff without forced black");
 p.begin(100);require(p.step(9000,true)==A::RestoreNative,"switch invisibly on native WASTED fade");
 p.begin(0xfffffff0u);require(p.step(100,false)==A::Wait,"visible delay survives tick wrap");
 require(p.step(14984,false)==A::RestoreNative,"native handoff safety timer survives tick wrap");
 p.reset();require(p.step(0,false)==A::RestoreNative,"manual/immediate recovery remains available");
 std::puts("PASS: avatar through WASTED, handoff only on native black, bounded fallback, no forced death fade, manual recovery, tick wrap");}
