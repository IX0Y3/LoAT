# LoAT -- the Loop Acceleration Tool

LoAT (**Lo**op **A**cceleration **T**ool, formerly known as **Lo**wer Bounds **A**nalysis **T**ool) is a fully automated tool to analyze programs operating on integers.
Currently, it supports the inference of **lower bounds** on the worst-case runtime complexity and **non-termination proving** (in the branch [nonterm](https://github.com/aprove-developers/LoAT/tree/nonterm)).

LoAT has been inspired by [KoAT](<https://github.com/s-falke/kittel-koat/>) and uses a variation of ranking functions in combination with recurrence solving to deduce lower bounds.

To prove non-termination, it uses recurrent sets, a new loop acceleration technique based on a novel monotonicity criterion, and an SMT-based invariant inference technique.

The tool is based on the recurrence solver [PURRS](http://www.cs.unipr.it/purrs/) and the SMT solver [Z3](https://github.com/Z3Prover/z3/).

## Input Formats

To analyze programs with LoAT, they need to be represented as *Integer Transition Systems*.
It supports the most common formats for such systems.

### SMTLIB

LoAT can parse the [SMTLIB-format](https://www.microsoft.com/en-us/research/wp-content/uploads/2016/02/SMTPushdownPrograms.pdf) used in the category *Termination of Integer Transition Systems* at the annual [*Termination and Complexity Competition*](http://termination-portal.org/wiki/Termination_Competition).

### KoAT

LoAT also supports an extended version of [KoAT's input format](aprove.informatik.rwth-aachen.de/eval/IntegerComplexity/), which is also used in the category *Complexity of Integer Transition Systems* at the annual *Termination and Complexity Competition*.

In this extension, rules can be annotated with polynomial costs:
```
l1(A,B) -{A^2,A^2+B}> Com_2(l2(A,B),l3(A,B)) :|: B >= 0
```
Here, `A^2` and `A^2+B` are lower and upper bounds on the cost of the rule.
The upper bound is ignored by LoAT.
The lower bound has to be non-negative for every model of the transition's guard.

### T2

The branch [nonterm](https://github.com/aprove-developers/LoAT/tree/nonterm) comes with experimental support for the native input format of [T2](https://github.com/mmjb/T2).

## Publications

The techniques implemented in LoAT are described in the following publications (in chronological order):

* [Lower Runtime Bounds for Integer Programs](http://aprove.informatik.rwth-aachen.de/eval/integerLower/compl-paper.pdf)\
  F. Frohn, M. Naaf, J. Hensel, M. Brockschmidt, and J. Giesl\
  IJCAR '16
* [Proving Non-Termination via Loop Acceleration](https://arxiv.org/abs/1905.11187)\
  F. Frohn and J. Giesl\
  FMCAD '19, to appear
* Inferring Lower Runtime Bounds for Integer Programs\
  F. Frohn, M. Naaf, M. Brockschmidt, and J. Giesl\
  coming soon

## Awards

Since 2016, [AProVE](http://aprove.informatik.rwth-aachen.de/) is using LoAT as backend to prove lower bounds on the runtime complexity of integer transition systems.
In this constellation, AProVE and LoAT won the following awards:

* 1st place in the category *Complexity of Integer Transition Systems* at the [*Termination and Complexity Competition 2016*](https://termcomp.imn.htwk-leipzig.de/competitions/Y2016)
* 1st place in the category *Complexity of Integer Transition Systems* at the *Termination and Complexity Competition 2017* (unfortunately, the results are no longer available online)
* 1st place in the category *Complexity of Integer Transition Systems* at the [*Termination and Complexity Competition 2018*](http://group-mmm.org/termination/competitions/Y2018/)
* 1st place in the category *Complexity of Integer Transition Systems* at the [*Termination and Complexity Competition 2019*](http://group-mmm.org/termination/competitions/Y2019/)

## Build

Unfortunately, building LoAT is rather complex, so please consider using our [pre-compiled releases](https://github.com/aprove-developers/LoAT/releases).
If you need a different version, write an email to ffrohn [at] mpi-inf.mpg.de.

To compile LoAT, you will need the following libraries (and their dependencies, including CLN, NTL, and giac):

* [GiNaC](http://www.ginac.de)
* a custom version of [PURRS](https://github.com/aprove-developers/LoAT-purrs)
* [Z3](https://github.com/Z3Prover/z3)
* [boost](https://www.boost.org)

To install Z3, download and unpack the latest [Z3 release](https://github.com/Z3Prover/z3/releases) and add `/path/to/z3/bin` to your `PATH`.
After installing all dependencies, run:

```
mkdir build && cd build && cmake .. && make
```

If you experience any problems, contact ffrohn [at] mpi-inf.mpg.de.
