"""Reference for the Stage 4 rebuild, part 5: the fault-trace cases and the fault-trace text of
the gate, part 2 (plan STAGE_04, 3.15). Its output must equal the golden fault_traces.txt, which
the ORIGINAL C++ produced (orig_trace/orig_fault_trace.cpp)."""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(HERE, '..', 'stage3'))
import ref_fault_csv as rcsv  # noqa: E402
import ref_faults as rf  # noqa: E402
import ref_material as rmat  # noqa: E402
import ref_trace as rt  # noqa: E402


class FaultCase:
    def __init__(self):
        self.base = None
        self.layout_seed = 0
        self.run_seed = 0
        self.events = []


def parse_event(w):
    e = rmat.FaultEvent()
    e.id = w[1]
    e.layout_seed = int(w[2])
    e.run_seed = int(w[3])
    e.index = int(w[4])
    e.step = int(w[5])
    e.type = w[6]
    e.target = w[7]
    e.common = w[8] == '1'
    if e.type in ('F1', 'F4'):
        if w[10] == 'none':
            k = 11
        else:
            e.has_cell = True
            e.cell = (int(w[10]), int(w[11]))
            k = 12
        if e.type == 'F1':
            e.direction = w[k]
    elif e.type == 'F2':
        e.order = w[9:13]
    elif e.type == 'F3':
        e.action = w[9]
        e.rotation = w[10]
    return e


def read_fault_cases(lines):
    bases = rt.read_cases(lines)
    cases = []
    current = FaultCase()
    for line in lines:
        w = line.split()
        if not w:
            continue
        if w[0] == 'case':
            current = FaultCase()
        elif w[0] == 'run':
            current.layout_seed = int(w[1])
            current.run_seed = int(w[2])
        elif w[0] == 'event':
            current.events.append(parse_event(w))
        elif w[0] == 'end':
            current.base = bases[len(cases)]
            cases.append(current)
    return cases


def opt(v):
    return '-' if v is None else str(v)


def activation_text(r):
    out = 'activation %s %s' % (r.event_id, r.status)
    if r.status == 'applied':
        out += ' at %d' % r.application_step
    if r.reason:
        out += ' | ' + r.reason
    return out


def score_text(event_id, s):
    return ('score %s activated %s at %d active %s end %s %d by %d opportunity %s revealing %s mismatch %s '
            'strict %s' % (event_id, rt.yes_no(s.activated), s.activation_step, rt.yes_no(s.active), s.end_state,
                           s.end_step, s.correcting_id, opt(s.opportunity_step), rt.yes_no(s.revealing),
                           opt(s.mismatch_step), opt(s.strict_step)))


class FaultPrinter:
    def __init__(self, events):
        self.events = events
        self.activations = [''] * len(events)
        self.scores = [''] * len(events)

    def changes(self, out, injector, scorer):
        for i, record in enumerate(injector.records):
            text = activation_text(record)
            if text != self.activations[i]:
                out.append(text)
                self.activations[i] = text
        for i, score in enumerate(scorer.scores):
            text = score_text(self.events[i].id, score)
            if text != self.scores[i]:
                out.append(text)
                self.scores[i] = text


def c0_step_lines(out, a, given, d):
    out.append('step %d' % a.step)
    out.append(rt.observation_line(given))
    out.append('decision ' + d.action)
    out.append('belief ' + rt.state_text(a.belief))
    out.append('plan %d searches %d' % (len(a.plan), a.searches))
    out.append('evaluations %d total %d' % (a.predictor.used_this_step(), a.predictor.used_total))


def trace(fc):
    c = fc.base
    single = c.agent != 'C0'
    a = rt.make_c1(c) if single else rt.make_c0(c)
    body = []
    printer = rt.C1Printer()
    faults = FaultPrinter(fc.events)

    def watch_decision(given, decision, injector, scorer):
        if single:
            printer.step(body, a, given, decision)
        else:
            c0_step_lines(body, a, given, decision)
        faults.changes(body, injector, scorer)

    def watch_final(given, injector, scorer):
        printer.final(body, a, given)
        faults.changes(body, injector, scorer)
    r, _, _ = rf.run_with_faults(c.world, a, c.settings, fc.events, True, watch_decision, watch_final)
    out = ['case ' + c.name] + body
    if single:
        out.append(rt.result_line(r.outcome, r.world_steps, r.decision_steps, a.safe_stops, len(a.mismatches),
                                  len(a.corrections), a.verified_count(), r.evaluations))
        safe_stops = a.safe_stops
    else:
        out.append(rt.result_line(r.outcome, r.world_steps, r.decision_steps, 0, 0, 0, 0, r.evaluations))
        safe_stops = 0
    out.append('csv run ' + rcsv.run_row(c.agent, fc.layout_seed, fc.run_seed, r, safe_stops))
    for e, act, score in zip(fc.events, r.activations, r.event_scores):
        out.append('csv event ' + rcsv.event_row(c.agent, e, act, score))
    out.append('observed %d' % r.observed_evaluations)
    out.append('end')
    return out
