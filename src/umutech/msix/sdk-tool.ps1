function Find-SdkTool {
    param([string]$Name)

    $root = "${env:ProgramFiles(x86)}\Windows Kits\10\bin"
    if (Test-Path $root) {
        $found = @(
            Get-ChildItem $root -Directory -ErrorAction SilentlyContinue |
            Where-Object { $_.Name -match '^\d+(\.\d+)+$' } |
            Sort-Object { [version]$_.Name } -Descending |
            ForEach-Object { Join-Path $_.FullName "x64\$Name.exe" } |
            Where-Object { Test-Path $_ }
        )
        if ($found.Count -gt 0) { return $found[0] }
    }

    $inPath = Get-Command "$Name.exe" -ErrorAction SilentlyContinue
    if ($inPath) { return $inPath.Source }

    throw "找不到 $Name.exe，请安装 Windows SDK（Signing Tools / App Packaging）"
}
