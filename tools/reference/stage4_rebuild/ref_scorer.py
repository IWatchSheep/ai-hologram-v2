"""Reference for the Stage 4 rebuild, part 3: the scorer and the shadow computations
(plan STAGE_04, 3.7-3.9).

Reference contract (R10): it follows the original C++ (src/experiment at 89ea9b6: scorer,
scorer_life, scorer_commit, scorer_observe, shadow) function by function, in the same order.
The scorer only reads the agent (ScoredView reads the agent's attributes at each use, as the
C++ view's pointers do); it never writes to it."""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 'stage3'))
import ref_parts as rp  # noqa: E402

rw = rp.rw


# ---------------- the read-only view (plan 4.2) ----------------
class ScoredView:
    """For C0 the corrections, mismatches and committed prediction are None."""

    def __init__(self, agent, single):
        self._agent = agent
        self._single = single

    belief = property(lambda self: self._agent.belief)
    model = property(lambda self: self._agent.model)
    not_dead = property(lambda self: self._agent.not_dead)
    dead_overrides = property(lambda self: self._agent.dead_overrides)
    corrections = property(lambda self: self._agent.corrections if self._single else None)
    mismatches = property(lambda self: self._agent.mismatches if self._single else None)
    committed = property(lambda self: self._agent.committed if self._single else None)


def view_of(agent):
    return ScoredView(agent, hasattr(agent, 'corrections'))


# ---------------- end states and scores ----------------
class EventScore:
    def __init__(self):
        self.activated = False
        self.activation_step = 0
        self.active = False
        self.end_state = 'none'          # 'none', 'corrected', 'overwritten'
        self.end_step = 0
        self.correcting_id = 0
        self.correction_status = None
        self.opportunity_step = None
        self.revealing = False
        self.mismatch_step = None
        self.strict_step = None


class RunScore:
    def __init__(self):
        self.claims = 0
        self.false_alarms = 0
        self.wrong_accepted = 0
        self.justified_safe_stops = 0
        self.mismatches = 0
        self.mismatches_without_cause = 0
        self.labels_correct = 0
        self.labels_scored = 0


def claim_value_is_true(record, true_layout, true_history, mismatches):
    kind = record.element.kind
    if kind == 'wall':
        return record.new_blocked == rw.is_wall(true_layout, record.element.cell)
    if kind == 'model':
        return True
    if kind == 'trap':
        m = rp.compute_dead_squares(true_layout, [], [], rp.Counts())
        cell = record.element.cell
        if not rw.is_inside(cell):
            return False
        return m.dead[cell[1]][cell[0]]
    if not mismatches:
        return False
    step = mismatches[-1].step
    if step < 2:
        return False
    index = step - 2
    if index >= len(true_history):
        return False
    return record.new_position == true_history[index].state.object


def element_of_event(event):
    if event.type == 'F1':
        return ('wall', event.cell)
    if event.type in ('F2', 'F5'):
        return ('object', None)
    if event.type == 'F3':
        return ('model', event.action)
    return ('trap', event.cell)


def same_element(event_element, element):
    kind, key = event_element
    if kind != element.kind:
        return False
    if kind in ('wall', 'trap'):
        return key == element.cell
    if kind == 'model':
        return key == element.action
    return True


def source_of(fault_type):
    return 'S1' if fault_type in ('F3', 'F4') else 'S2'


# ---------------- shadow computations (plan 3.9) ----------------
def predictions_differ(first_belief, first, second_belief, second, action):
    """first: (action, outcome, state); second: (state, outcome)."""
    if first[1] != second[1]:
        return True
    cells = rp.footprint_for_two(first_belief, first[1], second_belief, second[1], action)
    differ, diffs = rp.compare_footprint(first[2], first[1], second[0], second[1], cells)
    return differ or bool(diffs)


def shadow_input(event, state, view, true_layout, true_object):
    if event.type == 'F1':
        if not event.has_cell:
            return None
        if rw.is_wall(state, event.cell) == rw.is_wall(true_layout, event.cell):
            return None
        if not rw.is_inside(event.cell):
            return None
        shadow = state.copy()
        shadow.walls[event.cell[1]][event.cell[0]] = rw.is_wall(true_layout, event.cell)
        return shadow
    if event.type in ('F2', 'F5'):
        if view.belief is None:
            return None
        if view.belief.object == true_object:
            return None
        if state.object != view.belief.object:
            return None
        shadow = state.copy()
        shadow.object = true_object
        return shadow
    return None


def evaluation_effective(event, state, action, model_used, result, view, true_layout, true_object):
    if event.type == 'F4':
        return False
    if event.type == 'F3':
        if action != event.action:
            return False
        if view.model is None or view.model.is_restored(action):
            return False
        standard = rw.apply_action(state, action)
        return predictions_differ(state, result, state, standard, action)
    shadow = shadow_input(event, state, view, true_layout, true_object)
    if shadow is None:
        return False
    if not rw.is_valid(shadow):
        return True
    shadow_result = model_used(shadow, action)
    return predictions_differ(state, result, shadow, shadow_result, action)


def shadow_map(event, view, true_layout):
    if view.belief is None or view.not_dead is None or view.dead_overrides is None:
        return None
    counts = rp.Counts()
    if event.type == 'F1':
        if not event.has_cell or not rw.is_inside(event.cell):
            return None
        if rw.is_wall(view.belief, event.cell) == rw.is_wall(true_layout, event.cell):
            return None
        belief = view.belief.copy()
        belief.walls[event.cell[1]][event.cell[0]] = rw.is_wall(true_layout, event.cell)
        return rp.compute_dead_squares(belief, view.not_dead, view.dead_overrides, counts)
    if event.type == 'F4':
        overrides = list(view.not_dead)
        if event.cell in overrides:
            overrides.remove(event.cell)
        return rp.compute_dead_squares(view.belief, overrides, view.dead_overrides, counts)
    return None


# ---------------- the scorer ----------------
class Scorer:
    def __init__(self, events, true_layout, true_history):
        self.events = list(events)
        self.scores = [EventScore() for _ in self.events]
        self.true_layout = true_layout
        self.true_history = true_history
        self.run = RunScore()
        self.current_step = 0
        self.live_view = None
        self.observed_evaluations = 0
        self.seen_corrections = 0
        self.seen_mismatches = 0
        self.committed_causes = []

    # -- active now (scorer.cpp) --
    def active_now(self, view, i):
        event = self.events[i]
        if event.type == 'F1':
            return rw.is_wall(view.belief, event.cell) != rw.is_wall(self.true_layout, event.cell)
        if event.type in ('F2', 'F5'):
            if not self.true_history:
                return False
            return view.belief.object != self.true_history[-1].state.object
        if event.type == 'F3':
            return not view.model.is_restored(event.action)
        if view.dead_overrides is None:
            return True
        return event.cell not in view.dead_overrides

    def value_is_true(self, record, view):
        mismatches = view.mismatches if view.mismatches is not None else []
        return claim_value_is_true(record, self.true_layout, self.true_history, mismatches)

    def find_record(self, view, cid):
        if view.corrections is None:
            return None
        for record in view.corrections:
            if record.id == cid:
                return record
        return None

    # -- phase entry points --
    def end_phase_a(self, view, step):
        self.current_step = step
        self.end_phase(view)

    def after_activation(self, view, records, step):
        self.current_step = step
        for i in range(len(self.events)):
            if i >= len(records) or records[i].status != 'applied':
                continue
            score = self.scores[i]
            if score.activated:
                continue
            score.activated = True
            score.activation_step = step
            score.active = True
            if not self.active_now(view, i):
                score.active = False
                score.end_state = 'overwritten'
                score.end_step = step

    def end_final_check(self, view):
        self.end_phase(view)

    def finish(self, view):
        if view.corrections is None:
            return
        for score in self.scores:
            if score.end_state != 'corrected' or score.correcting_id == 0:
                continue
            for record in view.corrections:
                if record.id == score.correcting_id:
                    score.correction_status = record.status

    # -- the event life (scorer_life.cpp) --
    def end_phase(self, view):
        count = len(self.events)
        start_active = [self.scores[i].activated and self.scores[i].end_state == 'none' and self.scores[i].active
                        for i in range(count)]
        new_records = []
        if view.corrections is not None:
            for record in view.corrections[self.seen_corrections:]:
                new_records.append(record)
                self.run.claims += 1
                if not self.value_is_true(record, view):
                    self.run.wrong_accepted += 1
                found = False
                for e in range(count):
                    if not start_active[e]:
                        continue
                    if not same_element(element_of_event(self.events[e]), record.element):
                        continue
                    found = True
                    if self.scores[e].strict_step is None:
                        self.scores[e].strict_step = self.current_step
                if not found:
                    self.run.false_alarms += 1
            self.seen_corrections = len(view.corrections)
        if view.mismatches is not None:
            for record in view.mismatches[self.seen_mismatches:]:
                self.run.mismatches += 1
                causes = [i for i in self.committed_causes if start_active[i]]
                if not causes:
                    self.run.mismatches_without_cause += 1
                    continue
                self.run.labels_scored += 1
                correct = False
                for i in causes:
                    if record.suspected == source_of(self.events[i].type):
                        correct = True
                    if self.scores[i].mismatch_step is None:
                        self.scores[i].mismatch_step = record.step
                if correct:
                    self.run.labels_correct += 1
            self.seen_mismatches = len(view.mismatches)
        for i in range(count):
            score = self.scores[i]
            if not score.activated:
                continue
            now = self.active_now(view, i)
            if score.end_state == 'none' and score.active and not now:
                element = element_of_event(self.events[i])
                last_true = None
                for record in new_records:
                    if same_element(element, record.element) and self.value_is_true(record, view):
                        last_true = record
                if last_true is not None:
                    score.end_state = 'corrected'
                    score.correcting_id = last_true.id
                elif self.events[i].type in ('F3', 'F4'):
                    last_id = 0
                    for record in (view.corrections or []):
                        if same_element(element, record.element) and record.status != 'REJECTED':
                            last_id = record.id
                    score.end_state = 'corrected'
                    score.correcting_id = last_id
                else:
                    score.end_state = 'overwritten'
                score.end_step = self.current_step
                score.active = False
            elif score.end_state == 'corrected' and score.correcting_id != 0 and now:
                record = self.find_record(view, score.correcting_id)
                if record is not None and record.status == 'REJECTED':
                    score.end_state = 'none'
                    score.active = True
                    score.correcting_id = 0
                    score.end_step = 0
        self.committed_causes = []

    # -- the committed prediction (scorer_commit.cpp) --
    def committed_effective(self, i, pre, action, result_state, result_outcome, view):
        event = self.events[i]
        if event.type == 'F4':
            return False
        if event.type == 'F3':
            if action != event.action:
                return False
            if view.model is None or view.model.is_restored(action):
                return False
            standard_state, standard_outcome = rw.apply_action(pre, action)
            return standard_outcome != result_outcome or not standard_state.same(result_state)
        shadow = pre.copy()
        if event.type == 'F1':
            true_wall = rw.is_wall(self.true_layout, event.cell)
            if rw.is_wall(pre, event.cell) == true_wall:
                return False
            if not rw.is_inside(event.cell):
                return False
            shadow.walls[event.cell[1]][event.cell[0]] = true_wall
        else:
            if view.belief is None or not self.true_history:
                return False
            true_object = self.true_history[-1].state.object
            if view.belief.object == true_object:
                return False
            if pre.object != view.belief.object:
                return False
            shadow.object = true_object
        if not rw.is_valid(shadow):
            return True
        if view.model is None:
            return False
        shadow_state, shadow_outcome = view.model.current()(shadow, action)
        if shadow_outcome != result_outcome:
            return True
        cells = rp.footprint_for_two(pre, result_outcome, shadow, shadow_outcome, action)
        differ, diffs = rp.compare_footprint(result_state, result_outcome, shadow_state, shadow_outcome, cells)
        return differ or bool(diffs)

    def after_decide(self, view, decision):
        self.end_phase(view)
        if not decision.is_action:
            if any(s.activated and s.end_state == 'none' and s.active for s in self.scores):
                self.run.justified_safe_stops += 1
            return
        if view.committed is None or not view.committed.has:
            return
        pre = view.committed.belief_before
        action, outcome, state = view.committed.prediction
        for i, score in enumerate(self.scores):
            if not score.activated or score.end_state != 'none' or not score.active:
                continue
            if not self.committed_effective(i, pre, action, state, outcome, view):
                continue
            self.committed_causes.append(i)
            score.revealing = True
            if score.opportunity_step is None:
                score.opportunity_step = self.current_step

    # -- the watch-only observer callbacks (scorer_observe.cpp) --
    def attach_view(self, view):
        self.live_view = view

    def awaiting(self, score):
        return (score.activated and score.end_state == 'none' and score.active
                and score.opportunity_step is None)

    def on_evaluation(self, state, action, variant_model, result):
        self.observed_evaluations += 1
        view = self.live_view
        if view is None or view.model is None or not self.true_history:
            return
        true_object = self.true_history[-1].state.object
        model_used = variant_model if variant_model is not None else view.model.current()
        for i, event in enumerate(self.events):
            if not self.awaiting(self.scores[i]):
                continue
            if evaluation_effective(event, state, action, model_used, result, view, self.true_layout,
                                    true_object):
                self.scores[i].opportunity_step = self.current_step

    def on_dead_read(self, cell, value):
        view = self.live_view
        if view is None or not rw.is_inside(cell):
            return
        for i, event in enumerate(self.events):
            if not self.awaiting(self.scores[i]) or event.type not in ('F1', 'F4'):
                continue
            shadow = shadow_map(event, view, self.true_layout)
            if shadow is None:
                continue
            if shadow.dead[cell[1]][cell[0]] != value:
                self.scores[i].opportunity_step = self.current_step

    def on_heuristic(self, state, value):
        view = self.live_view
        if view is None or view.belief is None:
            return
        counts = rp.Counts()
        for i, event in enumerate(self.events):
            if not self.awaiting(self.scores[i]):
                continue
            if event.type in ('F1', 'F4'):
                shadow = shadow_map(event, view, self.true_layout)
                if shadow is not None and rp.heuristic(state, shadow, counts) != value:
                    self.scores[i].opportunity_step = self.current_step
                continue
            if event.type not in ('F2', 'F5') or not self.true_history:
                continue
            true_object = self.true_history[-1].state.object
            shadow = shadow_input(event, state, view, self.true_layout, true_object)
            if shadow is None:
                continue
            if not rw.is_valid(shadow):
                self.scores[i].opportunity_step = self.current_step
                continue
            if view.not_dead is None or view.dead_overrides is None:
                continue
            current = rp.compute_dead_squares(view.belief, view.not_dead, view.dead_overrides, counts)
            if rp.heuristic(shadow, current, counts) != value:
                self.scores[i].opportunity_step = self.current_step
