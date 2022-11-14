#include "accelerationfactory.hpp"
#include "accelerationviacalculus.hpp"
#include "accelerationviaqe.hpp"

using AT = std::unique_ptr<AccelerationTechnique<IntTheory, BoolTheory>>;

AT AccelerationFactory::get(
        const LinearRule &rule,
        option<Recurrence::Result> closed,
        ITSProblem &its,
        const Approx approx = UnderApprox) {
//    if (closed && rule.getGuard()->isPoly() && closed->update.isPoly()) {
//        return AT(new AccelerationViaQE(rule, *closed, its));
//    } else {
        return AT(new AccelerationViaCalculus(rule, closed, its, approx));
//    }
}
