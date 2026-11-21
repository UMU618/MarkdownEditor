param(
    [ValidateSet('x64', 'arm64', 'all')]
    [string]$Arch = 'all',

    [string]$ProductName = 'MarkdownEditor',

    [string]$OutDir = '',

    [switch]$Copy,
    [switch]$Sign,
    [switch]$Compress
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$srcDir = Join-Path $PSScriptRoot '..\bin' -Resolve
$exeName = $ProductName + '.exe'
# Write-Host "Source directory: $srcDir"

$archList = if ($Arch -eq 'all') { @('x64', 'arm64') } else { @($Arch) }
if (-not $OutDir) { $OutDir = Join-Path $PSScriptRoot 'build' }

Write-Host "Arch  : $archList"
Write-Host "OutDir: $OutDir"

if ($Copy) {
    Write-Host 'Copying...'
    foreach ($currentArch in $archList) {
        Write-Host "  [$currentArch]"
        $dstDir = Join-Path $OutDir "layout-$currentArch"
        if (-not (Test-Path $dstDir -PathType Container)) {
            New-Item -ItemType Directory $dstDir -Force
        }
        Copy-Item (Join-Path $srcDir $currentArch 'MTRelease' $exeName) $dstDir
    }
}

if ($Sign) {
    Write-Host 'Signing...'
    . (Join-Path $PSScriptRoot 'sdk-tool.ps1')
    $signtool = Find-SdkTool 'signtool'
    Write-Host "  signtool : $signtool"
    $baseArgs = @('sign', '/sha1', 'ff8a8160116e35775506d0dd86360935877aaf3b', '/fd', 'SHA512', '/t', 'http://timestamp.digicert.com/')
    foreach ($currentArch in $archList) {
        Write-Host "  [$currentArch]"
        $exePath = Join-Path $OutDir "layout-$currentArch" $exeName -Resolve
        & $signtool ($baseArgs + @($exePath)) | Out-Host
    }
}

if ($Compress) {
    Write-Host "Compressing..."
    $sevenZip = Join-Path $env:ProgramFiles '7-Zip' '7z.exe' -Resolve
    # exe 适合开启 BCJ2 过滤器
    # 单文件 exe，固实块 `-ms=on` 不加也没关系；多文件打包才需要
    $baseArgs = @('a', '-t7z', '-m0=LZMA2:d=64m:fb=64:mf=bt4', '-mx=9')
    $now = Get-Date
    $dt = $now.ToString('yyyy.M.d')
    foreach ($currentArch in $archList) {
        Write-Host "  [$currentArch]"
        $exePath = Join-Path $OutDir "layout-$currentArch" $exeName -Resolve
        $outPath = Join-Path $OutDir "${ProductName}_${dt}_${currentArch}.7z"
        & $sevenZip ($baseArgs + @($outPath, $exePath)) | Out-Host
    }
}
