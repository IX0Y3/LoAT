#pragma once

#include "abmc_api.hpp"
#include "adcl_api.hpp"
#include "adclsat_api.hpp"

class LoAT
{
public:
    static std::unique_ptr<ABMC_API> create_abmc(ITSPtr its)
    {
        return std::make_unique<ABMC_API>(its);
    }

    static std::unique_ptr<ADCL_API> create_adcl(ITSPtr its, const std::function<void(const ITSCpxCex &)> &print_cpx_cex)
    {
        return std::make_unique<ADCL_API>(its, print_cpx_cex);
    }

    static std::unique_ptr<ADCLSAT_API> create_adclsat(ITSPtr its, const Config::TRPConfig &config)
    {
        return std::make_unique<ADCLSAT_API>(its, config);
    }
};