#pragma once

#include "rule.hpp"
#include "itsproblem.hpp"
#include "accelerationtechnique.hpp"
#include "recurrence.hpp"

class AccelerationViaQE: public AccelerationTechnique<IntTheory, BoolTheory> {

    using AcceleratorPair = typename AccelerationTechnique<IntTheory, BoolTheory>::AcceleratorPair;

public:

    AccelerationViaQE(
            const LinearRule &rule,
            const Recurrence::Result closed,
            ITSProblem &its,
            const Approx approx);

    AcceleratorPair computeRes() override;

};
