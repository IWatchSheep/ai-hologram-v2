# Purpose: Holds the helpers every run_checks step uses: how
# a run stops, and how external programs are run and printed.
# Introduced in: Stage 0b (rebuild)
# Architecture: These functions moved here unchanged when
# run_checks.ps1 was split by job (plan 3.5, D-057). This file
# contains only function definitions; run_checks.ps1 dot-sources
# it before any step runs, so Stop-Checks ends the whole run.

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
