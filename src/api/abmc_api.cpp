#include "abmc_api.hpp"

ABMC_API::ABMC_API(ITSPtr its) : abmc(its) {}

std::optional<SmtResult> ABMC_API::do_step()
{
    return abmc.do_step();
}

ITSModel ABMC_API::get_model()
{
    return abmc.get_model();
}

ITSSafetyCex ABMC_API::get_cex()
{
    return abmc.get_cex();
}