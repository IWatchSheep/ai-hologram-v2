"""Equivalence check between the C++ code checker and this reference (plan STAGE_00 v1.5).

Writes many small random files built from C++-like pieces into a scratch folder, runs both
checkers over it, and compares their complete outputs line by line.
Usage:  python3 equivalence_fuzz.py <path to built code_checker> <scratch folder> [seeds...]
The scratch folder is deleted and re-created; never point it at the repository."""
import os
import random
import shutil
import subprocess
import sys

ATOMS = ['{', '}', '(', ')', '[', ']', '<', '>', '=', ',', ';', '\n', '\n', '\n', ' ', '  ',
         'a', 'b', 'int', 'if', 'else', 'for', 'while', 'do', 'return', 'template', 'enum',
         'class', 'struct', 'namespace', 'operator', 'noexcept', 'decltype', 'mutable', '->',
         '::', '"x;y"', "'c'", '// c;,\n', '/* a,b; */', 'R"(z)"', '#include "experiment/x.h"\n',
         '#include "../experiment/y.h"\n', '0', '1.5', '&', '*', '-', '!', '_k', 'function A {',
         '\n    x\n']


def write_files(root, seed, count):
    random.seed(seed)
    shutil.rmtree(root, ignore_errors=True)
    for i in range(count):
        sub = random.choice(['src/agent', 'src', 'tests', 'tools/t'])
        ext = '.ps1' if sub == 'tools/t' and random.random() < 0.2 else '.cpp'
        path = os.path.join(root, sub, f's{i}{ext}')
        os.makedirs(os.path.dirname(path), exist_ok=True)
        pieces = [random.choice(ATOMS) for _ in range(random.randint(1, 120))]
        with open(path, 'w', encoding='utf-8') as f:
            f.write(''.join(pieces))


def main():
    checker, root = sys.argv[1], sys.argv[2]
    seeds = [int(x) for x in sys.argv[3:]] or [1, 2, 3, 4, 5]
    reference = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'ref_code_checker.py')
    all_same = True
    for seed in seeds:
        write_files(root, seed, 3000)
        cpp = subprocess.run([checker, root], capture_output=True, text=True).stdout
        ref = subprocess.run([sys.executable, reference, root], capture_output=True, text=True).stdout
        same = cpp == ref
        all_same = all_same and same
        print(f'seed {seed}: {ref.strip().splitlines()[-1]}:', 'IDENTICAL' if same else 'DIFFERENT')
    print('all identical' if all_same else 'DIFFERENCES FOUND')


if __name__ == '__main__':
    main()
