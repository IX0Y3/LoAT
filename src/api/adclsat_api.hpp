#pragma once

#include "adclsat.hpp"

class ADCLSAT_API
{
private:
    ADCLSat adcl;

public:
    explicit ADCLSAT_API(const ITSPtr its, const Config::TRPConfig &config);

    std::optional<SmtResult> do_step();
    ITSModel get_model();
};