# Purpose: Runs every project check in order and stops at the first failure
# (rule R14 in docs/RULES.md).
# Introduced in: the first commit of ai-hologram-v2 (provenance check and git
# status only). Each stage plan adds its steps (configure, build, tests,
# CTest, size/style/architecture checks, equivalence gate, output counts).
# Architecture: Uses only Windows PowerShell 5.1 built-ins, so it works on a
# fresh Windows session with nothing extra installed.
#
# Run from the repository folder:
#   powershell -ExecutionPolicy Bypass -File .\run_checks.ps1

$ErrorActionPreference = "Stop"
$root = $PSScriptRoot

# Stops the whole run with a clear message and a non-zero exit code.
function Stop-Checks([string]$message) {
    Write-Host ""
    Write-Host "CHECKS FAILED: $message" -ForegroundColor Red
    exit 1
}

# Puts CLion's bundled MinGW on the PATH for this run only, so the
# project's programs find their DLLs even right after a restart.
function Set-CompilerPath {
    $mingw = "C:\Program Files\JetBrains\CLion 2026.2.3.1\bin\mingw\bin"
    if (Test-Path $mingw) {
        $env:Path = "$mingw;" + $env:Path
        Write-Host "[path] CLion MinGW added to PATH"
    } else {
        Write-Host "[path] CLion MinGW folder not found (not needed yet)" -ForegroundColor Yellow
    }
}

# Checks one preserved file: it exists, has no carriage return (so its
# bytes equal the committed LF bytes), and has the recorded SHA-256.
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

# Shows what Git sees, for the owner's review (never changes anything).
function Show-GitStatus {
    Write-Host "[git] status:"
    git -C $root status --short --branch
}

Set-CompilerPath
$verified = Test-Provenance
Show-GitStatus
Write-Host ""
Write-Host "ALL CHECKS PASSED ($verified preserved files verified)" -ForegroundColor Green
exit 0
