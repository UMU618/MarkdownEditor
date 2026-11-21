# 由源图 mde.png 生成 MSIX 所需的各尺寸 PNG 图标
# 使用 WPF 成像管线缩放：高质量重采样 + 正确处理透明通道（不会出现黑/白边）
param(
    [Parameter(Mandatory = $true)]
    [string]$OutDir,

    [string]$SourcePng = (Join-Path $PSScriptRoot '..\MarkdownEditor\res\mde.png'),

    [switch]$NoRelaunch
)

$ErrorActionPreference = 'Stop'

try {
    Add-Type -AssemblyName PresentationCore -ErrorAction Stop
    Add-Type -AssemblyName WindowsBase -ErrorAction Stop
}
catch {
    $ps51 = Join-Path $env:windir 'System32\WindowsPowerShell\v1.0\powershell.exe'
    if (-not $NoRelaunch -and (Test-Path $ps51)) {
        Write-Host '当前会话无法加载 WPF，改用 Windows PowerShell 5.1 生成图标...'
        & $ps51 -NoProfile -ExecutionPolicy Bypass -File $PSCommandPath -OutDir $OutDir -SourcePng $SourcePng -NoRelaunch
        exit $LASTEXITCODE
    }
    throw '无法加载 WPF (PresentationCore)，无法生成图标。'
}

if (-not (Test-Path $SourcePng)) { throw "找不到源图标: $SourcePng" }
$SourcePng = (Resolve-Path $SourcePng).Path
if (-not (Test-Path $OutDir)) { New-Item -ItemType Directory -Path $OutDir -Force | Out-Null }

function New-ScaledLogo {
    param([string]$Source, [int]$W, [int]$H, [string]$File, [double]$Padding = 0.03)

    $bmp = [System.Windows.Media.Imaging.BitmapImage]::new()
    $bmp.BeginInit()
    $bmp.UriSource = [System.Uri]$Source
    $bmp.CacheOption = [System.Windows.Media.Imaging.BitmapCacheOption]::OnLoad
    $bmp.EndInit()

    $srcW = [double]$bmp.PixelWidth
    $srcH = [double]$bmp.PixelHeight

    # 等比缩放并居中（contain），保留透明背景
    $availW = $W * (1.0 - 2.0 * $Padding)
    $availH = $H * (1.0 - 2.0 * $Padding)
    $scale = [math]::Min($availW / $srcW, $availH / $srcH)
    $dw = $srcW * $scale
    $dh = $srcH * $scale
    $dx = ($W - $dw) / 2.0
    $dy = ($H - $dh) / 2.0

    $group = [System.Windows.Media.DrawingGroup]::new()
    [System.Windows.Media.RenderOptions]::SetBitmapScalingMode($group, [System.Windows.Media.BitmapScalingMode]::HighQuality)

    $imgDrawing = [System.Windows.Media.ImageDrawing]::new()
    $imgDrawing.ImageSource = $bmp
    $imgDrawing.Rect = [System.Windows.Rect]::new($dx, $dy, $dw, $dh)
    $group.Children.Add($imgDrawing)

    $visual = [System.Windows.Media.DrawingVisual]::new()
    $dc = $visual.RenderOpen()
    try {
        $dc.DrawImage([System.Windows.Media.DrawingImage]::new($group), [System.Windows.Rect]::new(0, 0, $W, $H))
    }
    finally { $dc.Close() }

    $rtb = [System.Windows.Media.Imaging.RenderTargetBitmap]::new($W, $H, 96, 96, [System.Windows.Media.PixelFormats]::Pbgra32)
    $rtb.Render($visual)

    $enc = [System.Windows.Media.Imaging.PngBitmapEncoder]::new()
    $enc.Frames.Add([System.Windows.Media.Imaging.BitmapFrame]::Create($rtb))
    $fs = [System.IO.File]::Create($File)
    try { $enc.Save($fs) } finally { $fs.Close() }
}

$targets = @(
    @{ Name = 'Square44x44Logo.png';   W = 44;  H = 44 },
    @{ Name = 'StoreLogo.png';         W = 50;  H = 50 },
    @{ Name = 'Square71x71Logo.png';   W = 71;  H = 71 },
    @{ Name = 'Square150x150Logo.png'; W = 150; H = 150 },
    @{ Name = 'Square310x310Logo.png'; W = 310; H = 310 },
    @{ Name = 'Wide310x150Logo.png';   W = 310; H = 150 }
)

Write-Host "源图标: $SourcePng"
foreach ($t in $targets) {
    $file = Join-Path $OutDir $t.Name
    if (-not (Test-Path $file)) {
        New-ScaledLogo -Source $SourcePng -W $t.W -H $t.H -File $file
        $size = (Get-Item $file).Length
        Write-Host ("  生成 {0} ({1}x{2}, {3:N0} 字节)" -f $t.Name, $t.W, $t.H, $size)
    }
}
