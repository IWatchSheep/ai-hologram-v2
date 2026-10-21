"""Claude's deliberate failure checks (plan STAGE_00 Section 7, R11), tried on the reference first."""
import sys
sys.path.insert(0, '.')
import ref_code_checker as ref
import snippets

print('baseline failing:', snippets.failing())
ref.MAX_FUNCTION = 41
print('DF0b-1 (limit 41) failing:', snippets.failing())
ref.MAX_FUNCTION = 40

original_braces = ref.check_braces


def braces_ignoring_else(toks):
    return [v for v in original_braces(toks) if "'else'" not in v[2]]


ref.check_braces = braces_ignoring_else
print('DF0b-2 (else ignored) failing:', snippets.failing())
ref.check_braces = original_braces

original_lambda = ref.lambda_in_head


def never_lambda(head):
    return False


ref.lambda_in_head = never_lambda
print('DF0b-3 (lambda bodies not recognised) failing:', snippets.failing())
ref.lambda_in_head = original_lambda


def no_specifier_groups(rest):
    return all(t[1] not in ('(', ')', '=', '[', ']') for t in rest)


original_specifiers = ref.only_specifiers
ref.only_specifiers = no_specifier_groups
print('DF0b-6 (noexcept(...) not allowed in lambda heads) failing:', snippets.failing())
ref.only_specifiers = original_specifiers
