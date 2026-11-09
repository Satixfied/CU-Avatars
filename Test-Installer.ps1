$ErrorActionPreference='Stop'
function Apply-CUInstallerXml([xml]$Document,[xml]$Assembly,[string]$Path='common\data\dlclist.xml'){
 foreach($command in $Assembly.SelectNodes("/package/content/archive/xml[@path='$Path']/*")){
  if($command.Name -eq 'remove'){
   foreach($node in @($Document.SelectNodes($command.GetAttribute('xpath')))){[void]$node.ParentNode.RemoveChild($node)}
  } elseif($command.Name -eq 'add'){
   $parent=$Document.SelectSingleNode($command.GetAttribute('xpath'))
   foreach($child in $command.ChildNodes){if($child.NodeType -eq [System.Xml.XmlNodeType]::Element){[void]$parent.AppendChild($Document.ImportNode($child,$true))}}
  } elseif($command.Name -eq 'replace'){
   foreach($node in @($Document.SelectNodes($command.GetAttribute('xpath')))){
    $child=@($command.ChildNodes | Where-Object {$_.NodeType -eq [System.Xml.XmlNodeType]::Element})
    if($child.Count -ne 1){throw 'Expected one replacement element'}
    [void]$node.ParentNode.ReplaceChild($Document.ImportNode($child[0],$true),$node)
   }
  } else {throw ('Unexpected installer command: '+$command.Name)}
 }
}
$cuInstall=[xml](Get-Content -LiteralPath (Join-Path $PSScriptRoot 'install-assembly.xml') -Raw)
$cuRemove=[xml](Get-Content -LiteralPath (Join-Path $PSScriptRoot 'uninstall-assembly.xml') -Raw)
$cuFixture=[xml]'<SMandatoryPacksData><Paths><Item>platform:/dlcPacks/base/</Item><Item>dlcpacks:/another_mod/</Item><Item>dlcpacks:/custom_car/</Item><Item> DLCpacks:\CUAVATARS\ </Item></Paths></SMandatoryPacksData>'
Apply-CUInstallerXml $cuFixture $cuInstall
Apply-CUInstallerXml $cuFixture $cuInstall
$cuEntries=@($cuFixture.SelectNodes('/SMandatoryPacksData/Paths/Item') | ForEach-Object {$_.InnerText})
if($cuEntries.Count -ne 4 -or @($cuEntries | Where-Object {$_ -eq 'dlcpacks:/cuavatars/'}).Count -ne 1){throw 'Reinstall/duplicate registration check failed'}
Apply-CUInstallerXml $cuFixture $cuRemove
$cuRemaining=@($cuFixture.SelectNodes('/SMandatoryPacksData/Paths/Item') | ForEach-Object {$_.InnerText})
if(($cuRemaining -join '|') -ne 'platform:/dlcPacks/base/|dlcpacks:/another_mod/|dlcpacks:/custom_car/'){throw 'Unrelated entries were changed'}
if($cuInstall.SelectNodes('/package/content/add').Count -ne 1 -or $cuInstall.package.content.add.InnerText -ne 'update\x64\dlcpacks\cuavatars\dlc.rpf'){throw 'Unexpected file install target'}
if($cuRemove.package.content.delete -ne 'update\x64\dlcpacks\cuavatars\dlc.rpf'){throw 'Unexpected uninstall target'}
foreach($cuCapacity in @(3200,4000,8000)){
 $cuConfigFixture=[xml]("<fwAllConfigs><ConfigArray><Item><Build>Any</Build><Platforms>Any</Platforms><Config type='CGameConfig'><PoolSizes><Entries><Item><PoolName>MetaDataStore</PoolName><PoolSize value='$cuCapacity'/></Item><Item><PoolName>FragmentStore</PoolName><PoolSize value='42000'/></Item></Entries></PoolSizes></Config></Item></ConfigArray></fwAllConfigs>")
 Apply-CUInstallerXml $cuConfigFixture $cuInstall 'common\data\gameconfig.xml'
 Apply-CUInstallerXml $cuConfigFixture $cuInstall 'common\data\gameconfig.xml'
 $cuExpected=[Math]::Max(4000,$cuCapacity)
 if($cuConfigFixture.SelectSingleNode("//Item[PoolName='MetaDataStore']/PoolSize").GetAttribute('value') -ne [string]$cuExpected){throw 'Metadata capacity preservation failed'}
 if($cuConfigFixture.SelectSingleNode("//Item[PoolName='FragmentStore']/PoolSize").GetAttribute('value') -ne '42000'){throw 'Unrelated pool changed'}
 Apply-CUInstallerXml $cuConfigFixture $cuRemove 'common\data\gameconfig.xml'
 if($cuConfigFixture.SelectSingleNode("//Item[PoolName='MetaDataStore']/PoolSize").GetAttribute('value') -ne [string]$cuExpected){throw 'Uninstall must retain shared capacity'}
}
'PASS: install twice yields one namespaced DLC entry; uninstall retains stock and unrelated mod entries.'
'PASS: metadata capacity raised only below 4000; larger limits and unrelated pools preserved.'
