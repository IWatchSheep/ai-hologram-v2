"""Reference for the Stage 4 rebuild, part 2: applying faults, the activation records and the
run with faults (plan STAGE_04, 3.4-3.6).

Reference contract (R10): the fault application and the injector follow the original C++
(src/experiment/fault_apply.cpp and fault_injector.cpp at 89ea9b6) check by check. The run with
faults follows the rebuild's design (plan 3.6, D-07x): the Stage 3 run loop itself, with three
watch-only watchers (onObservation, afterDecision, afterFinalCheck), instead of a second copy of
the loop. It gives exactly the original runWithFaults results (the golden fault traces)."""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.path.insert(0, os.path.join(HERE, '..', 'stage3'))
import ref_agents as ra  # noqa: E402
import ref_parts as rp  # noqa: E402
import ref_scorer as rs  # noqa: E402
import ref_verify as rc  # noqa: E402

rw = rp.rw
ORDER = ['North', 'East', 'South', 'West']
LEFT = {'North': 'West', 'East': 'North', 'South': 'East', 'West': 'South'}
RIGHT = {'North': 'East', 'East': 'South', 'South': 'West', 'West': 'North'}


# ---------------- 3.4 the F3 faulty model and applying faults ----------------
def rotated(action, rotation):
    return LEFT[action] if rotation == 'left' else RIGHT[action]


def make_faulty_model(faulted):
    """faulted: {action: rotation}, copied now (fault_injector.cpp makeFaultyModel)."""
    table = dict(faulted)

    def model(state, action):
        standard_state, standard_outcome = rw.apply_action(state, action)
        if action not in table or standard_outcome != 'Pushed':
            return standard_state, standard_outcome
        agent_cell = rw.moved(state.agent, action)
        object_cell = rw.moved(agent_cell, rotated(action, table[action]))
        if rw.is_wall(state, object_cell):
            return state, 'Blocked'
        nxt = state.copy()
        nxt.agent = agent_cell
        nxt.object = object_cell
        return nxt, 'Pushed'
    return model


def previous_object(true_history):
    for i in range(len(true_history) - 1, 0, -1):
        if true_history[i].state.object != true_history[i - 1].state.object:
            return true_history[i - 1].state.object
    return None


def try_apply_fault(event, agent, faulted, true_history):
    """Returns (applied, reason). agent is the reasoner (the parts: belief, model, "not dead"
    overrides, predictor)."""
    b = agent.belief
    if event.type == 'F1':
        if not event.has_cell:
            return False, 'no cell of the required kind'
        if event.direction == 'free->blocked':
            if event.cell == b.agent:
                return False, 'the wall would be under the believed agent'
            if event.cell == b.object:
                return False, 'the wall would be under the believed object'
        if rw.is_inside(event.cell):
            b.walls[event.cell[1]][event.cell[0]] = not b.walls[event.cell[1]][event.cell[0]]
    elif event.type == 'F2':
        for direction in event.order:
            cell = rw.moved(b.object, direction)
            if rw.is_wall(b, cell) or cell == b.agent:
                continue
            b.object = cell
            break
        else:
            return False, 'no direction passes the test'
    elif event.type == 'F3':
        faulted[event.action] = event.rotation
        agent.model.set_base(make_faulty_model(faulted))
        agent.model.unrestore(event.action)
    elif event.type == 'F4':
        if not event.has_cell:
            return False, 'the layout has no true dead square'
        agent.not_dead.append(event.cell)
    else:
        previous = previous_object(true_history)
        if previous is None:
            return False, 'the object has not moved yet'
        if previous == b.agent:
            return False, 'the previous square is the believed agent square'
        if rw.is_wall(b, previous):
            return False, 'the previous square is believed to be a wall'
        b.object = previous
    agent.predictor.clear_cache()
    return True, ''


# ---------------- 3.5 activation records ----------------
class ActivationRecord:
    def __init__(self, event):
        self.event_id = event.id
        self.type = event.type
        self.scheduled_step = event.step
        self.status = 'scheduled'          # 'scheduled', 'applied', 'not-applicable', 'pending'
        self.application_step = 0
        self.reason = ''


def activation_delay(record):
    if record.status != 'applied':
        return 0
    return record.application_step - record.scheduled_step


def reported_as(record):
    if record.status == 'applied':
        if record.application_step == record.scheduled_step:
            return 'Activated at its scheduled step'
        return 'Activated after deferral'
    if record.status == 'scheduled':
        return 'Not activated: run ended before the scheduled step'
    if record.status == 'not-applicable':
        return 'Not activated: not applicable at the scheduled step'
    return 'Not activated: deferred, never applicable'


class FaultInjector:
    def __init__(self, events, true_history):
        self.events = list(events)
        self.records = [ActivationRecord(e) for e in self.events]
        self.true_history = true_history
        self.faulted = {}

    def at_activation_point(self, agent, step):
        for event, record in zip(self.events, self.records):
            should_try = ((record.status == 'scheduled' and record.scheduled_step == step)
                          or (record.status == 'pending' and step > record.scheduled_step))
            if not should_try:
                continue
            history = self.true_history if self.true_history is not None else []
            applied, reason = try_apply_fault(event, agent, self.faulted, history)
            if applied:
                record.status = 'applied'
                record.application_step = step
                record.reason = ''
            else:
                record.reason = reason
                record.status = 'pending' if event.type == 'F5' else 'not-applicable'


# ---------------- 3.6 the run loop with watchers, and the run with faults ----------------
def run_c1_watched(world, agent, settings, on_observation=None, after_decision=None, after_final=None):
    """The Stage 3 run loop (ref_agents.run_c1) with the rebuild's onObservation watcher: it is
    called with every observation just before the agent receives it (decide or the final check)."""
    env = rw.Environment(world)
    if world.object == world.destination:
        return 'Success', 0
    current = rc.Observation(1, world)
    world_steps = 0
    while True:
        if on_observation is not None:
            on_observation(agent, current)
        decision = agent.decide(current)
        if after_decision is not None:
            after_decision(agent, current, decision)
        if not decision.is_action:
            if agent.safe_stops >= settings.safe_stop_limit:
                return 'Stopped -- unresolved', world_steps
            current = rc.Observation(agent.step + 1, current.state)
            continue
        state, outcome = env.step(decision.action)
        world_steps += 1
        nxt = rc.Observation(agent.step + 1, env.state, decision.action, outcome)
        reached = env.state.object == env.state.destination
        if reached or world_steps >= settings.max_world_steps:
            if on_observation is not None:
                on_observation(agent, nxt)
            agent.final_check(nxt)
            if after_final is not None:
                after_final(agent, nxt)
            return ('Success' if reached else 'Step limit'), world_steps
        current = nxt


def run_c0_watched(world, agent, settings, on_observation=None, after_decision=None):
    env = rw.Environment(world)
    if world.object == world.destination:
        return 'Success', 0
    current = rc.Observation(1, world)
    world_steps = 0
    while True:
        if on_observation is not None:
            on_observation(agent, current)
        decision = agent.decide(current)
        if after_decision is not None:
            after_decision(agent, current, decision)
        state, outcome = env.step(decision.action)
        world_steps += 1
        nxt = rc.Observation(agent.step + 1, env.state, decision.action, outcome)
        if env.state.object == env.state.destination:
            return 'Success', world_steps
        if world_steps >= settings.max_world_steps:
            return 'Step limit', world_steps
        current = nxt


class FaultRunResult:
    def __init__(self):
        self.outcome = ''
        self.world_steps = 0
        self.decision_steps = 0
        self.lines = []
        self.evaluations = 0
        self.activations = []
        self.event_scores = []
        self.score = None
        self.observed_evaluations = 0


class ScorerObserver:
    """The watch-only observer (IE-1 H3, H4) that only calls the scorer. The Stage 3 reference
    predictor reports an own-model evaluation with False; it is passed on as None, as the C++
    passes a null model."""

    def __init__(self, scorer):
        def on_evaluation(state, action, model, prediction):
            scorer.on_evaluation(state, action, model if callable(model) else None, prediction)
        self.on_evaluation = on_evaluation
        self.on_dead_read = scorer.on_dead_read
        self.on_heuristic = scorer.on_heuristic


def report_variant_models(predictor):
    """The Stage 3 reference predictor reports only that a variant model was used (True); the
    C++ H3 passes the model itself (computation_observer.h). For the run with faults this
    predictor reports the model used, as the C++ does. Nothing else changes."""
    def predict_variant(state, action, model):
        used = model if model is not None else rp.standard_rule
        result = predictor._evaluate(state, action, used)
        if result is None:
            return None
        predictor.used_total += 1
        if predictor.observer is not None and predictor.observer.on_evaluation is not None:
            predictor.observer.on_evaluation(state, action, used, result)
        return result
    predictor.predict_variant = predict_variant


def run_with_faults(world, agent, settings, events, observe=True, watch_decision=None, watch_final=None):
    """watch_decision and watch_final are extra watch-only watchers, called after the scorer's
    (the trace printer of the gate uses them)."""
    single = hasattr(agent, 'corrections')
    true_history = []
    injector = FaultInjector(events, true_history)
    scorer = rs.Scorer(events, world, true_history)
    view = rs.view_of(agent)
    if observe:
        scorer.attach_view(view)
        observer = ScorerObserver(scorer)
        report_variant_models(agent.predictor)
        agent.predictor.observer = observer
        agent.dead_cache.observer = observer

    def hook(step):
        scorer.end_phase_a(view, step)
        injector.at_activation_point(agent, step)
        scorer.after_activation(view, injector.records, step)
    agent.activation_hook = hook

    def on_observation(_agent, observation):
        true_history.append(observation)

    def after_decision(_agent, observation, decision):
        scorer.after_decide(view, decision)
        if watch_decision is not None:
            watch_decision(observation, decision, injector, scorer)

    def after_final(_agent, observation):
        scorer.end_final_check(view)
        if watch_final is not None:
            watch_final(observation, injector, scorer)

    if single:
        outcome, steps = run_c1_watched(world, agent, settings, on_observation, after_decision, after_final)
    else:
        outcome, steps = run_c0_watched(world, agent, settings, on_observation, after_decision)
    agent.activation_hook = None
    agent.predictor.observer = None
    agent.dead_cache.observer = None
    agent.dead_cache.map.observer = None
    scorer.finish(view)
    r = FaultRunResult()
    r.outcome = outcome
    r.world_steps = steps
    r.decision_steps = agent.step
    r.evaluations = agent.predictor.used_total
    if single:
        r.lines = agent.log + ra.result_lines(outcome, steps, agent.safe_stops, len(agent.mismatches),
                                              len(agent.corrections), agent.verified_count(), r.evaluations)
    else:
        r.lines = ra.result_lines(outcome, steps, 0, 0, 0, 0, r.evaluations)
    r.activations = injector.records
    r.event_scores = scorer.scores
    r.score = scorer.run
    r.observed_evaluations = scorer.observed_evaluations
    return r, injector, scorer
