param(
    [ValidateSet('x64', 'arm64', 'all')]
    [string]$Arch = 'all',

    [string]$ProductName = 'MarkdownEditor',
    [string]$Publisher = 'CN=A3AC9B40-C722-4AD6-B074-FCB759F2C7C4',

    [string]$OutDir = ''
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$archList = if ($Arch -eq 'all') { @('x64', 'arm64') } else { @($Arch) }
if (-not $OutDir) { $OutDir = Join-Path $PSScriptRoot 'build' }
Write-Host "Arch  : $archList"
Write-Host "OutDir: $OutDir"

. (Join-Path $PSScriptRoot 'sdk-tool.ps1')
$makeappx = Find-SdkTool 'MakeAppx'
$signtool = Find-SdkTool 'signtool'
Write-Host "MakeAppx: $makeappx"
Write-Host "signtool: $signtool"

function New-MsixPackage {
    param(
        [string]$TargetArch,
        [string]$ExeName,
        [string]$OutDir,
        [string]$Publisher
    )

    $layoutDir = Join-Path $OutDir "layout-$TargetArch"
    $exePath = Join-Path $layoutDir $ExeName -Resolve

    $exeInfo = Get-Item $exePath
    $version = $exeInfo.VersionInfo.ProductVersionRaw.ToString()
    if ([string]::IsNullOrWhiteSpace($version)) { $version = '1.0.0.0' }

    Write-Host ''
    Write-Host "---------- 打包 $TargetArch ----------"
    Write-Host "源程序   : $exePath ($version, $TargetArch)"

    $manifest = Get-Content (Join-Path $PSScriptRoot 'AppxManifest.xml') -Raw -Encoding UTF8
    $manifest = $manifest -creplace ' Version="[0-9]+(\.[0-9]+){1,3}"', (' Version="{0}"' -f $version)
    $manifest = $manifest -creplace 'Publisher="[^"]+"', ('Publisher="{0}"' -f $Publisher)
    $manifest = $manifest -creplace 'ProcessorArchitecture="[^"]+"', ('ProcessorArchitecture="{0}"' -f $TargetArch)
    $utf8NoBom = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText((Join-Path $layoutDir 'AppxManifest.xml'), $manifest, $utf8NoBom)

    Write-Host '生成图标资源...'
    & (Join-Path $PSScriptRoot 'make-assets.ps1') -OutDir (Join-Path $layoutDir 'Assets')

    # 打包
    $msixPath = Join-Path $OutDir ("MarkdownEditor_{0}_{1}.msix" -f $version, $TargetArch)
    if (Test-Path $msixPath) { Remove-Item $msixPath -Force }

    Write-Host '调用 MakeAppx 打包...'
    & $makeappx pack /d $layoutDir /p $msixPath /o | Out-Host
    if ($LASTEXITCODE -ne 0) { throw "MakeAppx 打包失败（$TargetArch），退出码 $LASTEXITCODE" }
    Write-Host "已生成: $msixPath"

    return $msixPath
}

$exeName = $ProductName + '.exe'
$builtPackages = @()
foreach ($currentArch in $archList) {
    $builtPackages += New-MsixPackage -TargetArch $currentArch -ExeName $exeName -OutDir $OutDir -Publisher $Publisher
}
Write-Host ''
Write-Host '================= 完成 ================='
foreach ($pkg in $builtPackages) { Write-Host "MSIX  : $pkg" }
