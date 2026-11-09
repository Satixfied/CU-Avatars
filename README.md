# CU Avatars — Expie, Milky and Dune

Source for the **0.1.1 Legacy alpha and Enhanced preview** singleplayer mod by Satixfied. Published for source review, including Nexus Mods review of the compiled `CUAvatars.asi` plugin.

The mod lets players select an additive avatar without replacing the original character assets. The public packages do not include the private filming presets.

## Build the ASI (both editions)

1. Install Visual Studio 2022 Build Tools with **Desktop development with C++**, the MSVC x64 compiler and a Windows SDK.
2. Download the **Script Hook V SDK** separately from [Alexander Blade's official page](https://www.dev-c.com/gtav/scripthookv/). Extract it into a folder containing `inc/main.h` and `lib/ScriptHookV.lib`.
3. Download this repository using **Code → Download ZIP**, extract it, and open **x64 Native Tools Command Prompt for VS 2022**.
4. Change into the extracted repository folder, for example `cd /d "D:\CU-Avatars"`.
5. Build Legacy:

```bat
build.cmd "D:\Dependencies\ScriptHookV_SDK"
```

Output: `build/CUAvatars.asi`.

6. Build Enhanced:

```bat
enhanced\build.cmd "D:\Dependencies\ScriptHookV_SDK"
```

Output: `enhanced/build/CUAvatars.asi`.

Replace the example SDK path with your own. Both builds compile the same C++17 implementation, statically link the MSVC runtime (`/MT`), and link ScriptHookV, user32 and winmm. Enhanced's entry point additionally checks for `GTA5_Enhanced.exe`. Artwork, game files, Python, .NET and CodeWalker are **not required to compile either ASI**. Rebuilds can differ in PE timestamps and toolchain output; release hashes identify the uploaded artifacts rather than promising byte-identical builds.

## Run the standalone regression checks

In the same x64 developer prompt:

```bat
test.cmd
```

These four tests cover cosmetic-task arbitration, hurt-reaction timing, manual switching and respawn delay. They do not require GTA. They do not replace in-game testing.

## Review map

- `CUAvatars.cpp`: plugin lifecycle, menu, avatar swaps, native gameplay integration and recovery.
- `CosmeticPolicy.h`: yielding blink/tail animation to gameplay tasks, including the phone.
- `RespawnDelay.h`: death/respawn handoff timing.
- `HurtAudio.h` and `HurtReactionPolicy.h`: local WAV playback and reaction policy.
- `ManualSwitchStamp.h`, `WeaponCatalog.h`: switch coordination and weapon preservation.
- `enhanced/RuntimeEntry.cpp`: Enhanced executable gate.
- `CUAvatars.ini`: distributed configuration.
- `AssetTool/`, `enhanced/Converter/`: optional C# archive/conversion tools; require .NET 8 and a separately obtained CodeWalker source project.
- `package.py`, `enhanced/package.py`, installer XML and `DLC/`: release packaging source and metadata templates.
- `docs/` and `enhanced/INSTALLATION GUIDE.txt`: user installation guides.
- `RELEASE-HASHES.json`: hashes of the original 0.1.1 distribution archives and ASIs.

The packaging scripts are preserved release-workspace tools, not a one-command build of the full art-containing download from this repository. They also require the permitted converted models, sounds, external tooling, and the documented workspace layout. The ASI build instructions above are standalone.

## Version and testing status

0.1.1 limits F-key native-action suppression to free roam so Story Mode's native character switching remains available. Enhanced startup and prologue switching were confirmed on game version 1158.16 after increasing the metadata pool as described in the guide. Enhanced avatar visuals and broader interaction tests remain pending; it is a preview. Legacy avatar interaction testing predates the shared 0.1.1 mission-switch fix; that fix has not had a separate Legacy live regression test.

## Licensing and credits

Original plugin and packaging code: MIT, see [Code license.txt](Code%20license.txt). This does not license GTA, Script Hook V, CodeWalker, avatar artwork or audio. No original avatar projects, model/texture binaries, sounds, game files, SDK libraries, credentials or private permission messages are included here.

Models used with permission: **Akittu** ([Expie](https://akittu.gumroad.com/l/stylizedexperiment), [Milky](https://akittu.gumroad.com/l/stylizedmilky), [X](https://x.com/_akittu), [YouTube](https://www.youtube.com/@akittu)) and **Sevenscalesdragon** ([Dune](https://sevenscalesdragon.gumroad.com/l/SSDstylizedDune), [X](https://x.com/Sevenscales_DOU)). See [Credits.txt](Credits.txt) and [Asset permissions.txt](Asset%20permissions.txt) for full attribution and separate asset terms.

Concept, creative direction and in-game testing by Satixfied. AI tools assisted with code, character rigging and animation adaptation. GTA V belongs to Rockstar Games; this is an unofficial singleplayer fan mod.
