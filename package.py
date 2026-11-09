"""Create the free, game-ready alpha and source-code ZIPs. Does not install/upload."""
import hashlib,json,shutil,uuid,zipfile,xml.etree.ElementTree as E
from pathlib import Path
W=Path(__file__).resolve().parent;O=W.parent.parent/'outputs';B=W/'release';B.mkdir(exist_ok=True)
VERSION='0.1.1-alpha';NAME='CU Avatars'
CREDITS='''CREDITS

Concept, creative direction and testing: Satixfied.
Code and GTA rig/animation adaptation assisted by AI tools.
Hurt reaction sound clips supplied by Satixfied.

Much love to Akittu for giving permission to use their gorgeous models for
these mods and videos! Check out their work and show them some love:
Expie: https://akittu.gumroad.com/l/stylizedexperiment
Milky: https://akittu.gumroad.com/l/stylizedmilky
Twitter/X: https://x.com/_akittu
YouTube: https://www.youtube.com/@akittu

Much love to Sevenscalesdragon for allowing use of their beautiful Dune model!
Please go support them:
Dune: https://sevenscalesdragon.gumroad.com/l/SSDstylizedDune
Twitter/X: https://x.com/Sevenscales_DOU

Casualties: Unknown characters/setting: Orsoniks.
Grand Theft Auto V: Rockstar Games. This is an unofficial fan modification.
Runtime dependency: Script Hook V by Alexander Blade (download separately).
Model installation: OpenIV by the OpenIV Team (download separately).
Asset build tooling uses CodeWalker by dexyfex as an external dependency.

Retain these credits and links on the mod download page and associated videos.
Private permission messages/screenshots are not included in the public download.
'''
TERMS='''ASSET PERMISSIONS AND DISTRIBUTION

This is a free GTA V singleplayer mod. Do not sell this package or the models,
put the creators' work behind a paywall, or offer the models as a paid feature.

Akittu permitted Satixfied to use the free downloadable avatars in games/mods
and media, with Gumroad/Jinxxy product links and Twitter/X and YouTube credits.
Do not redistribute the original avatar files as supplied by the creator.
No purchased supporter-pack source files, original Blender files, FBX avatar
exports, Unity packages or Substance Painter project files are included here.

Sevenscalesdragon permitted a downloadable mod using Dune, with creator links.
Their stated restriction is no political use. Respect this condition.

The avatar artwork is NOT licensed under the source-code license. The files
in the custom DLC are converted GTA-specific runtime assets for this mod.
Permission for this mod does not grant a general license to repackage or resell
the creators' avatar products. Direct people to the original creators instead.
The supplied hurt sound clips are also separate from the code license and are
included as runtime assets for this mod.

These creator permissions do not constitute endorsement or permission from
Rockstar/Take-Two or a hosting platform. Their separate terms still apply.
No claim of blanket legal clearance or CurseForge approval is made.
'''
README='''CU AVATARS - SINGLEPLAYER
Version 0.1.1-alpha | Windows x64 | GTA V Legacy PC

Turn your player into Expie, Milky or Dune in offline free roam. All three use
unique add-on model names. No original NPC, protagonist, mission, cutscene or
spawn-list entry is replaced. Nothing changes until you choose an avatar.

CHANGES IN 0.1.1
Native character-switch controls remain available during story missions.
The prologue switching fix was confirmed in Enhanced; the shared fix is included
in Legacy too. Avatar menu shortcuts are reserved only in free roam.

STATUS
This new add-on package is an alpha. A local Legacy3889 test with the separate
roaming profile now loads Story Mode and shows the F5 avatar menu (user confirmed).
The first startup crashed with the metadata store full. Raising that one capacity
to at least 4000 resolved the startup failure in this test. The installer includes
that targeted change. Local mounting used RageOpenV, an OpenIV-compatible loader;
the OpenIV installation path, model selections, inventory/appearance restoration
and broad vehicle/weapon visual checks still need live verification.
Binary asset readback, custom-animation checks, packaging/installer XML checks
and native x64 compilation passed. Do not describe this build as fully tested.
The roaming fit aligns native hand/prop/foot/toe frames to the protagonists.
Offline checks passed across 54 animation clips for each avatar. These checks
measure skeleton anchors, not visible finger wrapping or clearance in vehicles.
Vehicle/weapon coverage and acceptance criteria are in Roaming test checklist.txt.

REQUIREMENTS - install separately from their authors
1. A legitimate GTA V Legacy PC installation. Enhanced is not supported/tested.
2. Script Hook V and an ASI loader, matching your game version:
   https://www.dev-c.com/gtav/scripthookv/
3. OpenIV with its ASI Manager's OpenIV.asi and a working mods folder:
   https://openiv.com/
   Set up/copy update/update.rpf into OpenIV's mods folder when prompted.
4. Story Mode with BattlEye disabled using the official launcher setting.
   This package does not include Script Hook V, an ASI loader or OpenIV.asi.

INSTALL
Close GTA. Back up any existing mods-folder update.rpf before changing it.
1. Extract this ZIP to a normal folder.
2. Copy the two files AND the sound folder from "Game folder" beside GTA5.exe:
      CUAvatars.asi
      CUAvatars.ini
      CUAvatars/ (contains sounds/ and six WAV clips)
3. Open "CU Avatars - Models.oiv" through OpenIV's Package Installer.
   Choose INSTALL INTO THE MODS FOLDER, not installation into original game files.
4. Start Story Mode. Press F5 and choose an avatar.

The OIV adds mods/update/x64/dlcpacks/cuavatars/dlc.rpf and its one entry
in the mods copy of update/update.rpf/common/data/dlclist.xml:
    <Item>dlcpacks:/cuavatars/</Item>
It preserves other DLC-list entries and removes duplicate entries for this pack.
It also raises only MetaDataStore in mods/update/update.rpf/common/data/gameconfig.xml
to at least 4000. Existing larger values and all other settings are preserved.
This reserves room for add-on metadata on builds whose stock pool is already full.
For manual installation, an .oiv is a ZIP: open it with an archive manager,
copy content/cuavatars into mods/update/x64/dlcpacks, and add the line above
using OpenIV. Do not copy this custom pack into an original character archive.
For manual installation, also ensure MetaDataStore is at least 4000 in the mods
copy of gameconfig.xml; keep a backup and never lower an existing larger value.

CONTROLS
F5: open/close avatar menu. Up/Down: choose. Enter: select. Backspace/Esc: close.
F6: restore the original player model, outfit and props captured on first switch.
F7: hold avatar eyelids closed for three seconds.
Death: keep the avatar through WASTED, use a native protagonist once GTA naturally
fades black, then return to the last avatar after the hospital flow. Missing native
fade has a15-second safety bound; the mod never forces its own death fade.
Arrest also uses native recovery and returns to the last avatar afterward.
Respawn/ShowAvatarDeath=0 and ReturnToAvatar=0 restore the simpler manual-selection
behavior. Full-avatar WASTED, native hospital return and moving-MTB avatar
switching passed local user tests. Broader all-model/arrest testing remains pending.
F6 also attempts original-character recovery during a stuck loading transition.
Change Windows virtual-key numbers in CUAvatars.ini, then restart GTA.
Health, armour, common/DLC weapon inventory and the vehicle seat are preserved
by the switcher. This logic needs live regression testing with your loadout.
Bike/bicycle/quad switches briefly pause and detach the old rider before replacing
the model, then reseat the new rider and restore motion to avoid stale cycle tasks.
Close the menu before driving or firing. Switch while standing safely or seated;
switching models interrupts ongoing tasks, so avoid switching during a stunt.
Original appearance is cached on an avatar switch in CUAvatars.state.ini so
restoration remains available if you reload a save while using an avatar.

FREE ROAM ONLY
Restore your normal protagonist with F6 BEFORE saving or starting any story
mission, cutscene or replay. Save with Michael, Franklin or Trevor, then choose
an avatar again after loading. Switching is blocked during missions/cutscenes. The mod
does not alter original mission casting or supply authored cutscene expressions.
Eyes blink regularly; the mouth uses a simple loop when GTA reports speech.
The tail follows an authored curved sway, without a physics simulation.
It is not an exact reproduction of VRChat's expression/physics systems.
HURT AUDIO
Four barks cover lighter hits; two whines cover heavier/fatal hits. Variations
and a cooldown prevent stacking. Whines are balanced down to bark loudness; the
supplied originals were kept untouched. Armour damage also triggers reactions. Human
breathing/exertion uses GTA's normal voice audio; pain is muted only during the
custom hurt clip (or while dead), not permanently. Audio/HurtSounds=0 disables
custom hurt reactions. Keep the supplied CUAvatars/sounds folder beside the ASI.
Custom WAV playback uses the Windows default audio output and GTA's app session;
GTA's internal voice-volume slider may not control these clips.
Cosmetic face/tail loops yield to phone use, weapon actions, cover, climbing,
swimming/parachuting, vehicle transitions and scenarios.
They resume after a quiet interval. F7 and speech do not bypass that protection.
This prevents cosmetics repeatedly replacing GTA's phone/upper-body animations.
The revised interruption policy passed offline regression tests; live interaction
verification remains required for this alpha.
Phone-safe pause and recovery after put-away were confirmed in the local test.
Known limitation: blinking and tail sway pause while holding the phone. They
return after it is put away. A concurrent facial-layer test had no visual effect,
so the download retains the working phone-safe pause/resume behavior.

COMPATIBILITY
Do not load this alongside the older private Expie.asi/FatherSon scene builds:
those packages replace slots and use overlapping keys. Other player-switcher
or clothing mods can also conflict. Change keys if another mod uses F5/F6/F7.
Do not use GTA Online or multiplayer platforms with this ASI.

UNINSTALL
Restore the normal protagonist first, save if desired, then close GTA.
Use "CU Avatars - Remove Models.oiv" in OpenIV, targeting the MODS folder.
Remove CUAvatars.asi, CUAvatars.ini and the supplied CUAvatars sound folder from the game root. The optional generated
CUAvatars.state.ini can also be removed. Other game files/mods are not targeted.
Manual alternative: remove the cuavatars DLC entry and its custom folder only.
Uninstall leaves metadata capacity intact so other add-ons can continue using it.
Restore your backed-up gameconfig.xml only if you also intend to remove its other
changes. The installer never replaces the complete gameconfig.xml.

PERFORMANCE
No world ped scans, periodic configuration polling or background model loading.
Debug tracing defaults off. Optional AnimationTrace=1 records state and animation
clock once per second for five minutes after selecting an avatar, then stops.
Rare death/arrest recovery events are recorded in CUAvatars.recovery.log.
Assets load when selected. Settings do not change GTA/OBS graphics.
If recording, use an actual frame cap/VSync and leave GPU headroom for OBS.

PUBLICATION
Keep this download free. Include Credits.txt links in the description and
respect Asset permissions.txt. No original avatar editor/source files or private
permission screenshots are bundled. A code-only source ZIP is provided separately.
See Publishing notes.txt for the hosting status and pre-release live-test list.
'''
PUBLISH='''PUBLISHING NOTES -26October2026

Proposed title: CU Avatars - Expie, Milky and Dune
Summary: Optional Expie, Milky and Dune add-on player models for GTA V singleplayer.

Release type: ALPHA. Local Legacy3889 Story Mode startup and the F5 menu were
confirmed by the user. Complete the remaining tests before marking stable.
Local Legacy 3889 startup failed with MetaDataStore full (3200/3200), before the
avatar script's main function ran. A targeted minimum-4000 pool edit is now included.
The corrected startup and F5 menu work. Model-selection, restoration and visual
interaction coverage still need confirmation. The local loader was RageOpenV;
this does not verify the OpenIV backend or the OIV installer's actual execution.

CURSEFORGE
GTA V is absent from the public game catalog checked on26October2026:
https://www.curseforge.com/games
CurseForge's submission guide says to request a game through support if missing:
https://support.curseforge.com/support/solutions/articles/9000199552-project-submission-guide-and-tips
Do not upload a GTA V package under Minecraft or another unrelated game.
Game inclusion and project acceptance are decisions for CurseForge support.
This package has not been uploaded or submitted anywhere.

Other GTA-specific hosting options can be investigated, including GTA5-Mods.com
and Nexus Mods' GTA V category. Follow each host's moderation and asset policies.
Model-creator permission does not guarantee a hosting site's approval or blanket
permission under Rockstar's separate mod guidelines:
https://www.rockstargames.com/community-resources/mod-guidelines

REQUIRED LIVE CHECKS
Clean Legacy install with no private replacement/scene packages.
OIV install to mods folder; all three unique model IDs registered.
F5 menu, select all3, regular blinks, F7 held blink, tail sway, speech loop.
Complete Roaming test checklist.txt for EACH of the three avatars. Anchor math
has passed; visible grip, seat and clearance checks are still pending in-game.
F6 restores each protagonist's outfit/props, health/armour and weapon loadout.
Death/arrest temporarily uses a protagonist for native recovery, then returns
to the last avatar. Verify the visible ragdoll, fade and automatic reapplication
on all three. Local tests confirmed both immediate native recovery and the new visible
death/automatic avatar return. Repeat across all avatars and arrest situations.
Light/heavy hits play supplied bark/whine sounds, without stacking or false
triggers on health regeneration/model changes. Human exertion/breathing remains
audible between reactions. Test Windows default output and OBS audio capture.
Save/reload after restoring the protagonist; then select an avatar again.
Emergency restoration from an old transformed save is not a supported save flow.
Mission/cutscene/online guards; normal missions after restoring original player.
Key rebinding and another mod/DLC present; OIV reinstall/uninstall preserves them.

Author/creator credit block: copy Credits.txt, including the model and social links.
No price tag, paid model tier, original source-avatar redistribution, or political
use of Dune. Artist permissions are specific; do not replace them with a generic
"all assets MIT" license. Screenshots of private permissions remain private.
'''
CODELICENSE='''MIT License - original switcher and packaging code only

Copyright(c)2026 Satixfied

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

This code license does not cover avatar artwork, converted model/texture assets,
supplied sound clips,
Script Hook V, its SDK, OpenIV, CodeWalker, GTA V or any other third-party work.
The complete art-containing mod download must remain free under its model terms.
'''
CHECKLIST='''ROAMING VISUAL TESTS - CU AVATARS 0.1.1-alpha

LIVE STATUS: ALL ROWS PENDING. Repeat every row for Expie, Milky AND Dune.
Do not mark a row passed solely because the offline skeleton check passed.
Record game build, other mods, avatar, vehicle/weapon model and camera view for
any issue. Start on a clean Legacy mod profile without the private movie mods.

1. Ground movement: stand, walk, run, sprint, stop, turn, stairs and slopes.
   Feet should plant correctly; no stretching, body collapse or stuck poses.
2. Jump, vault, climb, ladders, falling/ragdoll and getting back up.
   Hands/feet should follow the contact surface; custom loops must not freeze body.
3. Cars: compact (Panto), sedan (Tailgater), sports (Comet), supercar (Adder),
   convertible (Sentinel), SUV (Granger), low car (Banshee), van (Speedo),
   pickup (Bison), truck (Phantom). Enter/exit both sides, steer fully both ways,
   reverse, driver/passenger/rear seats. Check hands, feet, seat and head clearance.
4. Motorcycles: sports (Bati 801), cruiser (Daemon), dirt (Sanchez), scooter
   (Faggio), quad (Blazer). Mount/dismount, idle, steer/lean, accelerate/brake.
   Hands should meet handlebars and feet meet pegs without twisted arms.
5. Bicycles: BMX, Scorcher and race bike (Tri-Cycles). Pedal standing/seated,
   steer, jump, mount/dismount; check bars and pedals throughout their motion.
6. Aircraft/watercraft: helicopter (Frogger), plane (Dodo), boat (Dinghy),
   jet ski (Seashark). Check controls, seating, entry/exit and camera clearance.
7. One-handed weapons: Pistol, Combat Pistol, Micro SMG and Stun Gun.
   Holster/draw, idle carry, aim, fire, reload; inspect right hand from both sides.
8. Two-handed weapons: SMG, Assault Rifle, Carbine Rifle, Pump Shotgun,
   Sniper Rifle and Combat MG. Test stock and extended/suppressed variants;
   carry, aim/fire, recoil, reload while standing/walking/crouched/in cover.
   Inspect right trigger hand AND left support hand, wrists and elbows.
9. Heavy weapons/tools: RPG, Grenade Launcher, Minigun, Jerry Can and extinguisher.
   Check distinct grip points, carrying, firing/pouring and switching weapons.
10. Melee/throwables: fists, Knife, Baseball Bat, Grenade and Sticky Bomb.
    Swing, block, strike and throw without limb distortion or stuck props.
11. Vehicle weapons: pistol/Micro SMG drive-by and passenger aiming in a car
    and on a motorcycle. Check hands when switching between controls and gun.
12. Swim on surface/underwater, dive, climb out; parachute equip/deploy/land.
    Check limbs/straps and whether the backpack intersects the body/tail.
13. Phone: draw, keep open for30seconds, navigate contacts/messages, make/end
    a call, put away. Repeat walking/seated and with all3avatars. The draw must
    not repeat, and F7/ambient speech must not interrupt it. After put-away and
    a short quiet interval, automatic blink/tail sway should resume.
    Repeat interrupts with reload, aim/fire, weapon swaps, cover, melee, vault,
    swim, parachute, entry/exit, ambient gestures and another scripted animation.
    Face must follow head; tail animation must not deform arms/spine/legs.
14. Repeat representative aim/driving in third person and first person.
    Large heads/muzzles/tails and custom hand shapes may need camera-specific
    fixes; first-person clearance has not been validated in this alpha.
15. F5 switch on foot/in driver and passenger seats. F6 restore all three
    protagonists with outfit, props, ammo, clips, tints/components and parachute.
    Restore protagonist before saving/reloading, then select an avatar again.
    Check death/respawn and key rebinding.

VISUAL PASS CRITERIA
No visible floating grips, severe hand/prop separation, inverted joints, twisted
limbs, broken feet, blocked camera, persistent facial errors or body animation
overrides during the tested action. Record smaller avatar/seat/tail clipping too;
an unusual body shape can need a targeted correction for an individual vehicle.
Do not advertise every vehicle/weapon as verified based on representative tests.

OFFLINE RESULTS
54 sampled native body animations; 58,400 hand/prop/foot/toe position comparisons
per avatar. Maximum anchor difference below 0.002 mm. These are native frame
alignment tests, not rendered measurements of fingers, props or car interiors.
Face/tail clips contain only custom bones; no native body animation tracks.
'''
QUICK='''CU AVATARS - QUICK INSTALL (GTA V LEGACY, WINDOWS)

This is an alpha for offline Story Mode. Enhanced is not supported/tested.
One-time requirements: Script Hook V + ASI loader, and OpenIV + OpenIV.asi.
These third-party tools must be downloaded separately from their authors.

1. CLOSE GTA. Find its folder (the one containing GTA5.exe). In Steam you can
   use the game's Installed Files > Browse button. Back up any existing mods
   copy of update/update.rpf before installing this or another add-on.

2. Download Script Hook V: https://www.dev-c.com/gtav/scripthookv/
   From its bin folder, copy ScriptHookV.dll and dinput8.dll beside GTA5.exe.
   You do not need NativeTrainer.asi for CU Avatars.

3. Download/set up OpenIV: https://openiv.com/
   Select GTA V for Windows and your game folder. In its ASI Manager, install
   OpenIV.asi; an ASI loader must also be installed. Use the mods folder.
   Open update/update.rpf in OpenIV and use Copy to mods folder if needed.

4. Extract the CU Avatars ZIP. In OpenIV's Tools > Package Installer, select
   CU Avatars - Models.oiv and choose INSTALL INTO THE MODS FOLDER.
   It installs the three model filesets, registers their DLC and raises the
   metadata capacity to at least4000. Larger existing limits are kept.
   There is no manual dlclist or gameconfig edit with this installer.

5. Copy CUAvatars.asi, CUAvatars.ini AND the CUAvatars folder (with sounds inside)
   from the ZIP's Game folder directory beside GTA5.exe. All three items go in
   the GAME ROOT, not inside mods or scripts.

6. Disable BattlEye in the Rockstar launcher's GTA settings. Start Story Mode.
   F5 opens the avatar menu; arrows/Enter select; F6 restores your character.
   F7 briefly closes the eyes. Do not use this mod in GTA Online.
   On death, the avatar stays through WASTED until GTA naturally fades black, then
   a native protagonist remains dead for the hospital flow. The last avatar returns
   automatically afterward. Full-avatar WASTED and hospital return passed a live test.
   The included barks/whines play for hurt; ordinary effort/breathing stays human.

Use F6 to restore Michael/Franklin/Trevor BEFORE SAVING or starting missions/replays.
Select an avatar again after loading. This package changes
only your selected free-roam avatar; it does not replace NPCs or the story cast.
Face/tail cosmetics yield during phone/weapon/body interactions, then resume.

If F5 does nothing, check the two runtime files are beside GTA5.exe, Script Hook V
matches your game build, and no other mod owns the same key. If the menu reports
a missing model, repeat the Models.oiv installation into the correct mods folder.
See Read me first.txt for details and Roaming test checklist.txt for alpha status.

UNINSTALL: restore your protagonist, close GTA, use Remove Models.oiv in OpenIV's
mods folder, then remove CUAvatars.asi/ini, its optional state.ini and the supplied
CUAvatars sound folder. The shared
metadata headroom remains in place to avoid breaking other add-ons.
'''
WEBSITE='''CU AVATARS - EXPIE, MILKY AND DUNE FOR GTA V

Become Expie, Milky or Dune in GTA V Legacy singleplayer free roam. Select an
avatar with F5 and restore your original character with F6. No original NPC or
story character slots are replaced.

Version: 0.1.1-alpha. Free Windows download. GTA V Legacy only; Enhanced is not
supported/tested. Requires Script Hook V, an ASI loader and OpenIV/OpenIV.asi.
The download includes a model installer/uninstaller and an installation guide.

Custom blinking and tail sway pause during phone use and other protected
interactions, then resume. Broad vehicle/weapon compatibility testing is still
ongoing; this is an alpha release. Restore the protagonist before story missions.

Concept, creative direction and in-game testing by Satixfied. AI tools assisted
with the mod's code, character rigging and animation adaptation.

CREATOR CREDITS - include these on the website download page, not only in ZIP:
'''+CREDITS+'''\nRead Asset permissions.txt before redistributing. Keep the download free.
'''
QUICK=(W/'docs/Legacy installation guide.txt').read_text(encoding='utf-8')
for name,body in [('Read me first.txt',README),('INSTALLATION GUIDE.txt',QUICK),('Quick install.txt',QUICK),('Website description and credits.txt',WEBSITE),('Credits.txt',CREDITS),('Asset permissions.txt',TERMS),('Publishing notes.txt',PUBLISH),('Code license.txt',CODELICENSE),('Roaming test checklist.txt',CHECKLIST)]:
 (B/name).write_text(body,encoding='utf-8')
game=B/'Game folder';game.mkdir(exist_ok=True);shutil.copy2(W/'build/CUAvatars.asi',game/'CUAvatars.asi');shutil.copy2(W/'CUAvatars.ini',game/'CUAvatars.ini')
sounds=game/'CUAvatars/sounds';sounds.mkdir(parents=True,exist_ok=True)
for name in ['bark1.wav','bark2.wav','bark3.wav','bark4.wav','whine1.wav','whine2.wav']:shutil.copy2(W/'sounds'/name,sounds/name)

xpath="/SMandatoryPacksData/Paths/Item[translate(normalize-space(.), 'ABCDEFGHIJKLMNOPQRSTUVWXYZ\\', 'abcdefghijklmnopqrstuvwxyz/')='dlcpacks:/cuavatars/']"
def assembly(remove=False):
 root=E.Element('package',version='2.2',id='{'+str(uuid.uuid4()).upper()+'}',target='Five');m=E.SubElement(root,'metadata');E.SubElement(m,'name').text=NAME+(' - Remove Models' if remove else ' - Models');v=E.SubElement(m,'version');E.SubElement(v,'major').text='0';E.SubElement(v,'minor').text='1';E.SubElement(v,'tag').text='ALPHA 0.1.1';a=E.SubElement(m,'author');E.SubElement(a,'displayName').text='Satixfied';E.SubElement(m,'description').text='Install into the mods folder. '+('Remove only this custom DLC and its list entry.' if remove else 'Three optional unique add-on models. No original character slots are replaced. Copy the two runtime files and sound folder separately.');E.SubElement(m,'largeDescription').text=CREDITS;E.SubElement(m,'licence').text=TERMS
 colors=E.SubElement(root,'colors');E.SubElement(colors,'headerBackground',useBlackTextColor='False').text='$FF361A53';E.SubElement(colors,'iconBackground').text='$FF361A53';content=E.SubElement(root,'content')
 if remove:E.SubElement(content,'delete').text='update\\x64\\dlcpacks\\cuavatars\\dlc.rpf'
 else:E.SubElement(content,'add',source='cuavatars\\dlc.rpf').text='update\\x64\\dlcpacks\\cuavatars\\dlc.rpf'
 archive=E.SubElement(content,'archive',path='update\\update.rpf',createIfNotExist='False',type='RPF7');xml=E.SubElement(archive,'xml',path='common\\data\\dlclist.xml');E.SubElement(xml,'remove',xpath=xpath)
 if not remove:E.SubElement(E.SubElement(xml,'add',xpath='/SMandatoryPacksData/Paths',append='Last'),'Item').text='dlcpacks:/cuavatars/'
 if not remove:
  config=E.SubElement(archive,'xml',path='common\\data\\gameconfig.xml')
  patch=E.SubElement(config,'replace',xpath="/fwAllConfigs/ConfigArray/Item/Config[@type='CGameConfig']/PoolSizes/Entries/Item[PoolName='MetaDataStore']/PoolSize[number(@value) < 4000]")
  E.SubElement(patch,'PoolSize',value='4000')
 E.indent(root);return E.tostring(root,encoding='utf-8',xml_declaration=True)
for remove in [False,True]:
 name='CU Avatars - '+('Remove Models' if remove else 'Models')+'.oiv';data=assembly(remove);(W/('uninstall-assembly.xml' if remove else 'install-assembly.xml')).write_bytes(data)
 with zipfile.ZipFile(B/name,'w',zipfile.ZIP_DEFLATED) as z:
  z.writestr('assembly.xml',data)
  if not remove:z.write(W/'dlc.rpf','content/cuavatars/dlc.rpf')
  else:z.writestr('content/Readme.txt','Removes only this custom model pack. Remove runtime files manually after closing GTA.')

manifest=[]
for p in sorted(B.rglob('*')):
 if p.is_file() and p.name!='SHA256.json':
  with p.open('rb') as f:h=hashlib.file_digest(f,'sha256').hexdigest()
  manifest.append({'path':p.relative_to(B).as_posix(),'sha256':h,'bytes':p.stat().st_size})
(B/'SHA256.json').write_text(json.dumps(manifest,indent=2))
archive=O/f'CU-Avatars-{VERSION}.zip'
with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED) as z:
 for p in sorted(B.rglob('*')):
  if p.is_file():z.write(p,'CU Avatars/'+p.relative_to(B).as_posix())

source=O/f'CU-Avatars-{VERSION}-Source.zip'
with zipfile.ZipFile(source,'w',zipfile.ZIP_DEFLATED) as z:
 for name in ['CUAvatars.cpp','WeaponCatalog.h','ManualSwitchStamp.h','Test-ManualSwitchStamp.cpp','CosmeticPolicy.h','Test-CosmeticPolicy.cpp','RespawnDelay.h','Test-RespawnDelay.cpp','HurtAudio.h','HurtReactionPolicy.h','Test-HurtReactionPolicy.cpp','CUAvatars.ini','build.cmd','AssetTool/Program.cs','AssetTool/AssetTool.csproj','package.py','docs/Legacy installation guide.txt','docs/make_guides.py','Test-Installer.ps1','verify_package.py']:
  z.write(W/name,'CU Avatars Source/'+name)
 for name in ['install-assembly.xml','uninstall-assembly.xml']:z.write(W/name,'CU Avatars Source/Installer/'+name)
 for name in ['peds.meta']:
  z.write(W/'dlc-tree/common/data'/name,'CU Avatars Source/DLC/common/data/'+name)
 for name in ['content.xml','setup2.xml']:z.write(W/'dlc-tree'/name,'CU Avatars Source/DLC/'+name)
 for name in ['Credits.txt','Asset permissions.txt','Code license.txt','Roaming test checklist.txt']:z.write(B/name,'CU Avatars Source/'+name)
 z.writestr('CU Avatars Source/Build instructions.txt','''Plugin: download Script Hook V SDK separately. In an x64 Visual Studio Native
Tools prompt run build.cmd "path to SDK". It uses static MSVC runtime linking.
Policy regression: cl /EHsc /std:c++17 Test-CosmeticPolicy.cpp, then run the EXE.
Do not redistribute ScriptHookV.dll, the SDK headers/import library or ASI loader.

AssetTool is an optional .NET8 packing/checking tool. Download CodeWalker source
separately and respect its terms. Build AssetTool.csproj with
/p:CodeWalkerCoreProject="path to CodeWalker.Core.csproj".
No CodeWalker or SharpDX binaries, extracted game keys or SDK files are included.
Game-ready art is supplied only in the release's custom DLC. Original avatar
Blender/FBX/Unity/Substance sources and private messages are deliberately absent.
The DLC metadata templates are included; the artist's asset permissions still apply.
The private conversion pipeline also uses licensed converted avatar inputs;
this code archive does not contain the creators' original editable avatar projects.
The release DLC contains the fitted runtime assets. Extract its custom model RPF
with OpenIV if you need those binaries for a permitted rebuild, then use AssetTool
pack with the included DLC metadata. Do not treat the source license as an art license.
''')
(W/'package-report.json').write_text(json.dumps({'version':VERSION,'download':str(archive),'source':str(source),'download_bytes':archive.stat().st_size,'source_bytes':source.stat().st_size,'public_upload_performed':False,'local_live_startup_and_F5':'passed: user confirmed Legacy3889, RageOpenV backend','OpenIV_live_install':'pending','interaction_regression':'pending','original_game_archives_bundled':False,'third_party_runtime_binaries_bundled':False},indent=2))
print('Created',archive.name,archive.stat().st_size,'bytes; source',source.stat().st_size,'bytes')
