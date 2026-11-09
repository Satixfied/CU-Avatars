param([Parameter(Mandatory=$true)][string]$InputXml,[Parameter(Mandatory=$true)][string]$OutputXml,[switch]$Remove)
$ErrorActionPreference='Stop'
$source=(Resolve-Path -LiteralPath $InputXml).Path
$target=[IO.Path]::GetFullPath($OutputXml)
if($source -eq $target){throw 'Use a separate output XML; the input is preserved.'}
if(Test-Path -LiteralPath $target){throw 'Output already exists; choose a new output path.'}
$doc=New-Object System.Xml.XmlDocument
$doc.XmlResolver=$null
$doc.Load($source)
$paths=$doc.SelectSingleNode('/SMandatoryPacksData/Paths')
if($null -eq $paths){throw 'Not an exported GTA dlclist.xml (SMandatoryPacksData/Paths missing).'}
foreach($item in @($paths.SelectNodes('Item'))){
 if($item.InnerText.Trim().Replace('\','/').TrimEnd('/') -ieq 'dlcpacks:/cuavatars'){
  [void]$paths.RemoveChild($item)
 }
}
if(-not $Remove){$item=$doc.CreateElement('Item');$item.InnerText='dlcpacks:/cuavatars/';[void]$paths.AppendChild($item)}
$doc.Save($target)
Write-Output "Saved $target. Import it as common/data/dlclist.xml in the Enhanced MODS copy of update.rpf."
