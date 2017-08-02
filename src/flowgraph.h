/*  This file is part of LoAT.
 *  Copyright (c) 2015-2016 Matthias Naaf, RWTH Aachen University, Germany
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

#ifndef FLOWGRAPH_H
#define FLOWGRAPH_H

#include "global.h"

#include "graph.h"
#include "itrs.h"
#include "expression.h"
#include <list>
#include <string>
#include <z3++.h>


/**
 * Represents one transition in the graph with the given target, guards and updates
 */
struct Transition {
    Transition() : cost(1) {}
    GuardList guard;
    UpdateMap update;
    Expression cost;
};

std::ostream& operator<<(std::ostream &, const Transition &);



/**
 * Represents the final runtime complexity result,
 * including the final cost and guard
 */
struct RuntimeResult {
    Expression bound;
    GuardList guard;
    Complexity cpx;
    bool reducedCpx;
    RuntimeResult() : bound(0), cpx(Expression::ComplexNone), reducedCpx(false) {}
};


/**
 * Flow graph for an ITRS.
 * This class implements the main logic of chaining and metering.
 */
class FlowGraph : private Graph<Transition> {
public:
    /**
     * Creates the flow graph for the given itrs
     */
    FlowGraph(ITRSProblem &itrs);

    /**
     * Prints the graph in a readable but ugly format for debugging
     * @param s the output stream to print to (e.g. cout)
     */
    void print(std::ostream &s) const;

    /**
     * Print the graph in a more readable format suitable for the proof output
     */
    void printForProof() const;

    /**
     * Print the graph as dot outpus
     * @param s the output stream to print to (the dotfile)
     * @param step the number of the subgraph (should increase for every call)
     * @param desc a description of the current subgraph
     */
    void printDot(std::ostream &s, int step, const std::string &desc) const;
    void printDotText(std::ostream &s, int step, const std::string &desc) const;

    /**
     * Print the graph in the T2 format, to allow conversion from koat -> T2.
     * @param s the output stream to be used
     */
    void printT2(std::ostream &s) const;

    /**
     * Returns true if there are no (reachable) transitions from the initial location
     */
    bool isEmpty() const;

    //calls Preprocess::...)
    /**
     * Performs extensive preprocessing to simplify the graph (i.e. remove unreachable nodes, simplify guards)
     * @note this is a slow operation and should be used rarely (e.g. only once before the processing begins)
     * @return true iff the graph was modified
     */
    bool simplifyTransitions();

    /**
     * Checks initial transitions for satisfiability, removes unsat transitions.
     * @return true iff the graph was modified
     */
    bool reduceInitialTransitions();

    /**
     * Apply simple chaining (i.e. only linear paths)
     * @return true iff the graph was modified
     */
    bool chainLinear();

    /**
     * Apply branched chaining (i.e. the eliminated node can have multiple outgoing edges)
     * @note this is quite powerful, but often creates many branches. Consider pruning afterwards
     * @return true iff the graph was modified
     */
    bool chainBranches();

    /**
     * Replaces all selfloops with linear transitions by searching for metering functions and iterated costs/updates.
     * Also handles nesting and chaining of parallel selfoops (where possible).
     * @return true iff the graph was modified (which is always the case if any selfloops were present)
     */
    bool removeSelfloops();

    /**
     * Tries to identify and remove duplicate transitions
     * @param trans list of transitions that are checked
     * @note does not catch all duplicates, as this is a purely syntactical check (no z3 calls)
     * @return true iff the graph was modified (i.e. a duplicate got deleted)
     */
    bool removeDuplicateTransitions(const std::vector<TransIndex> &trans);

    /**
     * Reduces the number of parallel transitions by applying some greedy heuristic to find the "best" transitions
     * @note also removes unreachable nodes and irrelevant const transitions using removeConstLeafsAndUnreachable()
     * @return true iff the graph was modified (transitions were deleted)
     */
    bool pruneTransitions();

    /**
     * Returns true iff all paths have a length of at most 1
     */
    bool isFullyChained() const;

    /**
     * For a fully chained graph problem, this calculates the maximum runtime complexity (using the infinity check)
     */
    RuntimeResult getMaxRuntime();

    /**
     * In case of a timeout (when the graph is not fully chained), this tries to find a good partial result at least
     */
    RuntimeResult getMaxPartialResult();

private:
    /**
     * Adds the given rule to this graph, calculating the required update
     */
    void addRule(const Rule &rule);

    /**
     * Adds a new node to the graph, returns the index of the created node
     */
    NodeIndex addNode();

    /**
     * Chains transition followTrans into trans
     * @param trans the first transition, will be modified
     * @param followTrans the second transition. Must follow trans!
     * @note it is valid for trans and followTrans to point to the same data
     * @note does only affect trans, the internal transitions are *not* modified
     * @return true iff contraction was performed, false if aborted as result was not SATable
     */
    bool chainTransitionData(Transition &trans, const Transition &followTrans) const;

    /**
     * Internal function for chainLinear
     * @return true iff the graph was modified
     */
    bool chainLinearPaths(NodeIndex node, std::set<NodeIndex> &visited);

    /**
     * Internal function for chainBranches
     * @return true iff the graph was modified
     */
    bool chainBranchedPaths(NodeIndex node, std::set<NodeIndex> &visited);

    /**
     * Helper function that checks with a very simple heuristic if the transitions might be nested loops
     * (this is done to avoid too many nesting attemts, as finding a metering function takes some time).
     */
    bool canNest(const Transition &inner, const Transition &outer) const;

    /**
     * Replaces all selfloops of the given location with linear transitions by searching for metering functions and iterated costs/updates.
     * Also handles nesting and chaining of parallel selfoops (where possible).
     * @return true iff the graph was modified (which is always the case if any selfloops were present)
     */
    bool removeSelfloopsFrom(NodeIndex node);

    /**
     * A simple syntactic comparision. If it returns true, a and b are guaranteed to be equal.
     * @note as this is a syntactic check, false has no guaranteed meaning
     */
    bool compareTransitions(TransIndex a, TransIndex b) const;

    /**
     * Removes all unreachable nodes and transitions to leaves with constant cost, as they have no impact on the runtime
     * @return true iff the graph was modified
     */
    bool removeConstLeafsAndUnreachable();

    /**
     * This removes all subgraphs that have only constant costs.
     * @note this will also remove selfloops and is only meant to be used after a soft timeout
     * @return true iff the graph was modified
     */
    bool removeIrrelevantTransitions(NodeIndex curr, std::set<NodeIndex> &visited);


/**********************************************************/

    // New functions developed in 2017

    // Z3 <-> GiNaC
    z3::expr ginacToZ3(GiNaC:: ex ex, z3::context &c);
    z3::expr ginacToZ3(GuardList guard, z3::context &c, std::set<VariableIndex> protectedFreeVariables);
    GiNaC::ex read_from(std::list<std::string> &tokens);
    GiNaC::ex z3ToGinac(z3::expr expr);
    GuardList z3ToGuard(z3::expr expr);

    // Guess and Check
    bool tryInstantiate(NodeIndex node);

private:
    NodeIndex initial;
    std::set<NodeIndex> nodes;
    NodeIndex nextNode;

    ITRSProblem &itrs;
};

#endif // FLOWGRAPH_H
