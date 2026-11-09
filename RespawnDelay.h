#pragma once
#include <cstdint>

// Keep the avatar visible throughout GTA's WASTED presentation, then hand off
// only once GTA naturally fades to black. Bound a missing fade without making
// our own black screen; native dead-state preservation owns hospital recovery.
class RespawnDelay {
public:
 enum class Action {Wait,RestoreNative};
 void begin(std::uint32_t now){started=now;running=true;}
 bool active() const{return running;}
 void reset(){running=false;}
 Action step(std::uint32_t now,bool black){
  if(!running||black)return Action::RestoreNative;
  auto elapsed=now-started;
  if(elapsed>=15000)return Action::RestoreNative;
  return Action::Wait;
 }
private:
 bool running=false;std::uint32_t started=0;
};
