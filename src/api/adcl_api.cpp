#include "adcl_api.hpp"

ADCL_API::ADCL_API(ITSPtr its, const std::function<void(const ITSCpxCex &)> &print_cpx_cex)
    : adcl(its, print_cpx_cex) {}

SmtResult ADCL_API::analyze()
{
    return adcl.analyze();
}

ITSSafetyCex ADCL_API::get_cex()
{
    return adcl.get_cex();
}