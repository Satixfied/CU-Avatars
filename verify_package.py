"""Package namespace, content/permission exclusions, hashes and PE architecture."""
import hashlib,json,struct,zipfile,wave,xml.etree.ElementTree as E
from pathlib import Path,PurePosixPath
W=Path(__file__).resolve().parent;O=W.parent.parent/'outputs';B=W/'release'
report={}
meta=E.parse(W/'dlc-tree/common/data/peds.meta').getroot()
assert [n.findtext('Name') for n in meta.find('InitDatas')]==['cu_expie','cu_milky','cu_dune']
assert all(n.find('IsStreamedGfx').get('value')=='false' for n in meta.find('InitDatas'))
assert {p.name for p in (W/'ped-files').iterdir()}=={f'cu_{model}{ext}' for model in ['expie','milky','dune'] for ext in ['.ydd','.yft','.ytd','.ymt','_faces.ycd']}
report['model_namespace']='passed: only3new IDs,15custom ped/face resources'
data=(B/'Game folder/CUAvatars.asi').read_bytes();pe=struct.unpack_from('<I',data,0x3c)[0]
assert data[:2]==b'MZ' and data[pe:pe+4]==b'PE\0\0' and struct.unpack_from('<H',data,pe+4)[0]==0x8664
assert struct.unpack_from('<H',data,pe+22)[0]&0x2000
assert b'C:\\Users\\Kevin' not in data
assert b'CUAvatars_QueryManualSwitch' in data
assert b'CUAvatars_IsRecoveryPending' in data and b'fallback-death-fade' not in data
report['plugin']='passed: native AMD64 DLL, no private source path'
for item in json.loads((B/'SHA256.json').read_text()):
 p=B/item['path'];assert p.stat().st_size==item['bytes'] and hashlib.file_digest(p.open('rb'),'sha256').hexdigest()==item['sha256']
report['file_hashes']='passed'
for name in ['bark1.wav','bark2.wav','bark3.wav','bark4.wav','whine1.wav','whine2.wav']:
 p=B/'Game folder/CUAvatars/sounds'/name
 assert p.read_bytes()==(W/'sounds'/name).read_bytes()
 with wave.open(str(p)) as wav:assert wav.getnchannels()==1 and wav.getsampwidth()==2 and wav.getframerate()==48000 and wav.getnframes()>0
report['hurt_sounds']='passed: six WAVs match approved build inputs (whines attenuated); mono PCM16/48kHz'
for suffix in ['', '-Source']:
 archive=O/('CU-Avatars-0.1.1-alpha'+suffix+'.zip')
 with zipfile.ZipFile(archive) as z:
  assert z.testzip() is None
  for name in z.namelist():
   p=PurePosixPath(name);assert not p.is_absolute() and '..' not in p.parts
   assert not name.lower().endswith(('.blend','.blend1','.fbx','.unitypackage','.spp','.png','.jpg','.pdb','.lib'))
   assert p.name.lower() not in ['scripthookv.dll','dinput8.dll','openiv.asi','x64e.rpf','x64v.rpf','x64g.rpf','update.rpf','common.rpf']
 report['zip'+suffix]='passed: CRC, safe paths, no avatar originals/DM screenshots/third-party runtime binaries'
for name in ['CU Avatars - Models.oiv','CU Avatars - Remove Models.oiv']:
 with zipfile.ZipFile(B/name) as z:
  assert z.testzip() is None;assembly=E.fromstring(z.read('assembly.xml'));assert assembly.get('target')=='Five' and assembly.get('version')=='2.2'
  for command in assembly.find('content'):
   if command.tag=='add':assert 'content/'+command.get('source').replace('\\','/') in z.namelist()
   if command.tag=='archive':assert command.get('path')=='update\\update.rpf' and command.get('createIfNotExist')=='False'
report['oiv']='passed: schema essentials and sources'
report['live_test']='User confirmed Legacy3889 startup/F5, phone pause/recovery, hurt sounds, F5 switches retaining presets, moving Scorcher/Milky switch and full-avatar WASTED followed by native hospital respawn and automatic avatar return. Latest log: native handoff on natural black after8.5sec, hospital relocation, normal control then Expie reapply. Whines balanced down14-15dB. Local RageOpenV backend. Broader all-model/arrest/vehicle/weapon regression and OpenIV backend still unverified.'
(W/'verification.json').write_text(json.dumps(report,indent=2))
print(json.dumps(report,indent=2))
