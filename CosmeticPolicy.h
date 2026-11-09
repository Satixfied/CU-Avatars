#pragma once
#include <cstdint>

// No GTA dependencies: the decision policy can be regression-tested offline.
// Gameplay owns the shared secondary task slot. Cosmetic clips are expendable.
class CosmeticPolicy {
public:
 enum class Mode {None,Idle,Talk,Closed};
 enum class Action {Wait,Play,Stop};
 Mode current() const {return mode;}
 void reset(){mode=Mode::None;cooling=false;starting=false;}
 Action step(std::uint32_t now,bool gameplayBusy,bool ownClipPlaying,Mode desired,bool dictionaryReady){
  if(gameplayBusy){
   bool stop=mode!=Mode::None;mode=Mode::None;defer(now);
   return stop?Action::Stop:Action::Wait;
  }
  // TASK_PLAY_ANIM is asynchronous. Give a newly submitted task a few frames
  // before treating a missing clip as an interruption or a failed start.
  if(starting&&reached(now,startDeadline))starting=false;
  if(mode!=Mode::None&&!ownClipPlaying&&!starting){mode=Mode::None;defer(now);return Action::Wait;}
  if(cooling){if(!reached(now,resumeDeadline))return Action::Wait;cooling=false;}
  if(!dictionaryReady||desired==Mode::None)return Action::Wait;
  if(mode!=desired){mode=desired;starting=true;startDeadline=now+300;return Action::Play;}
  return Action::Wait; // the native loop runs by itself; never resubmit per second
 }
private:
 Mode mode=Mode::None;
 bool cooling=false,starting=false;
 std::uint32_t resumeDeadline=0,startDeadline=0;
 static bool reached(std::uint32_t now,std::uint32_t deadline){return static_cast<std::int32_t>(now-deadline)>=0;}
 void defer(std::uint32_t now){cooling=true;starting=false;resumeDeadline=now+1200;}
};
