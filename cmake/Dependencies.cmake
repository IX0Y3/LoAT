# ------------------------------------------------------------------------------
# Find external libraries required by the project.
# ------------------------------------------------------------------------------

message(STATUS "Searching external libraries")

find_library(ANTLR4 antlr4-runtime)
find_library(FAUDES faudes)
find_library(Z3 z3)
find_library(YICES yices)
find_library(POLY poly)
find_library(POLYXX polyxx)
find_library(CUDD cudd)
find_library(CVC5 cvc5)
find_library(CADICAL cadical)
find_library(SWINE swine-z3)
find_library(CLN cln)
find_library(GMP gmp)

# ------------------------------------------------------------------------------
# Output the found paths to the libraries for debugging purposes.
# ------------------------------------------------------------------------------
message(STATUS "antlr4: ${ANTLR4}")
message(STATUS "faudes: ${FAUDES}")
message(STATUS "z3: ${Z3}")
message(STATUS "yices: ${YICES}")
message(STATUS "poly: ${POLY}")
message(STATUS "polyxx: ${POLYXX}")
message(STATUS "cudd: ${CUDD}")
message(STATUS "cvc5: ${CVC5}")
message(STATUS "cadical: ${CADICAL}")
message(STATUS "swine: ${SWINE}")
message(STATUS "cln: ${CLN}")
message(STATUS "gmp: ${GMP}")

# ------------------------------------------------------------------------------
# Store all libraries in a list for easy linking in the main CMakeLists.txt.
# ------------------------------------------------------------------------------
set(EXTERNAL_LIBS
    ${ANTLR4}
    ${FAUDES}
    ${SWINE}
    ${Z3}
    ${CVC5}
    ${CADICAL}
    ${YICES}
    ${POLYXX}
    ${POLY}
    ${CUDD}
    ${CLN}
    ${GMP}
)
    