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


# The C++ checker has ONE lambda-introducer test, used by the function rule and
# the declaration rule (plan 4, R4). Disabling it disables both uses, so the
# reference disables both of its uses too (corrected in plan v1.4).
original_intro = ref.is_lambda_intro
original_closes = ref.closes_lambda_intro
ref.is_lambda_intro = lambda prev: False
ref.closes_lambda_intro = lambda cleaned, j: False
print('DF0b-3 (isLambdaIntroducer always false) failing:', snippets.failing())
ref.is_lambda_intro = original_intro
ref.closes_lambda_intro = original_closes
ref.lambda_in_head = never_lambda
print('DF0b-3a (only the function rule loses lambdas) failing:', snippets.failing())
ref.lambda_in_head = original_lambda


def no_specifier_groups(rest):
    return all(t[1] not in ('(', ')', '=', '[', ']') for t in rest)


original_specifiers = ref.only_specifiers
ref.only_specifiers = no_specifier_groups
print('DF0b-6 (noexcept(...) not allowed in lambda heads) failing:', snippets.failing())
ref.only_specifiers = original_specifiers

original_kind = ref.checked_kind


def kind_without_reference_exemption(rel):
    if rel.startswith('tools/reference/'):
        return 'ps1' if rel.endswith('.ps1') else ('cpp' if rel.endswith(('.h', '.cpp')) else '')
    return original_kind(rel)


snippets.checked_kind = kind_without_reference_exemption
print('DF0b-7 (tools/reference/ not exempt) failing:', snippets.files_failing())
snippets.checked_kind = original_kind
print('file-kind baseline failing:', snippets.files_failing())
