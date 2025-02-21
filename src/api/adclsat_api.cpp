#include "adclsat_api.hpp"

ADCLSAT_API::ADCLSAT_API(const ITSPtr its, const Config::TRPConfig &config)
    : adcl(its, config) {}

std::optional<SmtResult>
ADCLSAT_API::do_step()
{
    return adcl.do_step();
}

ITSModel ADCLSAT_API::get_model()
{
    return adcl.get_model();
}