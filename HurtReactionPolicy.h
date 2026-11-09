#pragma once
#include <cstdint>

class HurtReactionPolicy {
public:
 struct Hit {int damage=0;bool heavy=false;};
 Hit sample(std::uint32_t now,int ped,int health,int armour,bool owned){
  if(!owned){tracked=0;return {};}
  if(ped!=tracked){tracked=ped;previousHealth=health;previousArmour=armour;cooling=false;return {};}
  int dh=previousHealth-health,da=previousArmour-armour;previousHealth=health;previousArmour=armour;
  int damage=(dh>0?dh:0)+(da>0?da:0);
  if(damage<3||(cooling&&static_cast<std::int32_t>(now-readyAt)<0))return {};
  return {damage,damage>=30||health<=100};
 }
 void played(std::uint32_t now,std::uint32_t duration){cooling=true;readyAt=now+duration+150;}
private:
 int tracked=0,previousHealth=0,previousArmour=0;bool cooling=false;std::uint32_t readyAt=0;
};
