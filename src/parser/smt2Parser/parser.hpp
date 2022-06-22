/*  This file is part of LoAT.
 *  Copyright (c) 2019 Florian Frohn
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses>.
 */

#ifndef LOAT_SEXPRESSION_PARSER_H
#define LOAT_SEXPRESSION_PARSER_H


#include "../../its/itsproblem.hpp"
#include "sexpresso/sexpresso.hpp"

namespace sexpressionparser {

    class Parser {

    public:
        static ITSProblem loadFromFile(const std::string &filename);

    private:
        void run(const std::string &filename);

        void parseCond(sexpresso::Sexp &sexp, Guard &guard);

        Rel parseConstraint(sexpresso::Sexp &sexp, bool negate);

        Expr parseExpression(sexpresso::Sexp &sexp);

        std::vector<std::string> preVars;
        std::vector<std::string> postVars;
        std::map<std::string, LocationIdx > locations;
        std::map<std::string, Var> vars;
        ITSProblem res;

    };

}

#endif //LOAT_SEXPRESSION_PARSER_H
