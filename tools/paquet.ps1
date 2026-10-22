# paquet.ps1 -- Mewgenics Multi : installer une build publiee et refaire le zip de l'ami.
#
#   powershell -ExecutionPolicy Bypass -File tools\paquet.ps1 -Build 52
#   powershell -ExecutionPolicy Bypass -File tools\paquet.ps1 -Build 52 -Installer
#   powershell -ExecutionPolicy Bypass -File tools\paquet.ps1 -Build 52 -Depuis C:\chemin\build-52
#
# 1. Les fichiers de la release build-N : telecharges par gh (GitHub CLI, connecte
#    avec un acces au depot), ou pris dans le dossier -Depuis (mgmp.dll,
#    mgmp_loader.exe et SHA256SUMS.txt, telecharges a la main depuis la page des
#    releases).
# 2. Leurs empreintes SHA-256 sont verifiees contre SHA256SUMS.txt.
# 3. Le zip de l'ami (un dossier mgmp\ : les fichiers de paquet\ et les deux
#    binaires) est ecrit dans -Dossier, sous le nom mgmp-pour-ton-ami.zip.
# 4. Avec -Installer : mgmp.dll et mgmp_loader.exe remplacent ceux de -Dossier
#    (le dossier d'ou Hizu lance le jeu), seulement si Mewgenics est ferme.
param(
    [Parameter(Mandatory = $true)][int]$Build,
    [string]$Dossier = (Join-Path ([Environment]::GetFolderPath('Desktop')) 'mgmp'),
    [string]$Depuis = '',
    [switch]$Installer
)
$ErrorActionPreference = 'Stop'
$depot = Split-Path -Parent $PSScriptRoot
$paquet = Join-Path $depot 'paquet'
$tmp = Join-Path ([IO.Path]::GetTempPath()) ("mgmp-build-$Build-" + [Guid]::NewGuid().ToString('N').Substring(0, 8))
New-Item -ItemType Directory -Force $tmp | Out-Null

# 1. Les fichiers de la release.
if ($Depuis) {
    $src = $Depuis
} else {
    $src = Join-Path $tmp 'release'
    New-Item -ItemType Directory -Force $src | Out-Null
    gh release download "build-$Build" -R Hizumimi/mewgenics-multi -D $src
    if ($LASTEXITCODE -ne 0) { throw "gh release download build-$Build a echoue" }
}

# 2. Les empreintes.
$attendu = @{}
foreach ($ligne in Get-Content (Join-Path $src 'SHA256SUMS.txt')) {
    if ($ligne -match '^([0-9a-fA-F]{64})\s+\*?(.+)$') { $attendu[$Matches[2].Trim()] = $Matches[1].ToLower() }
}
foreach ($nom in 'mgmp.dll', 'mgmp_loader.exe') {
    if (-not $attendu.ContainsKey($nom)) { throw "$nom n'est pas dans SHA256SUMS.txt" }
    $h = (Get-FileHash (Join-Path $src $nom) -Algorithm SHA256).Hash.ToLower()
    if ($h -ne $attendu[$nom]) { throw "$nom : empreinte $h, attendue $($attendu[$nom])" }
    Write-Host "$nom : empreinte verifiee"
}

# 3. Le zip de l'ami.
New-Item -ItemType Directory -Force $Dossier | Out-Null
$contenu = Join-Path $tmp 'zip\mgmp'
New-Item -ItemType Directory -Force $contenu | Out-Null
Copy-Item (Join-Path $paquet '*') $contenu
Copy-Item (Join-Path $src 'mgmp.dll'), (Join-Path $src 'mgmp_loader.exe') $contenu
$zip = Join-Path $Dossier 'mgmp-pour-ton-ami.zip'
# Pas Compress-Archive : sous Windows PowerShell 5.1, il ecrit les chemins avec des \
# (mgmp\mgmp.dll), que certains outils d'extraction prennent pour un nom de fichier.
Add-Type -AssemblyName System.IO.Compression, System.IO.Compression.FileSystem
if (Test-Path $zip) { Remove-Item $zip -Force }
$archive = [IO.Compression.ZipFile]::Open($zip, 'Create')
try {
    foreach ($f in Get-ChildItem $contenu -File) {
        [IO.Compression.ZipFileExtensions]::CreateEntryFromFile($archive, $f.FullName, "mgmp/$($f.Name)") | Out-Null
    }
} finally { $archive.Dispose() }
Write-Host "zip de l'ami : $zip"

# 4. L'installation chez Hizu.
if ($Installer) {
    if (Get-Process -Name Mewgenics -ErrorAction SilentlyContinue) {
        throw 'Mewgenics est ouvert : le fermer, puis relancer avec -Installer'
    }
    Copy-Item (Join-Path $src 'mgmp.dll'), (Join-Path $src 'mgmp_loader.exe') $Dossier -Force
    Write-Host "build $Build installee dans $Dossier"
}
