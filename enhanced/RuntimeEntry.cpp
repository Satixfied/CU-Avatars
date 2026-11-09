// Keep the proven gameplay implementation shared with Legacy. The Enhanced
// release has an edition gate so its Gen9 assets cannot be used accidentally
// by a Legacy runtime. ScriptHookV owns native hash translation for each build.
#define DllMain SharedDllMain
#include "../CUAvatars.cpp"
#undef DllMain

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved) {
 if(reason==DLL_PROCESS_ATTACH) {
  wchar_t path[MAX_PATH]{};
  if(!GetModuleFileNameW(nullptr,path,MAX_PATH))return FALSE;
  const wchar_t* base=wcsrchr(path,L'\\');base=base?base+1:path;
  if(_wcsicmp(base,L"GTA5_Enhanced.exe")!=0) {
   OutputDebugStringA("CU Avatars Enhanced: refusing non-Enhanced executable. Use the Legacy package for GTA5.exe.\n");
   return FALSE;
  }
 }
 return SharedDllMain(module,reason,reserved);
}
