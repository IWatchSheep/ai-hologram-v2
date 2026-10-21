# Purpose: Holds steps 7 and 8 of run_checks.ps1, both new in
# Stage 0b: the code checker over the repository, and the
# formatting check with CLion's clang-format.
# Introduced in: Stage 0b (rebuild)
# Architecture: The formatting step takes its file list from
# code_checker --list-cpp, so one list of checked files is
# used twice (plan 3.4, D-058). A missing clang-format fails
# the whole run (D-054). Only function definitions live here.

# Step 7: runs the code checker and returns how many files it checked.
function Invoke-CodeChecker {
    Write-Host "[code] running code checker"
    $exe = Join-Path $buildDir "code_checker.exe"
    $result = Invoke-Tool $exe @($root)
    Write-ToolLines $result.Lines
    if ($result.ExitCode -ne 0) {
        Stop-Checks "code checker failed with exit code $($result.ExitCode)"
    }
    $summary = ""
    foreach ($line in $result.Lines) {
        if ($line -match "^code checks: (\d+) files, (\d+) violations$") {
            $summary = $line
        }
    }
    if ($summary -eq "") {
        Stop-Checks "could not read the code checker summary line"
    }
    $matched = $summary -match "^code checks: (\d+) files, (\d+) violations$"
    if (-not $matched) {
        Stop-Checks "could not read the code checker summary line: $summary"
    }
    return [int]$Matches[1]
}

# Step 8: fails when any listed C++ file differs from clang-format.
function Invoke-FormatCheck {
    if (-not (Test-Path $clangFormat)) {
        Stop-Checks "clang-format not found at $clangFormat; formatting was not checked - ask Claude"
    }
    $versionResult = Invoke-Tool $clangFormat @("--version")
    if ($versionResult.ExitCode -ne 0) {
        Stop-Checks "clang-format --version failed with exit code $($versionResult.ExitCode)"
    }
    $version = "$($versionResult.Lines[0])" -replace "^clang-format version ", ""
    Write-Host "[format] clang-format $version"
    $checker = Join-Path $buildDir "code_checker.exe"
    $listResult = Invoke-Tool $checker @("--list-cpp", $root)
    if ($listResult.ExitCode -ne 0) {
        Stop-Checks "code_checker --list-cpp failed with exit code $($listResult.ExitCode)"
    }
    foreach ($relative in $listResult.Lines) {
        $file = Join-Path $root $relative
        $arguments = @("--style=file", "--dry-run", "--Werror", $file)
        $result = Invoke-Tool $clangFormat $arguments
        if ($result.ExitCode -ne 0) {
            Write-ToolLines $result.Lines
            Stop-Checks "formatting differs in $relative (clang-format exit code $($result.ExitCode))"
        }
    }
    Write-Host "[format] formatting checked"
}
