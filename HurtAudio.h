#pragma once
#include <mmsystem.h>
#include "HurtReactionPolicy.h"

// Player-local short WAV reactions. No extracted Rockstar audio is distributed.
class HurtAudio {
public:
 void configure(const std::string& folder,bool useHurt,bool trace){root=folder;enabled=useHurt;tracing=trace;random=GetTickCount();}
 bool active(DWORD now) const{return activeUntil&&static_cast<LONG>(now-activeUntil)<0;}
 void update(Ped ped,int avatar){
  DWORD now=GetTickCount();bool owned=avatar>=0&&ENTITY::DOES_ENTITY_EXIST(ped);
  if(!owned){
   if(voicePed&&ENTITY::DOES_ENTITY_EXIST(voicePed))AUDIO::DISABLE_PED_PAIN_AUDIO(voicePed,false);
   voicePed=0;muted=false;policy.sample(now,0,0,0,false);return;
  }
  if(ped!=voicePed){
   voicePed=ped;muted=false;activeUntil=0;
   const char* voices[]={"FRANKLIN_NORMAL","MICHAEL_NORMAL","TREVOR_NORMAL"};
   AUDIO::SET_AMBIENT_VOICE_NAME(ped,(char*)voices[avatar]);AUDIO::DISABLE_PED_PAIN_AUDIO(ped,false);
  }
  // Do not globally disable pain audio: GTA uses this same channel for normal
  // inhale/exhale, exhaustion, climbing and jump effort. Mute only during a hit.
  if(muted&&static_cast<LONG>(now-muteUntil)>=0&&!ENTITY::IS_ENTITY_DEAD(ped)){AUDIO::DISABLE_PED_PAIN_AUDIO(ped,false);muted=false;}
  auto hit=policy.sample(now,ped,ENTITY::GET_ENTITY_HEALTH(ped),PED::GET_PED_ARMOUR(ped),true);
  if(!enabled||!hit.damage)return;
  static const char* files[]={"bark1.wav","bark2.wav","bark3.wav","bark4.wav","whine1.wav","whine2.wav"};
  static const DWORD duration[]={304,409,293,375,759,647};
  random=random*1664525u+1013904223u;int clip=hit.heavy?4+(random>>16)%2:(random>>16)%4;
  if(clip==lastClip)clip=hit.heavy?4+(clip-4+1)%2:(clip+1)%4;
  std::string path=root+files[clip];
  // NOSTOP + policy cooldown prevents stacking or cutting off another WAV.
  bool played=PlaySoundA(path.c_str(),nullptr,SND_FILENAME|SND_ASYNC|SND_NODEFAULT|SND_NOSTOP)!=FALSE;
  if(played){
   AUDIO::DISABLE_PED_PAIN_AUDIO(ped,true);
   invoke<Void>(0x7A73D05A607734C7,ped); // STOP_CURRENT_PLAYING_SPEECH, including the replaced hurt grunt
   muted=true;muteUntil=now+duration[clip]+150;activeUntil=now+duration[clip];policy.played(now,duration[clip]);lastClip=clip;
  }
  if(tracing){FILE* f=nullptr;fopen_s(&f,(root+"audio-test.log").c_str(),"a");if(f){fprintf(f,"tick=%lu avatar=%d damage=%d clip=%s played=%d\n",now,avatar,hit.damage,files[clip],played);fclose(f);}}
 }
private:
 std::string root;bool enabled=true,tracing=false,muted=false;Ped voicePed=0;
 DWORD activeUntil=0,muteUntil=0,random=1;int lastClip=-1;HurtReactionPolicy policy;
};
