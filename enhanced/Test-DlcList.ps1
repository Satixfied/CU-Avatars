$ErrorActionPreference='Stop'
$dir=Join-Path $PSScriptRoot 'test-output'
[void][IO.Directory]::CreateDirectory($dir)
$id=[Guid]::NewGuid().ToString('N')
$inputPath=Join-Path $dir "$id-input.xml"
$added=Join-Path $dir "$id-added.xml"
$removed=Join-Path $dir "$id-removed.xml"
$bad=Join-Path $dir "$id-bad.xml"
[IO.File]::WriteAllText($inputPath,'<SMandatoryPacksData><Paths><Item>dlcpacks:/original/</Item><Item>dlcpacks:/CUAVATARS/</Item><Item>dlcpacks:\cuavatars\</Item><Item>dlcpacks:/othermod/</Item></Paths></SMandatoryPacksData>')
$original=[IO.File]::ReadAllText($inputPath)
& "$PSScriptRoot\Patch-DlcList.ps1" -InputXml $inputPath -OutputXml $added
[xml]$result=Get-Content -LiteralPath $added -Raw
if(($result.SMandatoryPacksData.Paths.Item -join '|') -cne 'dlcpacks:/original/|dlcpacks:/othermod/|dlcpacks:/cuavatars/'){throw 'Did not preserve unrelated entries/deduplicate pack'}
if([IO.File]::ReadAllText($inputPath) -cne $original){throw 'Mutated input'}
& "$PSScriptRoot\Patch-DlcList.ps1" -InputXml $added -OutputXml $removed -Remove
[xml]$result=Get-Content -LiteralPath $removed -Raw
if(($result.SMandatoryPacksData.Paths.Item -join '|') -cne 'dlcpacks:/original/|dlcpacks:/othermod/'){throw 'Uninstall altered unrelated entries'}
$refused=$false
try { & "$PSScriptRoot\Patch-DlcList.ps1" -InputXml $inputPath -OutputXml $inputPath }catch{$refused=$true}
if(-not $refused){throw 'Did not reject in-place write'}
$refused=$false
try { & "$PSScriptRoot\Patch-DlcList.ps1" -InputXml $inputPath -OutputXml $added }catch{$refused=$true}
if(-not $refused){throw 'Did not reject existing output'}
[IO.File]::WriteAllText($bad,'<wrong/>')
$refused=$false
try { & "$PSScriptRoot\Patch-DlcList.ps1" -InputXml $bad -OutputXml (Join-Path $dir "$id-reject.xml") }catch{$refused=$true}
if(-not $refused){throw 'Did not reject invalid XML structure'}
Write-Output 'PASS: DLC insertion/removal, duplicate handling, unrelated entries preserved, source preserved, overwrite and invalid-structure refusal.'
