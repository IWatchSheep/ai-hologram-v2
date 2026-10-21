// Purpose: Declares the model catalogue of the gate's case
// file: the six model specs a case may name (plan 3.14).
// Introduced in: Stage 3g (rebuild)
// Architecture: A case's model words either name the standard
// rule (no base model is given to the agent) or one wrong
// action model, hand-written exactly as build_model in
// ref_trace.py. The catalogue's names live in one table in
// trace_models.cpp, which both the word check and the builder
// use, so there is only one list of them (rule R4).

#ifndef AI_HOLOGRAM_TESTS_TRACE_MODELS_H
#define AI_HOLOGRAM_TESTS_TRACE_MODELS_H

#include <optional>
#include <string>
#include <vector>

#include "model/world_model.h"

namespace aih {

// True when spec (the words after "model") is one exact
// catalogue form: standard, or a known name with its actions.
bool traceModelSpecOk(const std::vector<std::string>& spec);

// The base model for a valid spec, or nothing for standard,
// which means the agent is built with no base model given.
// Callers pass only specs traceModelSpecOk accepts.
std::optional<ActionModel> traceBaseModel(const std::vector<std::string>& spec);

}  // namespace aih

#endif  // AI_HOLOGRAM_TESTS_TRACE_MODELS_H
