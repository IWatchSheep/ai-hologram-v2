"""The prediction-golden reader (plan STAGE_02, Section 6.6), mirrored on the reference.

It takes the file's lines and returns (ok, cases, problem); problem is '' when ok, else
exactly one of the messages below (line numbers count from 1). The C++ reader in
tests/golden_prediction_reader.cpp follows this data flow and these messages exactly.
It uses the shared helpers of Stage 1 (parse_integer; split on single spaces)."""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 'stage1'))
import ref_golden_readers as grd  # noqa: E402
import ref_model as rm  # noqa: E402

rw = rm.rw


def read_header(line):
    """'case <n> action <Action> differences <k>' -> (True, n, action, k)."""
    parts = line.split(' ')
    if (len(parts) != 6 or parts[0] != 'case' or parts[2] != 'action'
            or parts[4] != 'differences' or parts[3] not in rw.ACTIONS):
        return False, 0, '', 0
    ok_n, n = grd.parse_integer(parts[1])
    ok_k, k = grd.parse_integer(parts[5])
    if not ok_n or not ok_k or k < 0:
        return False, 0, '', 0
    return True, n, parts[3], k


def actual_lines(belief, truth, action):
    """The result line and difference lines the original produces for one case."""
    prediction = rm.WorldModel().predict(belief, action, rm.BudgetCounter(1))
    observed_state, observed_outcome = rw.apply_action(truth, action)
    comparison = rm.compare_prediction(prediction, observed_state, observed_outcome)
    verdict = 'match' if rm.matches(comparison) else 'MISMATCH'
    return (f'result {comparison[0]} {comparison[1]} {verdict}',
            rm.describe_differences(comparison[2]))


def check_case_lines(n, k, result_line, expected, actual):
    actual_result, actual_differences = actual
    if result_line != actual_result:
        return f'case {n}: expected [{result_line}] actual [{actual_result}]'
    if len(actual_differences) != k:
        return f'case {n}: expected {k} difference lines, actual {len(actual_differences)}'
    for j, (e, a) in enumerate(zip(expected, actual_differences), 1):
        if e != a:
            return f'case {n}: difference line {j}: expected [{e}] actual [{a}]'
    return ''


def check_prediction_golden_lines(lines):
    cases = 0
    i = 0
    while i < len(lines):
        ok, n, action, k = read_header(lines[i])
        if not ok:
            return False, cases, f'line {i + 1}: invalid case header'
        i += 1
        if i + 2 * rw.GRID > len(lines):
            return False, cases, f'case {n}: missing world lines'
        belief, error = rw.parse_world(lines[i:i + rw.GRID])
        if belief is None:
            return False, cases, f'case {n}: belief does not parse: {error}'
        truth, error = rw.parse_world(lines[i + rw.GRID:i + 2 * rw.GRID])
        if truth is None:
            return False, cases, f'case {n}: true world does not parse: {error}'
        i += 2 * rw.GRID
        if i >= len(lines):
            return False, cases, f'case {n}: missing result line'
        if not lines[i].startswith('result '):
            return False, cases, f'case {n}: malformed result line'
        result_line = lines[i]
        i += 1
        if i + k > len(lines):
            return False, cases, f'case {n}: missing difference lines'
        expected = lines[i:i + k]
        i += k
        cases += 1
        problem = check_case_lines(n, k, result_line, expected, actual_lines(belief, truth, action))
        if problem:
            return False, cases, problem
    return True, cases, ''
