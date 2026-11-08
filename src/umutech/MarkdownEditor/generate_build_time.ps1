$now = Get-Date
$outPath = "res\build_time.h"

$content = @"
#define BUILD_YEAR $($now.Year)
#define BUILD_MONTH $($now.Month)
#define BUILD_DAY $($now.Day)
#define BUILD_HOUR $($now.Hour)

"@

$content | Out-File -FilePath $outPath -Encoding utf8 -NoNewline

