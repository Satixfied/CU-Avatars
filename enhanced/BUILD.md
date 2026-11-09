# Enhanced port

Target: additive CU Avatars for GTA V Enhanced.
The Legacy source and known-good binaries remain authoritative for shared gameplay.
No private actor replacement / mission archives are ported by this pipeline.

## Build

1. Use an x64 MSVC developer prompt and Script Hook V SDK. Run
   `enhanced\build.cmd "path\to\sdk"`. The avatar output goes to enhanced/build;
   neither existing Legacy binary is overwritten. RuntimeEntry.cpp compiles
   the shared implementation with an Enhanced-only executable gate.
2. Use .NET 8 to build enhanced/Converter/Converter.csproj, with
   `-p:CodeWalkerCoreProject=absolute/path/to/CodeWalker.Core.csproj`.
   CodeWalker must include Gen9Converter and ShadersGen9Conversion.xml.
   The existing project dependency contains these; shader conversion data must
   accompany CodeWalker.Core.dll (its project copies this automatically).
3. Build AssetTool/AssetTool.csproj with the same property. Its pack/audit
   commands create OPEN archives and verify extracted bytes against inputs.
4. Input ped-files contains the 15 accepted Legacy addon resources; dlc-tree
   contains setup2.xml, content.xml and common/data/peds.meta. These are generated
   by the parent asset pipeline. A recipient of the source ZIP can also extract
   these inputs from the Legacy mod's DLC using CodeWalker. Do not substitute
   private scene rigs or raw author avatar files.
5. The packaging script expects the parent's sounds/, release credit/license
   texts, plus compiled avatar policy test executables. These assets/docs come
   from the Legacy package. Compile root Test-*.cpp using C++17 into build/. They have no game dependency.
6. Run `python enhanced/package.py` (Python 3.11+). This converts all 9 graphical
   resources, preserves the 6 shared-format metadata/animation files, checks
   skeletons/geometry, packs/readbacks the DLC, runs four avatar policy tests, and
   writes preview and source ZIPs with SHA256 manifests. Adjust ROOT/OUT only
   if building outside the original work/cu-avatars workspace layout.

The source ZIP contains our code, not third-party SDK/CodeWalker binaries or
raw avatar author projects. Keep creator credit/asset terms from the runtime ZIP.

## Evidence and limits

conversion-report.json records every input/output hash. verification.json records
offline checks and unchanged Legacy hashes. All nine Gen9 headers and loaded
resources are checked. No game executable, loader, encrypted original archive,
private permission message or raw author asset is bundled.

Enhanced 1158.16 is now installed locally. On 9 November 2026 the first load
failed with MetaDataStore full (3200/3200); raising only that setting in Enhanced's
own mods gameconfig to 4000 allowed the prologue to load. The user then confirmed
native prologue switching works after limiting our F-key action suppression to
free roam. Both public editions include this shared source fix in 0.1.1.
Avatar selection, visuals, phone/vehicle/weapon interactions and respawn remain
pending in Enhanced. The public guide now requires the targeted metadata edit.
No original gameconfig is redistributed. Local Converter install-dlclist and
install-metadata commands back up an existing mods update.rpf and verify readback;
these local developer commands are not a replacement for the public guide.

References checked 2026-10-30:
- https://www.dev-c.com/gtav/scripthookv/
- https://raw.githubusercontent.com/Sollumz/wiki/main/tutorials/asset-conversion-for-gtav-enhanced.md
- https://www.gta5-mods.com/tools/openrpf-openiv-asi-for-gta-v-enhanced
- https://docs.fivem.net/docs/alchemist/

CodeWalker conversion is used locally; Alchemist was researched but is not a
dependency. No online service submission or telemetry was needed for conversion.
