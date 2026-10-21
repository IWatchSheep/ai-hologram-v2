# Purpose: Holds steps 3 to 6 and 9 of run_checks.ps1:
# configure, build, the test program, CTest and the program.
# Introduced in: Stage 0b (rebuild)
# Architecture: These functions moved here when run_checks.ps1
# was split by job (plan 3.5, D-057). They keep their Stage 0a
# behaviour, except that the tests step now prints all the lines
# of the quiet test run. Only function definitions live here.

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

# Runs the tests quietly, prints all their lines, returns the check count.
function Invoke-TestProgram {
    Write-Host "[tests] running ai_hologram_tests (quiet)"
    $exe = Join-Path $buildDir "ai_hologram_tests.exe"
    $result = Invoke-Tool $exe @()
    if ($result.Lines.Count -eq 0) {
        Stop-Checks "test program printed nothing"
    }
    Write-ToolLines $result.Lines
    $last = $result.Lines[$result.Lines.Count - 1]
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
