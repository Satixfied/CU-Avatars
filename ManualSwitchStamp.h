#pragma once
#include <atomic>
#include <cstdint>

// A bounded, process-local confirmation of an intentional F5/F6 model switch.
// Death/arrest recovery and automatic respawn never publish this confirmation.
class ManualSwitchStamp {
 std::atomic<std::uint32_t> revision{0},born{0};
 std::atomic<std::uint64_t> handles{0};
public:
 void publish(int previous,int current,std::uint32_t now){
  revision.fetch_add(1,std::memory_order_acq_rel);
  handles.store((std::uint64_t(std::uint32_t(previous))<<32)|std::uint32_t(current),std::memory_order_relaxed);
  born.store(now,std::memory_order_relaxed);
  revision.fetch_add(1,std::memory_order_release);
 }
 std::uint32_t query(int previous,int current,std::uint32_t now)const{
  auto a=revision.load(std::memory_order_acquire);if(!a||(a&1))return 0;
  auto pair=handles.load(std::memory_order_relaxed);auto time=born.load(std::memory_order_relaxed);
  std::atomic_thread_fence(std::memory_order_acquire);
  if(a!=revision.load(std::memory_order_relaxed)||std::uint32_t(now-time)>3000)return 0;
  auto wanted=(std::uint64_t(std::uint32_t(previous))<<32)|std::uint32_t(current);
  return pair==wanted?a:0;
 }
};
