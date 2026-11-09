// CU Avatars: optional add-on player models for offline free roam.
// Script Hook V SDK is an external build dependency, not part of this release.
#define NOMINMAX
#include "main.h"
#include "natives.h"
#include <algorithm>
#include <array>
#include <string>
#include <vector>
#include <cstdio>
#include <cstddef>
#include <cstdlib>
#include "WeaponCatalog.h"
#include "CosmeticPolicy.h"
#include "RespawnDelay.h"
#include "HurtAudio.h"
#include "ManualSwitchStamp.h"

static ManualSwitchStamp manualSwitch;
extern "C" __declspec(dllexport) DWORD WINAPI CUAvatars_QueryManualSwitch(int previous,int current){
 return manualSwitch.query(previous,current,GetTickCount());
}
static void releaseFilmingCameraForMenu(){
 HMODULE creator=GetModuleHandleA("CUPhotoStage.asi");
 if(creator){using Release=void(WINAPI*)();auto release=(Release)GetProcAddress(creator,"CUPhotoStage_ReleaseForAvatarMenu");if(release)release();}
}
static bool filmingScreenIsClean(){
 HMODULE creator=GetModuleHandleA("CUPhotoStage.asi");
 if(!creator)return false;using Query=BOOL(WINAPI*)();auto query=(Query)GetProcAddress(creator,"CUPhotoStage_IsCleanScreen");return query&&query();
}

static volatile LONG inputBits=0, menuVisible=0;
static int menuKey=VK_F5, restoreKey=VK_F6, blinkKey=VK_F7;
static std::string iniPath,statePath;
static std::string recoveryLogPath;
static std::string hurtSoundPath;
static HurtAudio hurtAudio;
static Hash recoveryOutfitModel=0;
static bool recoveryHasCachedOutfit=false,autoReturnAvatar=true,showAvatarDeath=true;
static int pendingRespawnAvatar=-1;
static Hash heldRespawnAvatar=0;
static RespawnDelay respawnDelay;
static DWORD recoveryWaitSince=0,recoveryTraceAt=0;
extern "C" __declspec(dllexport) BOOL WINAPI CUAvatars_IsRecoveryPending(){return recoveryOutfitModel||respawnDelay.active()||pendingRespawnAvatar>=0;}
static std::string animationTracePath;
static bool animationTrace=false;
static DWORD nextAnimationTrace=0;
static DWORD animationTraceEnd=0;
static int selection=0;
static Hash originalModel=0;
static bool haveOriginal=false;
static Ped trackedPed=0;
static CosmeticPolicy cosmeticPolicy;
static bool primeCosmeticClock=false;
static DWORD blinkUntil=0, animRetryAt=0, nextSpeechCheck=0;
static bool speaking=false;
struct Avatar {const char* title;const char* model;const char* dict;const char* prefix;Hash hash;};
static Avatar avatars[]={
 {"Expie","cu_expie","cu_expie_faces","expie",0},
 {"Milky","cu_milky","cu_milky_faces","milky",0},
 {"Dune","cu_dune","cu_dune_faces","dune",0}
};
struct Clothes {int drawable,texture,palette;};
struct Prop {int drawable,texture;};
static std::array<Clothes,12> originalClothes;
static std::array<Prop,8> originalProps;
struct WeaponState {Hash hash;int ammo,clip,tint;std::vector<Hash> components;};
struct DlcWeaponData {int empty,pad0;Hash hash;int pad1;int unused,pad2,cost,pad3,ammoCost,pad4,ammoType,pad5,clip,pad6;char labels[256];};
struct DlcComponentData {int bone,pad0,active,pad1,unused,pad2;Hash hash;int pad3,cost0,pad4,cost,pad5;char labels[128];};
static_assert(offsetof(DlcWeaponData,hash)==8 && sizeof(DlcWeaponData)==312,"DLC weapon layout");
static_assert(offsetof(DlcComponentData,hash)==24 && sizeof(DlcComponentData)==176,"DLC component layout");
struct CatalogWeapon {Hash hash;std::vector<Hash> components;};
static std::vector<CatalogWeapon> catalog;

static void notice(const char* text){if(filmingScreenIsClean())return;UI::_SET_NOTIFICATION_TEXT_ENTRY("STRING");UI::_ADD_TEXT_COMPONENT_STRING((char*)text);UI::_DRAW_NOTIFICATION(false,false);}
static bool offline(){return !NETWORK::NETWORK_IS_SESSION_ACTIVE();}
static bool playable(){return offline()&&PLAYER::IS_PLAYER_PLAYING(PLAYER::PLAYER_ID())&&ENTITY::DOES_ENTITY_EXIST(PLAYER::PLAYER_PED_ID())&&PLAYER::IS_PLAYER_CONTROL_ON(PLAYER::PLAYER_ID())&&!UI::IS_PAUSE_MENU_ACTIVE()&&!STREAMING::IS_PLAYER_SWITCH_IN_PROGRESS()&&!CUTSCENE::IS_CUTSCENE_ACTIVE();}
static bool freeRoam(){return playable()&&!GAMEPLAY::GET_MISSION_FLAG();}
static int avatarIndex(Hash hash){for(int i=0;i<3;i++)if(avatars[i].hash==hash)return i;return -1;}
static void keyboard(DWORD key,WORD,BYTE,BOOL,BOOL withAlt,BOOL wasDown,BOOL isUp){
 if(withAlt||isUp||wasDown)return;
 LONG bit=key==(DWORD)menuKey?1:key==(DWORD)restoreKey?2:key==(DWORD)blinkKey?4:0;
 if(InterlockedCompareExchange(&menuVisible,0,0)){
  if(key==VK_UP)bit=8;else if(key==VK_DOWN)bit=16;else if(key==VK_RETURN)bit=32;else if(key==VK_BACK||key==VK_ESCAPE)bit=64;
 }
 if(bit)InterlockedOr(&inputBits,bit);
}
static void buildCatalog(){
 for(auto hash:baseWeapons){CatalogWeapon w{hash,{}};for(auto c:baseComponents)if(WEAPON::DOES_WEAPON_TAKE_WEAPON_COMPONENT(hash,c))w.components.push_back(c);catalog.push_back(w);}
 int count=std::clamp(DLC1::GET_NUM_DLC_WEAPONS(),0,2048);
 for(int i=0;i<count;i++){
  DlcWeaponData d{};if(!DLC1::GET_DLC_WEAPON_DATA(i,(Any*)&d)||!d.hash)continue;
  auto it=std::find_if(catalog.begin(),catalog.end(),[&](const auto& w){return w.hash==d.hash;});
  if(it==catalog.end()){catalog.push_back({d.hash,{}});it=catalog.end()-1;}
  for(int j=0,n=std::clamp(DLC1::GET_NUM_DLC_WEAPON_COMPONENTS(i),0,512);j<n;j++){
   DlcComponentData c{};if(DLC1::GET_DLC_WEAPON_COMPONENT_DATA(i,j,(Any*)&c)&&c.hash&&std::find(it->components.begin(),it->components.end(),c.hash)==it->components.end())it->components.push_back(c.hash);
  }
 }
}
static std::vector<WeaponState> weapons(Ped ped,Hash selected){
 std::vector<WeaponState> out;
 for(const auto& c:catalog)if(WEAPON::HAS_PED_GOT_WEAPON(ped,c.hash,false)){
  WeaponState w{c.hash,WEAPON::GET_AMMO_IN_PED_WEAPON(ped,c.hash),0,WEAPON::GET_PED_WEAPON_TINT_INDEX(ped,c.hash),{}};
  WEAPON::GET_AMMO_IN_CLIP(ped,c.hash,&w.clip);
  for(auto h:c.components)if(WEAPON::HAS_PED_GOT_WEAPON_COMPONENT(ped,c.hash,h))w.components.push_back(h);
  out.push_back(w);
 }
 if(selected&&selected!=0xA2719263&&std::none_of(out.begin(),out.end(),[&](const auto& w){return w.hash==selected;})){
  WeaponState w{selected,WEAPON::GET_AMMO_IN_PED_WEAPON(ped,selected),0,WEAPON::GET_PED_WEAPON_TINT_INDEX(ped,selected),{}};WEAPON::GET_AMMO_IN_CLIP(ped,selected,&w.clip);out.push_back(w);
 }
 return out;
}
static void saveOriginal(){
 WritePrivateProfileStringA("Restore","Pending","0",statePath.c_str());
 char value[96],key[32];sprintf_s(value,"%u",originalModel);WritePrivateProfileStringA("Restore","Model",value,statePath.c_str());
 for(int i=0;i<12;i++){auto c=originalClothes[i];sprintf_s(key,"Clothes%d",i);sprintf_s(value,"%d,%d,%d",c.drawable,c.texture,c.palette);WritePrivateProfileStringA("Restore",key,value,statePath.c_str());}
 for(int i=0;i<8;i++){auto p=originalProps[i];sprintf_s(key,"Prop%d",i);sprintf_s(value,"%d,%d",p.drawable,p.texture);WritePrivateProfileStringA("Restore",key,value,statePath.c_str());}
 WritePrivateProfileStringA("Restore","Pending","1",statePath.c_str());
}
static void loadOriginal(){
 if(!GetPrivateProfileIntA("Restore","Pending",0,statePath.c_str()))return;
 char value[96],key[32];GetPrivateProfileStringA("Restore","Model","0",value,sizeof(value),statePath.c_str());Hash model=(Hash)strtoul(value,nullptr,10);
 if(!model||avatarIndex(model)>=0||!STREAMING::IS_MODEL_IN_CDIMAGE(model)||!STREAMING::IS_MODEL_VALID(model))return;
 originalModel=model;
 for(int i=0;i<12;i++){auto& c=originalClothes[i];c={0,0,0};sprintf_s(key,"Clothes%d",i);GetPrivateProfileStringA("Restore",key,"0,0,0",value,sizeof(value),statePath.c_str());sscanf_s(value,"%d,%d,%d",&c.drawable,&c.texture,&c.palette);c.drawable=std::clamp(c.drawable,0,255);c.texture=std::clamp(c.texture,0,255);c.palette=std::clamp(c.palette,0,3);}
 for(int i=0;i<8;i++){auto& p=originalProps[i];p={-1,0};sprintf_s(key,"Prop%d",i);GetPrivateProfileStringA("Restore",key,"-1,0",value,sizeof(value),statePath.c_str());sscanf_s(value,"%d,%d",&p.drawable,&p.texture);p.drawable=std::clamp(p.drawable,-1,255);p.texture=std::clamp(p.texture,0,255);}
 haveOriginal=true;
}
static void rememberOriginal(Ped ped,Hash hash){
 originalModel=hash;haveOriginal=true;
 for(int i=0;i<12;i++)originalClothes[i]={PED::GET_PED_DRAWABLE_VARIATION(ped,i),PED::GET_PED_TEXTURE_VARIATION(ped,i),PED::GET_PED_PALETTE_VARIATION(ped,i)};
 for(int i=0;i<8;i++)originalProps[i]={PED::GET_PED_PROP_INDEX(ped,i),PED::GET_PED_PROP_TEXTURE_INDEX(ped,i)};
 saveOriginal();
}
static bool protagonist(Hash model){return model==0x0D7114C9||model==0x9B22DBAF||model==0x9B810FA2;}
static void applyOriginalAppearance(Ped ped){
 for(int i=0;i<12;i++){auto c=originalClothes[i];if(c.drawable>=0)PED::SET_PED_COMPONENT_VARIATION(ped,i,c.drawable,c.texture,c.palette);}
 PED::CLEAR_ALL_PED_PROPS(ped);for(int i=0;i<8;i++){auto p=originalProps[i];if(p.drawable>=0)PED::SET_PED_PROP_INDEX(ped,i,p.drawable,p.texture,true);}
}
static void recoveryEvent(const char* event,Hash model){
 FILE* file=nullptr;fopen_s(&file,recoveryLogPath.c_str(),"a");if(file){fprintf(file,"tick=%lu event=%s model=%u\n",GetTickCount(),event,model);fclose(file);}
}
static void recoveryTrace(Ped ped){
 if(!recoveryOutfitModel||GetTickCount()<recoveryTraceAt)return;
 DWORD now=GetTickCount();recoveryTraceAt=now+(now-recoveryWaitSince<30000?1000:5000);
 auto p=ENTITY::GET_ENTITY_COORDS(ped,true);FILE* f=nullptr;fopen_s(&f,recoveryLogPath.c_str(),"a");if(!f)return;
 fprintf(f,"tick=%lu recovery-state model=%u dead=%d playing=%d control=%d mission=%d cutscene=%d black=%d health=%d position=%.2f,%.2f,%.2f\n",now,ENTITY::GET_ENTITY_MODEL(ped),PLAYER::IS_PLAYER_DEAD(PLAYER::PLAYER_ID())||ENTITY::IS_ENTITY_DEAD(ped),PLAYER::IS_PLAYER_PLAYING(PLAYER::PLAYER_ID()),PLAYER::IS_PLAYER_CONTROL_ON(PLAYER::PLAYER_ID()),GAMEPLAY::GET_MISSION_FLAG(),CUTSCENE::IS_CUTSCENE_ACTIVE(),CAM::IS_SCREEN_FADED_OUT(),ENTITY::GET_ENTITY_HEALTH(ped),p.x,p.y,p.z);fclose(f);
}
static bool changeModel(int choice,bool quiet=false);
static void clearPendingAvatar(){
 if(heldRespawnAvatar)STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(heldRespawnAvatar);
 heldRespawnAvatar=0;pendingRespawnAvatar=-1;respawnDelay.reset();
}
static bool recoverOriginalIfNeeded(bool manual=false){
 // Deliberately outside playable()/freeRoam(): they are false during death,
 // arrest and the broken loading transition this safeguard must recover.
 if(!offline())return false;
 Ped old=PLAYER::PLAYER_PED_ID();if(!ENTITY::DOES_ENTITY_EXIST(old))return false;
 Hash oldModel=ENTITY::GET_ENTITY_MODEL(old);
 bool dead=ENTITY::IS_ENTITY_DEAD(old)||PLAYER::IS_PLAYER_DEAD(PLAYER::PLAYER_ID());
 bool arrested=PLAYER::IS_PLAYER_BEING_ARRESTED(PLAYER::PLAYER_ID(),true)!=0;
 if(manual&&(recoveryOutfitModel||respawnDelay.active())){clearPendingAvatar();}
 recoveryTrace(old);
 bool hospitalRevertedToAvatar=recoveryOutfitModel&&avatarIndex(oldModel)>=0&&!dead&&!arrested;
 if(recoveryOutfitModel&&oldModel==recoveryOutfitModel&&freeRoam()&&!dead&&!arrested&&!InterlockedCompareExchange(&menuVisible,0,0)){
  if(oldModel==recoveryOutfitModel&&recoveryHasCachedOutfit)applyOriginalAppearance(old);
  recoveryEvent("native-respawn-ready",oldModel);recoveryOutfitModel=0;
  int choice=pendingRespawnAvatar;pendingRespawnAvatar=-1;respawnDelay.reset();
  if(choice>=0){
   // Native death/arrest processing is complete. The avatar was kept loaded,
   // so the final switch occurs behind a short fade without another load wait.
   CAM::DO_SCREEN_FADE_OUT(0);bool ok=changeModel(choice,true);CAM::DO_SCREEN_FADE_IN(180);
   recoveryEvent(ok?"avatar-reapplied":"avatar-reapply-failed",avatars[choice].hash);
   clearPendingAvatar();return true;
  }
  clearPendingAvatar();notice("Original character restored for respawn.");
 }
 bool emergency=manual&&!freeRoam()&&!UI::IS_PAUSE_MENU_ACTIVE()&&!CUTSCENE::IS_CUTSCENE_ACTIVE();
 int avatar=avatarIndex(oldModel);
 if(avatar<0||(!dead&&!arrested&&!emergency&&!hospitalRevertedToAvatar))return false;
 if(!haveOriginal)loadOriginal();
 Hash model=recoveryOutfitModel?recoveryOutfitModel:haveOriginal&&protagonist(originalModel)?originalModel:0x0D7114C9; // Michael fallback
 bool cachedOutfit=recoveryOutfitModel?recoveryHasCachedOutfit:haveOriginal&&model==originalModel;
 if(!respawnDelay.active()&&!recoveryOutfitModel){
  recoveryEvent(dead?"death-detected":arrested?"arrest-detected":"manual-recovery",model);
  respawnDelay.begin((DWORD)GAMEPLAY::GET_GAME_TIMER());
  if(autoReturnAvatar&&!emergency){pendingRespawnAvatar=avatar;heldRespawnAvatar=avatars[avatar].hash;STREAMING::REQUEST_MODEL(heldRespawnAvatar);}
  STREAMING::REQUEST_MODEL(model);InterlockedExchange(&menuVisible,0);
  // Stop only cosmetic playback, leaving GTA's actual ragdoll/death pose alone.
  auto mode=cosmeticPolicy.current();if(mode!=CosmeticPolicy::Mode::None){
   const char* suffix=mode==CosmeticPolicy::Mode::Closed?"closed":mode==CosmeticPolicy::Mode::Talk?"talk":"idle";
   char clip[64];sprintf_s(clip,"%s_%s",avatars[avatar].prefix,suffix);AI::STOP_ANIM_TASK(old,(char*)avatars[avatar].dict,clip,-4);
  }
  cosmeticPolicy.reset();primeCosmeticClock=false;
 }
 if(dead&&showAvatarDeath&&!emergency&&!arrested&&!recoveryOutfitModel){
  auto action=respawnDelay.step((DWORD)GAMEPLAY::GET_GAME_TIMER(),CAM::IS_SCREEN_FADED_OUT()!=0);
  if(action==RespawnDelay::Action::Wait)return true;
 }
 InterlockedExchange(&menuVisible,0);STREAMING::REQUEST_MODEL(model);DWORD deadline=GetTickCount()+15000;
 while(!STREAMING::HAS_MODEL_LOADED(model)&&GetTickCount()<deadline){if(!offline())return false;WAIT(0);}
 if(!STREAMING::HAS_MODEL_LOADED(model)){recoveryEvent("model-load-timeout",model);STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(model);return true;}
 // Follow the SDK trainer's death/arrest model safeguard. Do not bypass the
 // hospital/busted flow with manual resurrection, healing, teleports or globals.
 old=PLAYER::PLAYER_PED_ID();auto pos=ENTITY::GET_ENTITY_COORDS(old,true);float direction=ENTITY::GET_ENTITY_HEADING(old);
 Hash selected=WEAPON::GET_SELECTED_PED_WEAPON(old);auto loadout=weapons(old,selected);
 PLAYER::SET_PLAYER_MODEL(PLAYER::PLAYER_ID(),model);Ped ped=PLAYER::PLAYER_PED_ID();
 PED::SET_PED_DEFAULT_COMPONENT_VARIATION(ped);if(cachedOutfit)applyOriginalAppearance(ped);
 ENTITY::SET_ENTITY_COORDS_NO_OFFSET(ped,pos.x,pos.y,pos.z,false,false,false);ENTITY::SET_ENTITY_HEADING(ped,direction);
 for(const auto& w:loadout){WEAPON::GIVE_WEAPON_TO_PED(ped,w.hash,w.ammo,false,false);for(auto c:w.components)WEAPON::GIVE_WEAPON_COMPONENT_TO_PED(ped,w.hash,c);WEAPON::SET_PED_WEAPON_TINT_INDEX(ped,w.hash,w.tint);WEAPON::SET_PED_AMMO(ped,w.hash,w.ammo);WEAPON::SET_AMMO_IN_CLIP(ped,w.hash,w.clip);}
 WEAPON::SET_CURRENT_PED_WEAPON(ped,selected,true);STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(model);
 // SET_PLAYER_MODEL must not accidentally revive the player at the death spot.
 // Keep the native protagonist dead in this same frame; GTA owns the hospital
 // relocation, fees and resurrection. Arrest/manual living recovery stays alive.
 if(dead)ENTITY::SET_ENTITY_HEALTH(ped,0);
 trackedPed=0;cosmeticPolicy.reset();primeCosmeticClock=false;animRetryAt=0;
 if(!recoveryOutfitModel)recoveryWaitSince=GetTickCount();
 recoveryOutfitModel=model;recoveryHasCachedOutfit=cachedOutfit;recoveryTraceAt=0;
 respawnDelay.reset();
 haveOriginal=false;WritePrivateProfileStringA("Restore","Pending","0",statePath.c_str());
 recoveryEvent("protagonist-restored-awaiting-native-flow",model);return true;
}
static bool changeModel(int choice,bool quiet){
 if(!freeRoam()){notice("Switch avatars in singleplayer free roam, outside missions and cutscenes.");return false;}
 Ped old=PLAYER::PLAYER_PED_ID();Hash oldHash=ENTITY::GET_ENTITY_MODEL(old);
 if(PED::IS_PED_RUNNING_MOBILE_PHONE_TASK(old)||AUDIO::IS_MOBILE_PHONE_CALL_ONGOING()){
  notice("Put the phone away before switching avatars.");return false;
 }
 bool restore=choice==3;if(restore&&!haveOriginal){notice("You are already using your original character.");return false;}
 Hash wanted=restore?originalModel:avatars[choice].hash;
 if(wanted==oldHash)return true;
 if(!STREAMING::IS_MODEL_IN_CDIMAGE(wanted)||!STREAMING::IS_MODEL_VALID(wanted)){notice("Avatar model is missing. Install the CU Avatars DLC and dlclist entry first.");return false;}
 STREAMING::REQUEST_MODEL(wanted);DWORD deadline=GetTickCount()+15000;
 while(!STREAMING::HAS_MODEL_LOADED(wanted)&&GetTickCount()<deadline){WAIT(0);if(!freeRoam()){STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(wanted);return false;}}
 if(!STREAMING::HAS_MODEL_LOADED(wanted)){STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(wanted);notice("Avatar streaming timed out. Your character was not changed.");return false;}
 if(!freeRoam()){STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(wanted);return false;}
 old=PLAYER::PLAYER_PED_ID();oldHash=ENTITY::GET_ENTITY_MODEL(old);
 if(PED::IS_PED_RAGDOLL(old)||PED::IS_PED_GETTING_INTO_A_VEHICLE(old)||PED::IS_PED_JUMPING_OUT_OF_VEHICLE(old)){
  STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(wanted);notice("Finish the fall or vehicle transition before switching avatars.");return false;
 }
 if(!restore&&avatarIndex(oldHash)<0)rememberOriginal(old,oldHash);
 auto pos=ENTITY::GET_ENTITY_COORDS(old,true);auto velocity=ENTITY::GET_ENTITY_VELOCITY(old);float heading=ENTITY::GET_ENTITY_HEADING(old);
 bool wasSkydiving=PED::IS_PED_IN_PARACHUTE_FREE_FALL(old)!=0;int parachuteState=PED::GET_PED_PARACHUTE_STATE(old);
 int health=ENTITY::GET_ENTITY_HEALTH(old),maxHealth=ENTITY::GET_ENTITY_MAX_HEALTH(old),armour=PED::GET_PED_ARMOUR(old);
 Hash selected=WEAPON::GET_SELECTED_PED_WEAPON(old);auto loadout=weapons(old,selected);
 Vehicle car=0;int seat=-2;if(PED::IS_PED_IN_ANY_VEHICLE(old,false)){car=PED::GET_VEHICLE_PED_IS_IN(old,false);for(int i=-1;i<VEHICLE::GET_VEHICLE_MAX_NUMBER_OF_PASSENGERS(car);i++)if(VEHICLE::GET_PED_IN_VEHICLE_SEAT(car,i)==old){seat=i;break;}}
 if(car&&seat==-2){STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(wanted);notice("Finish entering or leaving the vehicle before switching.");return false;}
 Hash vehicleModel=car?ENTITY::GET_ENTITY_MODEL(car):0;
 bool mountedSwap=car&&(VEHICLE::IS_THIS_MODEL_A_BICYCLE(vehicleModel)||VEHICLE::IS_THIS_MODEL_A_BIKE(vehicleModel)||VEHICLE::IS_THIS_MODEL_A_QUADBIKE(vehicleModel));
 bool swapInvincible=false,swapFade=false;Vector3 vehicleVelocity{};
 auto finishMountedSwap=[&](Ped rider){
  if(!mountedSwap)return;
  if(ENTITY::DOES_ENTITY_EXIST(car)&&ENTITY::DOES_ENTITY_EXIST(rider)&&!ENTITY::IS_ENTITY_DEAD(rider)&&VEHICLE::IS_VEHICLE_SEAT_FREE(car,seat))PED::SET_PED_INTO_VEHICLE(rider,car,seat);
  if(ENTITY::DOES_ENTITY_EXIST(old))ENTITY::FREEZE_ENTITY_POSITION(old,false);
  if(ENTITY::DOES_ENTITY_EXIST(car)){ENTITY::FREEZE_ENTITY_POSITION(car,false);ENTITY::SET_ENTITY_VELOCITY(car,vehicleVelocity.x,vehicleVelocity.y,vehicleVelocity.z);}
  if(ENTITY::DOES_ENTITY_EXIST(rider))ENTITY::FREEZE_ENTITY_POSITION(rider,false);
  PLAYER::SET_PLAYER_INVINCIBLE(PLAYER::PLAYER_ID(),swapInvincible);PLAYER::SET_PLAYER_CONTROL(PLAYER::PLAYER_ID(),true,0);
  if(swapFade)CAM::DO_SCREEN_FADE_IN(180);
 };
 if(mountedSwap){
  // Cycling tasks retain rider animation/IK references. Detach while the old
  // model is still valid and let that task retire before replacing the model.
  vehicleVelocity=ENTITY::GET_ENTITY_VELOCITY(car);swapInvincible=PLAYER::GET_PLAYER_INVINCIBLE(PLAYER::PLAYER_ID())!=0;swapFade=CAM::IS_SCREEN_FADED_IN()!=0;
  PLAYER::SET_PLAYER_INVINCIBLE(PLAYER::PLAYER_ID(),true);PLAYER::SET_PLAYER_CONTROL(PLAYER::PLAYER_ID(),false,0);ENTITY::FREEZE_ENTITY_POSITION(car,true);
  if(swapFade){CAM::DO_SCREEN_FADE_OUT(100);WAIT(120);}
  AI::TASK_LEAVE_VEHICLE(old,car,16);DWORD until=GetTickCount()+1500;
  while(PED::IS_PED_IN_VEHICLE(old,car,false)&&GetTickCount()<until)WAIT(0);
  if(PED::IS_PED_IN_VEHICLE(old,car,false)||!offline()||PLAYER::PLAYER_PED_ID()!=old||ENTITY::IS_ENTITY_DEAD(old)||UI::IS_PAUSE_MENU_ACTIVE()||GAMEPLAY::GET_MISSION_FLAG()||CUTSCENE::IS_CUTSCENE_ACTIVE()){
   finishMountedSwap(old);STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(wanted);notice("Bike switch cancelled safely. Try again after the current action ends.");return false;
  }
  AI::CLEAR_PED_TASKS_IMMEDIATELY(old);ENTITY::FREEZE_ENTITY_POSITION(old,true);WAIT(0);WAIT(0);
  if(!offline()||PLAYER::PLAYER_PED_ID()!=old||ENTITY::IS_ENTITY_DEAD(old)||UI::IS_PAUSE_MENU_ACTIVE()||GAMEPLAY::GET_MISSION_FLAG()||CUTSCENE::IS_CUTSCENE_ACTIVE()){
   finishMountedSwap(old);STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(wanted);return false;
  }
 }
 AI::CLEAR_PED_SECONDARY_TASK(old);PLAYER::SET_PLAYER_MODEL(PLAYER::PLAYER_ID(),wanted);
 Ped ped=PLAYER::PLAYER_PED_ID();PED::SET_PED_DEFAULT_COMPONENT_VARIATION(ped);
 if(restore){
  applyOriginalAppearance(ped);
 }
 ENTITY::SET_ENTITY_COORDS_NO_OFFSET(ped,pos.x,pos.y,pos.z,false,false,false);ENTITY::SET_ENTITY_HEADING(ped,heading);
 if(car&&ENTITY::DOES_ENTITY_EXIST(car))PED::SET_PED_INTO_VEHICLE(ped,car,seat);else ENTITY::SET_ENTITY_VELOCITY(ped,velocity.x,velocity.y,velocity.z);
 ENTITY::SET_ENTITY_MAX_HEALTH(ped,maxHealth);ENTITY::SET_ENTITY_HEALTH(ped,health);PED::SET_PED_ARMOUR(ped,armour);
 for(const auto& w:loadout){WEAPON::GIVE_WEAPON_TO_PED(ped,w.hash,w.ammo,false,false);for(auto c:w.components)WEAPON::GIVE_WEAPON_COMPONENT_TO_PED(ped,w.hash,c);WEAPON::SET_PED_WEAPON_TINT_INDEX(ped,w.hash,w.tint);WEAPON::SET_PED_AMMO(ped,w.hash,w.ammo);WEAPON::SET_AMMO_IN_CLIP(ped,w.hash,w.clip);}
 WEAPON::SET_CURRENT_PED_WEAPON(ped,selected,true);STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(wanted);
 if(!car){if(parachuteState==1||parachuteState==2)AI::TASK_PARACHUTE(ped,true);else if(wasSkydiving)AI::TASK_SKY_DIVE(ped);}
 trackedPed=0;cosmeticPolicy.reset();primeCosmeticClock=false;animRetryAt=0;
 if(!quiet){if(recoveryOutfitModel){recoveryOutfitModel=0;clearPendingAvatar();}manualSwitch.publish(old,ped,GetTickCount());}
 if(mountedSwap){WAIT(0);if(ENTITY::DOES_ENTITY_EXIST(car)&&VEHICLE::GET_PED_IN_VEHICLE_SEAT(car,seat)!=ped)PED::SET_PED_INTO_VEHICLE(ped,car,seat);finishMountedSwap(ped);}
 if(restore){haveOriginal=false;WritePrivateProfileStringA("Restore","Pending","0",statePath.c_str());if(!quiet)notice("Original character restored.");}else if(!quiet)notice("Avatar selected. Restore your original character before starting a story mission.");
 return true;
}
static void text(float x,float y,const char* message,float scale=0.34f){UI::SET_TEXT_FONT(0);UI::SET_TEXT_SCALE(scale,scale);UI::SET_TEXT_COLOUR(255,255,255,255);UI::SET_TEXT_OUTLINE();UI::_SET_TEXT_ENTRY("STRING");UI::_ADD_TEXT_COMPONENT_STRING((char*)message);UI::_DRAW_TEXT(x,y);}
static void drawMenu(){
 GRAPHICS::DRAW_RECT(.17f,.14f,.27f,.058f,54,26,83,235);text(.047f,.117f,"CU Avatars - Singleplayer",.39f);
 const char* names[]={"Expie","Milky","Dune","Restore original character"};
 for(int i=0;i<4;i++){float y=.194f+i*.046f;GRAPHICS::DRAW_RECT(.17f,y,.27f,.043f,i==selection?91:18,i==selection?49:18,i==selection?122:24,230);text(.047f,y-.015f,names[i]);}
 text(.047f,.385f,"Up/Down: choose   Enter: select   Backspace: close",.27f);
 const int controls[]={24,25,27,30,31,32,33,34,35,75,172,173,191,194,199,200};for(auto c:controls)CONTROLS::DISABLE_CONTROL_ACTION(0,c,true);
}
static const char* cosmeticSuffix(CosmeticPolicy::Mode mode){return mode==CosmeticPolicy::Mode::Closed?"closed":mode==CosmeticPolicy::Mode::Talk?"talk":"idle";}
static void animate(Ped ped,int index){
 if(index<0)return;
 auto& a=avatars[index];DWORD now=GetTickCount();
 if(trackedPed!=ped){trackedPed=ped;cosmeticPolicy.reset();primeCosmeticClock=false;}
 auto oldMode=cosmeticPolicy.current();char ownClip[64];sprintf_s(ownClip,"%s_%s",a.prefix,cosmeticSuffix(oldMode));
 bool ownPlaying=oldMode!=CosmeticPolicy::Mode::None&&ENTITY::IS_ENTITY_PLAYING_ANIM(ped,(char*)a.dict,ownClip,3);
 if(primeCosmeticClock&&ownPlaying){
  // A queued TASK_PLAY_ANIM is not live until the next engine update. Prime
  // its clock ONCE after it actually starts, including a post-phone restart.
  ENTITY::SET_ENTITY_ANIM_CURRENT_TIME(ped,(char*)a.dict,ownClip,0.0f);
  ENTITY::SET_ENTITY_ANIM_SPEED(ped,(char*)a.dict,ownClip,1.0f);
  primeCosmeticClock=false;
 }
 // TASK_PLAY_ANIM with AF_SECONDARY shares a task slot with the phone and
 // other upper-body actions even when a clip keys only custom bones. Yield
 // BEFORE attempting a clip restart; otherwise GTA repeatedly redraws its phone.
 // Do not use CAN_PHONE_BE_SEEN_ON_SCREEN: that native always returns true.
 bool actionInput=false;
 if(!InterlockedCompareExchange(&menuVisible,0,0)){
  // Check intent before GTA creates its task, including phone draw, attacks,
  // aiming, reload, weapon wheel, jumping, cover and getting in/out of vehicles.
  const int actions[]={27,24,25,45,37,22,23,44,75};
  for(auto control:actions)if(CONTROLS::IS_CONTROL_PRESSED(0,control)){actionInput=true;break;}
 }
 int parachuteState=PED::GET_PED_PARACHUTE_STATE(ped);
 bool nativeAction=!playable()||GAMEPLAY::GET_MISSION_FLAG()||actionInput||
  PED::IS_PED_RAGDOLL(ped)||PED::IS_PED_INJURED(ped)||PED::IS_PED_RUNNING_MOBILE_PHONE_TASK(ped)||
  AUDIO::IS_MOBILE_PHONE_CALL_ONGOING()||AI::IS_PLAYING_PHONE_GESTURE_ANIM(ped)||
  PED::IS_PED_RELOADING(ped)||PED::IS_PED_SHOOTING(ped)||PLAYER::IS_PLAYER_FREE_AIMING(PLAYER::PLAYER_ID())||
  PLAYER::IS_PLAYER_TARGETTING_ANYTHING(PLAYER::PLAYER_ID())||PED::IS_PED_IN_MELEE_COMBAT(ped)||
  PED::IS_PED_PLANTING_BOMB(ped)||PED::IS_PED_IN_COVER(ped,false)||PED::IS_PED_GOING_INTO_COVER(ped)||
  PED::IS_PED_CLIMBING(ped)||PED::IS_PED_VAULTING(ped)||PED::IS_PED_JUMPING(ped)||PED::IS_PED_FALLING(ped)||
  PED::IS_PED_DIVING(ped)||PED::IS_PED_SWIMMING(ped)||PED::IS_PED_IN_PARACHUTE_FREE_FALL(ped)||
  (parachuteState>=1&&parachuteState<=3)||PED::IS_PED_GETTING_INTO_A_VEHICLE(ped)||
  PED::IS_PED_JUMPING_OUT_OF_VEHICLE(ped)||PED::IS_PED_HANGING_ON_TO_VEHICLE(ped)||
  PED::IS_PED_JACKING(ped)||PED::IS_PED_BEING_JACKED(ped)||AI::IS_PED_GETTING_UP(ped)||
  PED::IS_PED_USING_ANY_SCENARIO(ped);
 // Never gate recovery on SCRIPT_TASK_ANY. Normal player-control tasks and
 // an exiting cosmetic task may remain active after the phone is put away.
 // Use concrete interactions above; the scheduler also backs off on interruption.
 bool ready=STREAMING::HAS_ANIM_DICT_LOADED((char*)a.dict);
 if(!nativeAction&&!ready&&now>=animRetryAt){STREAMING::REQUEST_ANIM_DICT((char*)a.dict);animRetryAt=now+1000;}
 if(now>=nextSpeechCheck){speaking=AUDIO::IS_ANY_SPEECH_PLAYING(ped)||hurtAudio.active(now);nextSpeechCheck=now+120;}
 auto desired=now<blinkUntil?CosmeticPolicy::Mode::Closed:speaking?CosmeticPolicy::Mode::Talk:CosmeticPolicy::Mode::Idle;
 auto decision=cosmeticPolicy.step(now,nativeAction,ownPlaying,desired,ready);
 if(decision==CosmeticPolicy::Action::Stop){
  // Stop ONLY our named clip; clearing the secondary task would erase gameplay.
  AI::STOP_ANIM_TASK(ped,(char*)a.dict,ownClip,-4.0f);
  primeCosmeticClock=false;
 }else if(decision==CosmeticPolicy::Action::Play){
  char clip[64];sprintf_s(clip,"%s_%s",a.prefix,cosmeticSuffix(desired));
  constexpr int flags=1|32|8192|131072|16777216; // loop, secondary, exit on interruption, force fresh start, abort on weapon damage
  AI::TASK_PLAY_ANIM(ped,(char*)a.dict,clip,8,-8,-1,flags,1.0f,false,false,false);
  primeCosmeticClock=true;
 }
 // Optional local QA only. Read once from INI; production defaults to off.
 // Bounded one-Hz state/clock trace, never a per-frame disk log.
 if(animationTrace&&!animationTraceEnd)animationTraceEnd=now+300000;
 if(animationTrace&&static_cast<LONG>(now-animationTraceEnd)>=0)animationTrace=false;
 if(animationTrace&&now>=nextAnimationTrace){
  nextAnimationTrace=now+1000;unsigned mask=0;
  if(!playable()||GAMEPLAY::GET_MISSION_FLAG())mask|=1;
  if(actionInput)mask|=2;
  if(PED::IS_PED_RUNNING_MOBILE_PHONE_TASK(ped))mask|=4;
  if(AUDIO::IS_MOBILE_PHONE_CALL_ONGOING())mask|=8;
  if(AI::IS_PLAYING_PHONE_GESTURE_ANIM(ped))mask|=16;
  if(PED::IS_PED_RELOADING(ped)||PED::IS_PED_SHOOTING(ped)||PLAYER::IS_PLAYER_FREE_AIMING(PLAYER::PLAYER_ID())||PLAYER::IS_PLAYER_TARGETTING_ANYTHING(PLAYER::PLAYER_ID())||PED::IS_PED_IN_MELEE_COMBAT(ped)||PED::IS_PED_PLANTING_BOMB(ped))mask|=32;
  if(PED::IS_PED_IN_COVER(ped,false)||PED::IS_PED_GOING_INTO_COVER(ped))mask|=64;
  if(PED::IS_PED_CLIMBING(ped)||PED::IS_PED_VAULTING(ped)||PED::IS_PED_JUMPING(ped)||PED::IS_PED_FALLING(ped)||PED::IS_PED_DIVING(ped)||PED::IS_PED_SWIMMING(ped)||PED::IS_PED_IN_PARACHUTE_FREE_FALL(ped)||(parachuteState>=1&&parachuteState<=3))mask|=128;
  if(PED::IS_PED_GETTING_INTO_A_VEHICLE(ped)||PED::IS_PED_JUMPING_OUT_OF_VEHICLE(ped)||PED::IS_PED_HANGING_ON_TO_VEHICLE(ped)||PED::IS_PED_JACKING(ped)||PED::IS_PED_BEING_JACKED(ped))mask|=256;
  if(PED::IS_PED_USING_ANY_SCENARIO(ped)||AI::IS_PED_GETTING_UP(ped))mask|=512;
  if(PED::IS_PED_RAGDOLL(ped)||PED::IS_PED_INJURED(ped))mask|=1024;
  int mode=(int)cosmeticPolicy.current();
  {
   FILE* log=nullptr;fopen_s(&log,animationTracePath.c_str(),"a");
   if(log){
    float phase=ownPlaying?ENTITY::GET_ENTITY_ANIM_CURRENT_TIME(ped,(char*)a.dict,ownClip):-1.0f;
    auto tip=PED::GET_PED_BONE_COORDS(ped,63009,0,0,0);auto localTip=ENTITY::GET_OFFSET_FROM_ENTITY_GIVEN_WORLD_COORDS(ped,tip.x,tip.y,tip.z);
    fprintf(log,"tick=%lu blockMask=%u busy=%d mode=%d ownPlaying=%d dictionary=%d parachute=%d anyTask=%d decision=%d phase=%.5f tail=%.5f,%.5f,%.5f\n",now,mask,nativeAction,mode,ownPlaying,ready,parachuteState,AI::GET_SCRIPT_TASK_STATUS(ped,0x55966344),(int)decision,phase,localTip.x,localTip.y,localTip.z);fclose(log);
   }
  }
 }
}
static void scriptMain(){
 for(auto& a:avatars)a.hash=GAMEPLAY::GET_HASH_KEY((char*)a.model);
 menuKey=GetPrivateProfileIntA("Keys","Menu",VK_F5,iniPath.c_str());restoreKey=GetPrivateProfileIntA("Keys","Restore",VK_F6,iniPath.c_str());blinkKey=GetPrivateProfileIntA("Keys","Blink",VK_F7,iniPath.c_str());
 animationTrace=GetPrivateProfileIntA("Debug","AnimationTrace",0,iniPath.c_str())!=0;
 autoReturnAvatar=GetPrivateProfileIntA("Respawn","ReturnToAvatar",1,iniPath.c_str())!=0;
 showAvatarDeath=GetPrivateProfileIntA("Respawn","ShowAvatarDeath",1,iniPath.c_str())!=0;
 hurtAudio.configure(hurtSoundPath,GetPrivateProfileIntA("Audio","HurtSounds",1,iniPath.c_str())!=0,GetPrivateProfileIntA("Debug","AudioTrace",0,iniPath.c_str())!=0);
 if(menuKey<32||menuKey>254)menuKey=VK_F5;
 if(restoreKey<32||restoreKey>254||restoreKey==menuKey){restoreKey=VK_F6;while(restoreKey==menuKey)restoreKey++;}
 if(blinkKey<32||blinkKey>254||blinkKey==menuKey||blinkKey==restoreKey){blinkKey=VK_F7;while(blinkKey==menuKey||blinkKey==restoreKey)blinkKey++;}
 // Build the inventory catalog once, after gameplay is available. No file
 // polling, world ped scans, disk logging, or background model preload.
 // Handle death/loading even before player control becomes available.
 while(!playable()){LONG startupKeys=InterlockedExchange(&inputBits,0);recoverOriginalIfNeeded((startupKeys&2)!=0);WAIT(0);}
 buildCatalog();if(avatarIndex(ENTITY::GET_ENTITY_MODEL(PLAYER::PLAYER_PED_ID()))>=0)loadOriginal();
 for(;;){
  LONG keys=InterlockedExchange(&inputBits,0);
  if(!offline()){InterlockedExchange(&menuVisible,0);WAIT(250);continue;}
  if(keys&3)releaseFilmingCameraForMenu();
  {Ped p=PLAYER::PLAYER_PED_ID();if(ENTITY::DOES_ENTITY_EXIST(p))hurtAudio.update(p,avatarIndex(ENTITY::GET_ENTITY_MODEL(p)));}
  if(recoverOriginalIfNeeded((keys&2)!=0)){WAIT(0);continue;}
  // Native switch actions also participate in scripted character selection.
  // Reserve the F-key actions only where our avatar menu is available; missions
  // (including the prologue tutorial) retain their normal switch controls.
  if(freeRoam())for(int f=VK_F5;f<=VK_F8;f++)if(menuKey==f||restoreKey==f||blinkKey==f)CONTROLS::DISABLE_CONTROL_ACTION(0,166+(f-VK_F5),true);
  if(keys&1){if(freeRoam())InterlockedExchange(&menuVisible,!InterlockedCompareExchange(&menuVisible,0,0));else notice("CU Avatars is for singleplayer free roam, outside missions.");}
  if(keys&2){InterlockedExchange(&menuVisible,0);changeModel(3);}
  if(keys&4)blinkUntil=GetTickCount()+3000;
  if(InterlockedCompareExchange(&menuVisible,0,0)){
   if(!freeRoam())InterlockedExchange(&menuVisible,0);
   else {if(keys&8)selection=(selection+3)%4;if(keys&16)selection=(selection+1)%4;if(keys&64)InterlockedExchange(&menuVisible,0);else if(keys&32){InterlockedExchange(&menuVisible,0);changeModel(selection);}else drawMenu();}
  }
  Ped ped=PLAYER::PLAYER_PED_ID();if(ENTITY::DOES_ENTITY_EXIST(ped)){
   int index=avatarIndex(ENTITY::GET_ENTITY_MODEL(ped));
   if(index>=0)animate(ped,index);else if(trackedPed&&trackedPed!=ped){trackedPed=0;cosmeticPolicy.reset();primeCosmeticClock=false;haveOriginal=false;WritePrivateProfileStringA("Restore","Pending","0",statePath.c_str());}
  }
  WAIT(0);
 }
}
BOOL APIENTRY DllMain(HMODULE module,DWORD reason,LPVOID){
 if(reason==DLL_PROCESS_ATTACH){char path[MAX_PATH];GetModuleFileNameA(module,path,MAX_PATH);auto stem=std::string(path).substr(0,std::string(path).find_last_of('.'));iniPath=stem+".ini";statePath=stem+".state.ini";hurtSoundPath=stem.substr(0,stem.find_last_of("\\/"))+"\\CUAvatars\\sounds\\";recoveryLogPath=stem+".recovery.log";animationTracePath=stem+".animation-debug.log";DisableThreadLibraryCalls(module);keyboardHandlerRegister(keyboard);scriptRegister(module,scriptMain);}
 else if(reason==DLL_PROCESS_DETACH){keyboardHandlerUnregister(keyboard);scriptUnregister(module);}
 return TRUE;
}
