"""Build the isolated Gen9 DLC and preview ZIPs; never installs or launches GTA."""
import hashlib,json,shutil,struct,subprocess,zipfile
from pathlib import Path
E=Path(__file__).resolve().parent; P=E.parent; ROOT=P.parent.parent; OUT=ROOT/'outputs'
def run(*args):subprocess.run(list(map(str,args)),check=True)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
legacy=[P/'dlc.rpf',P/'build/CUAvatars.asi',P/'creator/build/CUPhotoStage.asi']
before={str(p.relative_to(ROOT)):sha(p) for p in legacy}
run(E/'Converter/bin/Debug/net8.0/Converter.exe',P/'ped-files',E/'ped-files')
tree=E/'dlc-tree'; cd=tree/'x64/models/cdimages';cd.mkdir(parents=True,exist_ok=True)
for rel in ['setup2.xml','content.xml','common/data/peds.meta']:
 dest=tree/rel;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(P/'dlc-tree'/rel,dest)
tool=P/'AssetTool/bin/Debug/net8.0/AssetTool.exe'
run(tool,'pack',E/'ped-files',cd/'cuavatars.rpf')
run(tool,'pack',tree,E/'dlc.rpf')
run(tool,'audit',E/'dlc.rpf',tree,E/'ped-files')
report=json.loads((E/'conversion-report.json').read_text())
for row in report['files']:
 p=E/'ped-files'/row['file'];assert sha(p)==row['sha256']
 version=struct.unpack_from('<I',p.read_bytes(),4)[0]
 expected={'.ydd':159,'.yft':171,'.ytd':5}.get(p.suffix)
 if expected:assert version==expected,(p,version)
for name in ['CUAvatars']:
 data=(E/'build'/f'{name}.asi').read_bytes();pe=struct.unpack_from('<I',data,60)[0]
 assert data[:2]==b'MZ' and struct.unpack_from('<H',data,pe+4)[0]==0x8664
 assert 'GTA5_Enhanced.exe'.encode('utf-16-le') in data
 assert b'C:\\Users\\Kevin' not in data
for name in ['CosmeticPolicy','HurtReactionPolicy','ManualSwitchStamp','RespawnDelay']:
 run(P/'build'/f'Test-{name}.exe')
run('powershell','-NoProfile','-ExecutionPolicy','Bypass','-File',E/'Test-DlcList.ps1')
assert before=={str(p.relative_to(ROOT)):sha(p) for p in legacy},'Legacy changed'
files={
 'Game folder/CUAvatars.asi':E/'build/CUAvatars.asi',
 'Game folder/CUAvatars.ini':P/'CUAvatars.ini',
 'Game folder/mods/update/x64/dlcpacks/cuavatars/dlc.rpf':E/'dlc.rpf',
 'INSTALLATION GUIDE.txt':E/'INSTALLATION GUIDE.txt',
 'Patch-DlcList.ps1':E/'Patch-DlcList.ps1',
 'Verification/conversion-report.json':E/'conversion-report.json',
}
for p in sorted((P/'sounds').glob('*.wav')):files['Game folder/CUAvatars/sounds/'+p.name]=p
assert sum(n.endswith('.wav') for n in files)==6
for n in ['Credits.txt','Asset permissions.txt','Code license.txt']:files[n]=P/'release'/n
verification={'edition':'Enhanced','status':'preview; Enhanced1158.16 startup and prologue switching user-confirmed','converted_resources':9,'unchanged_shared_resources':6,'archive_readback':'15 resources + 3 metadata files match','policies':'four avatar policy suites passed','installation_xml_helper':'insertion/removal, preservation, duplicate handling, overwrite and invalid-structure rejection passed','legacy_unchanged':before,'runtime':'x64, Enhanced executable gate; shared ScriptHookV native implementation','live_checks_passed':['startup after MetaDataStore 3200 to 4000','native prologue character switching'], 'live_checks_pending':['F5 avatar selection','all three avatar visuals','phone, weapons, vehicles and respawn'], 'required_metadata_minimum':4000}
(E/'verification.json').write_text(json.dumps(verification,indent=2));files['Verification/verification.json']=E/'verification.json'
def archive(target,items,prefix):
 assert not any(any(t in name.lower() for t in ['cuphotostage','creator/', 'creator guide','filmpresets','filmingshots','actioncamera']) for name in items), 'Filming files in public package'
 with zipfile.ZipFile(target,'w',zipfile.ZIP_DEFLATED) as z:
  for name,p in sorted(items.items()):z.write(p,prefix+'/'+name)
  z.writestr(prefix+'/SHA256.json',json.dumps({name:sha(p) for name,p in sorted(items.items())},indent=2))
 with zipfile.ZipFile(target) as z:
  assert z.testzip() is None
  for name,h in json.loads(z.read(prefix+'/SHA256.json')).items():assert hashlib.sha256(z.read(prefix+'/'+name)).hexdigest()==h
 print('VERIFIED',target.name,target.stat().st_size,'bytes')
archive(OUT/'CU-Avatars-Enhanced-0.1.1-preview.zip',files,'CU Avatars Enhanced')
sources={}
for directory in [P,E,E/'Converter',P/'AssetTool']:
 for p in directory.iterdir():
  if p.is_file() and (p.suffix in {'.cpp','.h','.cs','.csproj','.cmd'} or (directory==E and p.name in {'package.py','Patch-DlcList.ps1','Test-DlcList.ps1','BUILD.md','INSTALLATION GUIDE.txt','verification.json','conversion-report.json'})):
   sources[str(p.relative_to(P)).replace('\\','/')]=p
for name in ['CUAvatars.ini','docs/make_guides.py','docs/Legacy installation guide.txt']:sources[name]=P/name
for name in ['Credits.txt','Code license.txt','Asset permissions.txt']:sources[name]=P/'release'/name
archive(OUT/'CU-Avatars-Enhanced-0.1.1-preview-Source.zip',sources,'CU Avatars Enhanced Source')
