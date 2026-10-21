# Purpose: Runs every project check in order and stops at the first
# failure (rule R14 in docs/RULES.md).
# Introduced in: the first commit of ai-hologram-v2. Stage 0a adds the
# configure, build, tests, CTest and program steps (plan 3.5). Stage 0b
# splits the step functions into tools/checks/ and adds steps 7 and 8.
# Architecture: Uses only Windows PowerShell 5.1 built-ins, so it works
# on a fresh Windows session. Run: .\run_checks.ps1 from the root.

$ErrorActionPreference = "Stop"
$root = $PSScriptRoot
$buildDir = Join-Path $root "cmake-build-checks"
$mingwBin = "C:\Program Files\JetBrains\CLion 2026.2.3.1\bin\mingw\bin"
$cmakeBin = "C:\Program Files\JetBrains\CLion 2026.2.3.1\bin\cmake\win\x64\bin"
$ninjaExe = "C:\Program Files\JetBrains\CLion 2026.2.3.1\bin\ninja\win\x64\ninja.exe"
$clangFormat = "C:\Program Files\JetBrains\CLion 2026.2.3.1\plugins\clion-radler\DotFiles\windows-x64\clang-format.exe"

# The step functions live in tools/checks/, split by job (D-057).
. (Join-Path $root "tools/checks/common.ps1")
. (Join-Path $root "tools/checks/provenance.ps1")
. (Join-Path $root "tools/checks/build_steps.ps1")
. (Join-Path $root "tools/checks/code_steps.ps1")

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
$codeFiles = Invoke-CodeChecker
Invoke-FormatCheck
Invoke-Program
Show-GitStatus
Write-Host ""
Write-Host "ALL CHECKS PASSED ($verified preserved files, $testChecks checks, $codeFiles code files checked, formatting checked)" -ForegroundColor Green
exit 0
