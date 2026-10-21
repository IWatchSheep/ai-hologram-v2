// Purpose: The test program entry point. It runs every test
// group, prints the summary line and reports failures by its
// exit code.
// Introduced in: Stage 0a (rebuild)
// Architecture: Groups are declared in test_suites.h. Output is
// quiet unless the first argument is --verbose (rule R15).

#include <iostream>
#include <string>

#include "test_suites.h"
#include "test_support.h"

int main(int argc, char* argv[]) {
    aih::TestCounter counter;
    if (argc > 1) {
        if (std::string(argv[1]) == "--verbose") {
            counter.verbose = true;
        }
    }
    aih::runProjectInfoTests(counter);
    std::cout << counter.checks << " checks, " << counter.failures
              << " failures\n";
    if (counter.failures == 0) {
        return 0;
    }
    return 1;
}
