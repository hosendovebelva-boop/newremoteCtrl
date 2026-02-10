# Fix #5: TRACE overflow in CClientSocket.h
$file = Join-Path $PSScriptRoot "RemoteClient\CClientSocket.h"
$lines = [System.IO.File]::ReadAllLines($file, [System.Text.Encoding]::UTF8)

$newLines = [System.Collections.Generic.List[string]]::new()
$i = 0
while ($i -lt $lines.Count) {
    $line = $lines[$i]
    # Match the TRACE line with strData.c_str() + 12
    if ($line -match '^\s+TRACE\("%s\\r\\n", strData\.c_str\(\) \+ 12\);') {
        # Replace previous comment line too
        if ($newLines.Count -gt 0 -and $newLines[$newLines.Count - 1] -match '^\s+//') {
            $newLines.RemoveAt($newLines.Count - 1)
        }
        $indent = "`t`t`t"
        $newLines.Add("$indent// [`u{539F}`u{4EE3}`u{7801}] TRACE(""%s\r\n"", strData.c_str() + 12);")
        $newLines.Add("$indent// [`u{95EE}`u{9898}] `u{5047}`u{8BBE} strData `u{81F3}`u{5C11}`u{6709} 12 `u{5B57}`u{8282}`u{FF0C}`u{4F46}`u{78C1}`u{76D8}`u{5206}`u{533A}`u{4FE1}`u{606F} ""C,D,E"" `u{53EA}`u{6709} 5 `u{5B57}`u{8282}")
        $newLines.Add("$indent//        strData.c_str() + 12 `u{8D8A}`u{754C}`u{8BFB}`u{53D6} `u{2192} Debug `u{6A21}`u{5F0F}`u{4E0B}`u{53EF}`u{80FD}`u{89E6}`u{53D1}`u{8BBF}`u{95EE}`u{51B2}`u{7A81}")
        $newLines.Add("$indent// [`u{65B0}`u{4EE3}`u{7801}] `u{6DFB}`u{52A0}`u{957F}`u{5EA6}`u{68C0}`u{67E5}")
        $newLines.Add("${indent}if (strData.size() > 12)")
        $newLines.Add("$indent{")
        $newLines.Add("$indent`tTRACE(""%s\r\n"", strData.c_str() + 12);")
        $newLines.Add("$indent}")
        $newLines.Add("$indent// [`u{65B0}`u{4EE3}`u{7801}`u{7ED3}`u{675F}]")
    } else {
        $newLines.Add($line)
    }
    $i++
}

[System.IO.File]::WriteAllLines($file, $newLines.ToArray(), [System.Text.Encoding]::UTF8)
Write-Host "Fix #5 done: CClientSocket.h TRACE fix applied"

# Fix #6: _findfirst handle truncation in Command.h
$file2 = Join-Path $PSScriptRoot "RemoteCtrl\Command.h"
$content2 = [System.IO.File]::ReadAllText($file2, [System.Text.Encoding]::UTF8)

$oldFind = "`t`t_finddata_t fdata;`r`n`t`tint hfind = 0;"
$newFind = "`t`t_finddata_t fdata;`r`n`t`t// [`u{539F}`u{4EE3}`u{7801}] int hfind = 0;`r`n`t`t// [`u{95EE}`u{9898}] _findfirst `u{8FD4}`u{56DE} intptr_t`u{FF08}64`u{4F4D}`u{7CFB}`u{7EDF}`u{4E3A} 8 `u{5B57}`u{8282}`u{FF09}`u{FF0C}`u{7528} int`u{FF08}4 `u{5B57}`u{8282}`u{FF09}`u{5B58}`u{50A8}`u{4F1A}`u{622A}`u{65AD}`u{53E5}`u{67C4}`u{503C}`r`n`t`t// [`u{65B0}`u{4EE3}`u{7801}] `u{4F7F}`u{7528} intptr_t `u{786E}`u{4FDD}`u{5728} 64 `u{4F4D}`u{7CFB}`u{7EDF}`u{6B63}`u{786E}`u{5B58}`u{50A8}`u{53E5}`u{67C4}`r`n`t`tintptr_t hfind = 0;`r`n`t`t// [`u{65B0}`u{4EE3}`u{7801}`u{7ED3}`u{675F}]"

if ($content2.Contains($oldFind)) {
    $content2 = $content2.Replace($oldFind, $newFind)
    [System.IO.File]::WriteAllText($file2, $content2, [System.Text.Encoding]::UTF8)
    Write-Host "Fix #6 done: Command.h intptr_t fix applied"
} else {
    # Try with LF line endings
    $oldFindLF = $oldFind.Replace("`r`n", "`n")
    if ($content2.Contains($oldFindLF)) {
        $content2 = $content2.Replace($oldFindLF, $newFind.Replace("`r`n", "`n"))
        [System.IO.File]::WriteAllText($file2, $content2, [System.Text.Encoding]::UTF8)
        Write-Host "Fix #6 done (LF): Command.h intptr_t fix applied"
    } else {
        Write-Host "Fix #6: pattern not found in Command.h"
    }
}

Write-Host "All fixes complete."
