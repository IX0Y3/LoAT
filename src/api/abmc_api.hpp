#pragma once

#include "../abmc/abmc.hpp"

class ABMC_API
{
private:
    ABMC abmc;

public:
    explicit ABMC_API(ITSPtr its);

    std::optional<SmtResult> do_step();
    ITSModel get_model();
    ITSSafetyCex get_cex();
};