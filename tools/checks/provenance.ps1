# Purpose: Holds step 2 of run_checks.ps1: every preserved
# file must exist, use LF endings and match its recorded hash.
# Introduced in: Stage 0b (rebuild)
# Architecture: These functions moved here unchanged when
# run_checks.ps1 was split by job (plan 3.5, D-057). This file
# contains only function definitions.

# Checks one preserved file exists, has no CR bytes, and matches its hash.
function Test-OneFile([string]$expected, [string]$relative) {
    $path = Join-Path $root $relative
    if (-not (Test-Path -LiteralPath $path)) {
        Stop-Checks "missing preserved file: $relative"
    }
    $bytes = [System.IO.File]::ReadAllBytes($path)
    if ([System.Array]::IndexOf($bytes, [byte]13) -ge 0) {
        Stop-Checks "$relative has CRLF line endings (expected LF from .gitattributes); do not edit it - ask Claude"
    }
    $actual = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLower()
    if ($actual -ne $expected) {
        Stop-Checks "$relative was changed (SHA-256 differs from docs/PROVENANCE.sha256)"
    }
}

# Rule R19: every carried-over or governing file matches its recorded hash.
function Test-Provenance {
    $list = Join-Path $root "docs/PROVENANCE.sha256"
    if (-not (Test-Path $list)) {
        Stop-Checks "docs/PROVENANCE.sha256 is missing"
    }
    $count = 0
    foreach ($line in [System.IO.File]::ReadAllLines($list)) {
        if ($line.Trim().Length -eq 0) {
            continue
        }
        $parts = $line -split "  ", 2
        Test-OneFile $parts[0].Trim() $parts[1].Trim()
        $count = $count + 1
    }
    Write-Host "[provenance] $count preserved files verified"
    return $count
}
