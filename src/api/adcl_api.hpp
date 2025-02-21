#pragma once

#include "../adcl/adcl.hpp"

class ADCL_API
{
private:
    adcl::ADCL adcl;

public:
    explicit ADCL_API(ITSPtr its, const std::function<void(const ITSCpxCex &)> &print_cpx_cex);

    SmtResult analyze();
    ITSSafetyCex get_cex();
};