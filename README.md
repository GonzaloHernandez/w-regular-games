## About w-regular-games

This repository is created to analyse the behaviour of Omega-Regular Games in different contexts.

## Folder Structure

The workspace contains the following folders:

- automatons:  Graphical models created using Graphing
- data:        Input data files for MiniZinc
- model:       MiniZinc model files
- mzn-python:  Python code that interacts with MiniZinc
- sat-solvers: Alternative implementations using OR-Tools or PySAT
- python:      Direct Python implementations

## Virtual Environment

To compile and run code under mzn-python and sat-solvers, first set up a Python virtual environment:

1. Press Ctrl+Shift+P in your IDE and select:
   Python: Create Environment (Venv)

2. Install required dependencies:

   pip install minizinc  
   pip install python-sat  
   pip install ortools  

## Graphical View of Automatons

To view automatons, use the Graphing application:
https://github.com/GonzaloHernandez/graphing

## Chuffed Setup and NoOpponentCycle Compilation

1. Install Chuffed as a Library

Clone and install Chuffed (recommended branch: 0.12.1):

    git clone --branch 0.12.1 https://github.com/chuffed/chuffed.git
    cmake -B build -S . -DCMAKE_INSTALL_PREFIX=/home/<user>/.local
    cmake --build build --target install

2. Compile NoOpponentCycle

    mkdir build
    g++ -std=c++17 various/*.cpp experiments/ex3.cpp -o build/noc \
        -I/home/<user>/.local/include \
        -L/home/<user>/.local/lib \
        -lchuffed_fzn -lchuffed

3. Example Executions

    ./build/noc --help
    ./build/noc --dzn data/wiki.dzn --print-game
    ./build/noc --dzn data/wiki.dzn --noc-even --filter-eager --print-times
    ./build/noc --jurd 3 2 --noc-odd --export-gm game.gm --print-time
    ./build/noc --rand 10 5 2 4 --noc-odd --filter-memo --print-times
    ./build/noc --rand 20 10 1 5 --print-times --export-dimacs game.cnf
    sh ./resources/noc-parallel.sh --jurd 5000 10 --min --print-times --filter-eager

## SAT/SMT Solver Support

Install the following external solvers if you want to solve DIMACS output (game.cnf):

- CaDiCaL: https://github.com/arminbiere/cadical  
- Kissat: https://github.com/arminbiere/kissat  
- ZChaff: https://www.princeton.edu/~chaff/zchaff.html  
- PySAT: https://pysathq.github.io/  
- Z3: https://www.microsoft.com/en-us/research/project/z3-3/  
- OR-Tools: https://github.com/google/or-tools  

Example usage:

    cadical game.cnf
    kissat game.cnf
    zchaff game.cnf

## MiniZinc Support

Install MiniZinc and its Python bindings:

- https://www.minizinc.org/  
- https://python.minizinc.dev/en/latest/

To enable custom FlatZinc backend support:

1. From the chuffed-patch folder:

    - Copy game.h and noc.h into chuffed/support
    - Apply flatzinc-NOC.patch to the Chuffed repository

2. Rebuild Chuffed:

    cmake -B build -S . -DCMAKE_INSTALL_PREFIX=/home/<user>/.local
    cmake --build build --target install

3. Use chuffed-noc.msc as the solver configuration in MiniZinc

4. Example:

    minizinc model/cp-based-extended.mzn data/jurd-2-1.dzn --solver chuffed-patch/chuffed-noc.msc

5. See more examples in the model/ and mzn-python/ folders

## Notes

- The reduction-sat.mzn model is designed only from the EVEN player's perspective.
- MiniZinc solvers (drivers):
    0 = Gecode
    1 = Chuffed
    2 = cpsattlp
    3 = chuffed-noc (custom patched)

To use driver 3, ensure chuffed-noc.msc is configured (typically in ~/.minizinc/solvers).
