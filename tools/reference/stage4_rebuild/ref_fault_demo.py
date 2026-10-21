"""Reference for the Stage 4 rebuild, part 6: the Stage 4 demonstration text and the program
output (plan STAGE_04, 3.12 and Section 5). It follows the original src/demo/fault_demo.cpp at
89ea9b6 line by line; its 68 lines equal lines 103-170 of the carried-over
../stage4/expected_output_stage4.txt."""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(HERE, '..', 'stage3'))
import ref_agents as ra  # noqa: E402
import ref_fault_csv as rcsv  # noqa: E402
import ref_faults as rf  # noqa: E402
import ref_material as rmat  # noqa: E402
import ref_verify as rc  # noqa: E402

rw = rmat.rw
SETTINGS = ra.Settings(200, 5, 100, 3)
HEADER = ['AI Hologram v2 -- Stage 4',
          'Grid world with a competent single agent (C1), a naive agent (C0), fault injection and measurement.',
          'Faults are injected only on demonstration material; no two-agent reasoning exists, and no experiment '
          'is run.']


def demo_material():
    runs = []
    layouts = []
    first = None
    for seed in range(500000, 500004):
        generated = rmat.generate_layout(seed)
        if seed == 500000:
            first = generated
        for run_seed in (1, 2):
            runs.append((seed, run_seed))
            layouts.append(generated.world)
    return first, layouts, rmat.generate_fault_events(runs, layouts, 4242)


def demo_run(condition, world, events):
    first = rc.Observation(1, world)
    agent = ra.C1(first, SETTINGS) if condition == 'C1' else ra.C0(first, SETTINGS)
    result, _, _ = rf.run_with_faults(world, agent, SETTINGS, events, True)
    safe_stops = agent.safe_stops if condition == 'C1' else 0
    return {'condition': condition, 'layout': events[0].layout_seed, 'run': events[0].run_seed,
            'events': events, 'result': result, 'safe_stops': safe_stops}


def activation_suffix(record):
    text = rf.reported_as(record)
    if text == 'Activated at its scheduled step':
        return ' (step %d)' % record.application_step
    if text == 'Activated after deferral':
        return ' (step %d, delay %d)' % (record.application_step, rf.activation_delay(record))
    if record.reason:
        return ' (%s)' % record.reason
    return ''


def fault_demo_lines():
    first, layouts, events = demo_material()
    runs = []
    for index in (1, 6):
        runs.append(demo_run('C1', layouts[index], events[index]))
        runs.append(demo_run('C0', layouts[index], events[index]))
    out = ['', 'Stage 4: the measuring instrument, on demonstration material (not an experiment)',
           "Faults are injected into a reasoner's beliefs or reasoning, never into the world.",
           'Demonstration seeds 500000-500003 only; no pilot or confirmatory layout is used.', '',
           'Layout 500000 (found at attempt %d, shortest solution %d world steps):' % (first.attempts,
                                                                                     first.shortest)]
    out += rw.render_world(first.world)
    out += ['', 'Fault events of runs 500000-1 and 500003-1 (fault master seed 4242):']
    for index in (0, 6):
        out += [rmat.fault_event_text(e) for e in events[index]]
    for run in runs:
        r = run['result']
        out.append('')
        out.append('Run %d-%d, %s: %s after %d world steps, %d decision steps, %d safe stops, %d model evaluations'
                   % (run['layout'], run['run'], run['condition'], r.outcome, r.world_steps, r.decision_steps,
                      run['safe_stops'], r.evaluations))
        for e, record in zip(run['events'], r.activations):
            out.append('  %s %s: %s%s' % (e.id, e.type, rf.reported_as(record), activation_suffix(record)))
    out += ['', 'runs.csv:', rcsv.RUN_HEADER]
    for run in runs:
        out.append(rcsv.run_row(run['condition'], run['layout'], run['run'], run['result'], run['safe_stops']))
    out += ['', 'events.csv:', rcsv.EVENT_HEADER]
    for run in runs:
        r = run['result']
        for e, record, score in zip(run['events'], r.activations, r.event_scores):
            out.append(rcsv.event_row(run['condition'], e, record, score))
    out += ['', 'These lines show the instrument working; they are not results and are never compared.']
    return out


def program_lines():
    """The whole Stage 4 program output: the new header, then the Stage 3 lines 4-102 unchanged,
    then the Stage 4 demonstration."""
    stage3 = open(os.path.join(HERE, '..', 'stage3', 'expected_program_output.txt')).read().split('\n')
    if stage3 and stage3[-1] == '':
        stage3 = stage3[:-1]
    return HEADER + stage3[3:] + fault_demo_lines()


if __name__ == '__main__':
    print('\n'.join(program_lines()))
