"""Reference for the Stage 4 rebuild, part 4: the CSV rows (plan STAGE_04, 3.11).

Reference contract (R10): it follows the original C++ (src/experiment/csv_output.cpp and
csv_output_event.cpp at 89ea9b6) field by field."""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import ref_faults as rf  # noqa: E402

RUN_HEADER = ('run_id,condition,layout_seed,run_seed,outcome,decision_steps,world_steps,safe_stops,'
              'justified_safe_stops,mismatches,mismatches_without_fault_cause,source_labels_correct,'
              'source_labels_scored,claims,false_alarms,wrong_accepted_corrections,evaluations,'
              'events_scheduled,events_activated')
EVENT_HEADER = ('event_id,condition,reasoner,layout_seed,run_seed,type,target,common_mode,cell,direction,'
                'order,action,rotation,scheduled_step,reported_as,activation_step,activation_delay,'
                'not_activated_reason,opportunity,opportunity_step,revealing_opportunity,mismatch_detected,'
                'mismatch_step,strictly_detected,strict_step,detection_delay,end_state,end_step,'
                'correction_status,verified_correction')


def bit(b):
    return '1' if b else '0'


def optional_step(v):
    return '' if v is None else str(v)


def run_row(condition, layout_seed, run_seed, r, safe_stops):
    activated = len([a for a in r.activations if a.status == 'applied'])
    s = r.score
    fields = ['%d-%d' % (layout_seed, run_seed), condition, str(layout_seed), str(run_seed), r.outcome,
              str(r.decision_steps), str(r.world_steps), str(safe_stops), str(s.justified_safe_stops),
              str(s.mismatches), str(s.mismatches_without_cause), str(s.labels_correct), str(s.labels_scored),
              str(s.claims), str(s.false_alarms), str(s.wrong_accepted), str(r.evaluations),
              str(len(r.activations)), str(activated)]
    return ','.join(fields)


END_TEXT = {'corrected': 'Corrected', 'overwritten': 'Overwritten without correction',
            'none': 'Active at end of run'}


def event_details(e):
    cell = '%d:%d' % e.cell if e.type in ('F1', 'F4') and e.has_cell else ''
    direction = e.direction if e.type == 'F1' else ''
    order = ' '.join(e.order) if e.type == 'F2' else ''
    action = e.action if e.type == 'F3' else ''
    rotation = e.rotation if e.type == 'F3' else ''
    return [cell, direction, order, action, rotation]


def event_row(condition, e, activation, score):
    fields = [e.id, condition, 'R', str(e.layout_seed), str(e.run_seed), e.type, e.target, bit(e.common)]
    fields += event_details(e)
    fields += [str(e.step), rf.reported_as(activation)]
    if activation.status != 'applied':
        reason = activation.reason if activation.status in ('not-applicable', 'pending') else ''
        fields += ['', '', reason] + [''] * 12
        return ','.join(fields)
    delay = '' if score.strict_step is None else str(score.strict_step - activation.application_step)
    status = ''
    if score.end_state == 'corrected' and score.correction_status is not None:
        status = score.correction_status
    fields += [str(activation.application_step), str(rf.activation_delay(activation)), '',
               bit(score.opportunity_step is not None), optional_step(score.opportunity_step),
               bit(score.revealing), bit(score.mismatch_step is not None), optional_step(score.mismatch_step),
               bit(score.strict_step is not None), optional_step(score.strict_step), delay,
               END_TEXT[score.end_state], '' if score.end_state == 'none' else str(score.end_step), status,
               bit(score.correction_status == 'VERIFIED')]
    return ','.join(fields)
