import os; os.system("clear")
import sys
arg = sys.argv

from z3 import *
sys.path.insert(1,".")
from pythonsrc import Game

def solveGame(g):
    solver = Solver()

    V = [Bool(f"S_{v}") for v in range(g.nvertices)]
    E = {(v, w): Bool(f"T_{v}_{w}") for v, w in zip(g.sources, g.targets)}
    X = {(v, p): Int(f"X_{v}_{p}") for v in range(g.nvertices) for p in set(g.colors) if p % 2 == 1}

    # First player vertices must be activated
    solver.add(V[0])

    # For every EVEN vertice, at least one outgoing edge must be activated
    for v in range(g.nvertices):
        if g.owners[v] == 0:
            solver.add(Or([V[v]==False]+[E[v, w] for w in g.targets if (v, w) in E]))

    # For every ODD vertice, each outgoing edge must be activated
    for v in range(g.nvertices):
        if g.owners[v] == 1:
            for w in g.targets:
                if (v, w) in E:
                    solver.add(Or([V[v]==False] +  [E[v, w]]))

    # For every activated edge, the source vertex must be activated
    for v in range(g.nvertices):
        for w in g.targets:
            if (v, w) in E:
                solver.add(Or([E[v, w]==False] + [V[v]]))

    # For every activated edge, the target vertex must be activated
    for v in range(g.nvertices):
        for w in g.targets:
            if (v, w) in E:
                solver.add(Or([E[v, w]==False] + [V[w]]))

    # Progress measure constraints
    for v, w in E:
        q = g.colors[w]
        constraints = [X[v, p] >= X[w, p] for p in set(g.colors) if p < q and p % 2 == 1 if (v, p) in X and (w, p) in X]
        if q % 2 == 0:
            solver.add(Implies(E[v, w], And(constraints)))
        else:
            solver.add(Implies(E[v, w], And(X[v, q] > X[w, q], And(constraints))))

    # Solve the formula
    if solver.check() == sat:
        print("SAT", end=" ")
    else:
        print("UNSAT", end=" ")

    print(solver.statistics().time)

g = Game('./data/game-jurdzinski-3-2.dzn',Game.FIRST0)

# levels  = int(arg[1])
# blocks  = int(arg[2])

# g = Game(Game.JURDZINSKI,levels,blocks,Game.FIRST0)
# g = Game(Game.RANDOM,10,Game.FIRST0)
# print(g)

solveGame(g)
