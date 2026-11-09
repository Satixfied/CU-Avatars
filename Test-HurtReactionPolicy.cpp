#include "HurtReactionPolicy.h"
#include <cstdio>
#include <cstdlib>
static void require(bool ok,const char* s){if(!ok){std::fprintf(stderr,"FAIL: %s\n",s);std::exit(1);}}
int main(){HurtReactionPolicy p;
 require(p.sample(0,1,200,50,true).damage==0,"model switch does not count as damage");
 auto hit=p.sample(10,1,190,50,true);require(hit.damage==10&&!hit.heavy,"ordinary hit uses bark category");p.played(10,400);
 require(p.sample(200,1,170,50,true).damage==0,"rapid hits do not stack clips");
 require(p.sample(600,1,180,50,true).damage==0,"healing does not bark");
 hit=p.sample(700,1,180,40,true);require(hit.damage==10&&!hit.heavy,"armour hits count");
 hit=p.sample(800,1,140,40,true);require(hit.heavy,"heavy hits use whine category");
 hit=p.sample(900,1,0,40,true);require(hit.heavy,"fatal hit uses whine category");
 require(p.sample(1000,2,200,0,true).damage==0,"respawn/new ped resets baseline");
 require(p.sample(1100,2,100,0,false).damage==0,"normal protagonists are unaffected");
 std::puts("PASS: damage/armour detection, severity, cooldown, healing, model changes, native-player exclusion");}
