# Purpose: Runs every project check in order and stops at the first
# failure (rule R14 in docs/RULES.md).
# Introduced in: the first commit of ai-hologram-v2. Stage 0a adds the
# configure, build, tests, CTest and program steps (plan 3.5).
# Architecture: Uses only Windows PowerShell 5.1 built-ins, so it works
# on a fresh Windows session. Run: .\run_checks.ps1 from the root.

$ErrorActionPreference = "Stop"
$root = $PSScriptRoot
$buildDir = Join-Path $root "cmake-build-checks"
$mingwBin = "C:\Program Files\JetBrains\CLion 2026.2.3.1\bin\mingw\bin"
$cmakeBin = "C:\Program Files\JetBrains\CLion 2026.2.3.1\bin\cmake\win\x64\bin"
$ninjaExe = "C:\Program Files\JetBrains\CLion 2026.2.3.1\bin\ninja\win\x64\ninja.exe"

# Stops the whole run with a clear message and a non-zero exit code.
function Stop-Checks([string]$message) {
    Write-Host ""
    Write-Host "CHECKS FAILED: $message" -ForegroundColor Red
    exit 1
}

# Runs one external program, captures both streams as text lines
# (so PowerShell 5.1 does not stop on them), and returns the exit code.
function Invoke-Tool([string]$program, [string[]]$arguments) {
    $previous = $ErrorActionPreference
    $ErrorActionPreference = "Continue"
    $lines = @(& $program @arguments 2>&1 | ForEach-Object { "$_" })
    $code = $LASTEXITCODE
    $ErrorActionPreference = $previous
    return [PSCustomObject]@{ Lines = $lines; ExitCode = $code }
}

# Prints the lines captured from one external program.
function Write-ToolLines([object[]]$lines) {
    foreach ($line in $lines) {
        Write-Host $line
    }
}

# Adds CLion's MinGW and CMake to PATH; both folders must exist.
function Set-CompilerPath {
    if (-not (Test-Path $mingwBin)) {
        Stop-Checks "CLion MinGW folder not found: $mingwBin"
    }
    if (-not (Test-Path $cmakeBin)) {
        Stop-Checks "CLion CMake folder not found: $cmakeBin"
    }
    $env:Path = "$cmakeBin;$mingwBin;" + $env:Path
    Write-Host "[path] CLion MinGW and CMake added to PATH"
}

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

# Configures with MinGW Makefiles, or CLion's Ninja if make is missing.
function Invoke-Configure {
    $mingwMake = Join-Path $mingwBin "mingw32-make.exe"
    $generator = ""
    $makeProgram = ""
    if (Test-Path $mingwMake) {
        $generator = "MinGW Makefiles"
    } elseif (Test-Path $ninjaExe) {
        $generator = "Ninja"
        $makeProgram = $ninjaExe
    } else {
        Stop-Checks "no generator: mingw32-make.exe not found at $mingwMake and ninja.exe not found at $ninjaExe"
    }
    Write-Host "[configure] generator: $generator"
    $arguments = @("-S", $root, "-B", $buildDir, "-G", $generator,
        "-DCMAKE_BUILD_TYPE=Debug", "-DAI_HOLOGRAM_WARNINGS_AS_ERRORS=ON")
    if ($makeProgram -ne "") {
        $arguments = $arguments + "-DCMAKE_MAKE_PROGRAM=$makeProgram"
    }
    $result = Invoke-Tool "cmake" $arguments
    Write-ToolLines $result.Lines
    if ($result.ExitCode -ne 0) {
        Stop-Checks "cmake configure failed with exit code $($result.ExitCode)"
    }
}

# Builds; warnings are compiler errors and are also searched for in text.
function Invoke-Build {
    Write-Host "[build] building in cmake-build-checks"
    $result = Invoke-Tool "cmake" @("--build", $buildDir)
    Write-ToolLines $result.Lines
    if ($result.ExitCode -ne 0) {
        Stop-Checks "build failed with exit code $($result.ExitCode)"
    }
    foreach ($line in $result.Lines) {
        if ($line -match "warning:") {
            Stop-Checks "build output contains a warning: $line"
        }
    }
}

# Runs the tests quietly, prints their last line, returns the check count.
function Invoke-TestProgram {
    Write-Host "[tests] running ai_hologram_tests (quiet)"
    $exe = Join-Path $buildDir "ai_hologram_tests.exe"
    $result = Invoke-Tool $exe @()
    if ($result.Lines.Count -eq 0) {
        Stop-Checks "test program printed nothing"
    }
    $last = $result.Lines[$result.Lines.Count - 1]
    Write-Host $last
    if ($result.ExitCode -ne 0) {
        Stop-Checks "test program failed with exit code $($result.ExitCode)"
    }
    $matched = $last -match "^(\d+) checks, (\d+) failures$"
    if (-not $matched) {
        Stop-Checks "could not read the test summary line: $last"
    }
    return [int]$Matches[1]
}

# Runs the same test program again through CTest.
function Invoke-Ctest {
    Write-Host "[ctest] running CTest"
    $arguments = @("--test-dir", $buildDir, "--output-on-failure")
    $result = Invoke-Tool "ctest" $arguments
    Write-ToolLines $result.Lines
    if ($result.ExitCode -ne 0) {
        Stop-Checks "ctest failed with exit code $($result.ExitCode)"
    }
}

# Runs the program and reports how many lines it printed.
function Invoke-Program {
    Write-Host "[program] running ai_hologram"
    $exe = Join-Path $buildDir "ai_hologram.exe"
    $result = Invoke-Tool $exe @()
    Write-ToolLines $result.Lines
    if ($result.ExitCode -ne 0) {
        Stop-Checks "program failed with exit code $($result.ExitCode)"
    }
    $nonBlank = 0
    foreach ($line in $result.Lines) {
        if ($line.Trim().Length -gt 0) {
            $nonBlank = $nonBlank + 1
        }
    }
    $total = $result.Lines.Count
    Write-Host "program output: $total lines, $nonBlank non-blank"
}

# Shows what Git sees, for the owner's review (never changes anything).
function Show-GitStatus {
    Write-Host "[git] status:"
    $result = Invoke-Tool "git" @("-C", $root, "status", "--short", "--branch")
    Write-ToolLines $result.Lines
    if ($result.ExitCode -ne 0) {
        Stop-Checks "git status failed with exit code $($result.ExitCode)"
    }
}

Set-CompilerPath
$verified = Test-Provenance
Invoke-Configure
Invoke-Build
$testChecks = Invoke-TestProgram
Invoke-Ctest
Invoke-Program
Show-GitStatus
Write-Host ""
Write-Host "ALL CHECKS PASSED FOR STAGE 0a ($verified preserved files, $testChecks checks; code and format checks are added in Stage 0b)" -ForegroundColor Green
exit 0
