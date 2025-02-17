import os; os.system("clear")
import sys,math
from pysat.solvers import Solver
from pysat.formula import CNF
from pysat.formula import IDPool
sys.path.insert(1,".")
from pythonsrc import Game

pool  = IDPool()


# ========================================================================================

def solveGame(g):

    def calculate_nbits(n) :
        return math.ceil(math.log2(sum(1 for x in range(g.nvertices) if g.colors[x] == n) + 1))

    # ----------------------------------------------------------------------------------------

    def calculate_total_nbits(n) :
        nbits = calculate_nbits(n)
        return nbits + nbits + (nbits-1) + (nbits-1) # v + c + e + s

    # ----------------------------------------------------------------------------------------

    def lexcomp_le(w, v, p, nbits) :
        cnf = []
        n = nbits-1
        c = nbits*2-1
        e = nbits*2+(nbits-1)
        s = nbits*2+(nbits-1)*2
        
        cnf.append([-X[v, p, c], -X[w, p, n], X[v, p, n]])
        cnf.append([+X[v, p, c], +X[w, p, n]])
        cnf.append([+X[v, p, c], -X[v, p, n]])
        
        for i in range(1, nbits):
            cnf.append([-X[v, p, e-i], -X[w, p, n-i], +X[v, p, n-i]])
            cnf.append([-X[v, p, e-i], +X[w, p, n-i], -X[v, p, n-i]])
            cnf.append([+X[v, p, e-i], +X[w, p, n-i], +X[v, p, n-i]])
            cnf.append([+X[v, p, e-i], -X[w, p, n-i], -X[v, p, n-i]])

            cnf.append([-X[v, p, s-i], -X[w, p, n-i]])
            cnf.append([-X[v, p, s-i], +X[v, p, n-i]])
            cnf.append([+X[v, p, s-i], +X[w, p, n-i], -X[v, p, n-i]])

            cnf.append([-X[v, p, c-i], +X[v, p, s-i], +X[v, p, e-i]])
            cnf.append([-X[v, p, c-i], +X[v, p, s-i], +X[v, p, c-(i-1)]])

            cnf.append([+X[v, p, c-i], -X[v, p, s-i]])
            cnf.append([+X[v, p, c-i], -X[v, p, e-i], -X[v, p, c-(i-1)]])
        
        cnf.append([X[v, p, nbits]]) # final clause encoding the fact that a <= b

        return cnf

    # ----------------------------------------------------------------------------------------

    def lexcomp_sl(w, v, p, nbits) :
        cnf = []
        n = nbits-1
        c = nbits*2-1
        e = nbits*2+(nbits-1)
        s = nbits*2+(nbits-1)*2
        
        cnf.append([+X[w, p, c], -X[v, p, n], +X[v, p, c]])
        cnf.append([-X[v, p, c], -X[w, p, n]])
        cnf.append([-X[v, p, c], +X[v, p, n]])
        
        for i in range(1, nbits):
            cnf.append([-X[v, p, e-i], -X[w, p, n-i], +X[v, p, n-i]])
            cnf.append([-X[v, p, e-i], +X[w, p, n-i], -X[v, p, n-i]])
            cnf.append([+X[v, p, e-i], +X[w, p, n-i], +X[v, p, n-i]])
            cnf.append([+X[v, p, e-i], -X[w, p, n-i], -X[v, p, n-i]])

            cnf.append([-X[v, p, s-i], -X[w, p, n-i]])
            cnf.append([-X[v, p, s-i], +X[v, p, n-i]])
            cnf.append([+X[v, p, s-i], +X[w, p, n-i], -X[v, p, n-i]])

            cnf.append([-X[v, p, c-i], +X[v, p, s-i], +X[v, p, e-i]])
            cnf.append([-X[v, p, c-i], +X[v, p, s-i], +X[v, p, c-(i-1)]])

            cnf.append([+X[v, p, c-i], -X[v, p, s-i]])
            cnf.append([+X[v, p, c-i], -X[v, p, e-i], -X[v, p, c-(i-1)]])
        
        cnf.append([X[v, p, nbits]]) # final clause encoding the fact that a <= b

        return cnf

    # ----------------------------------------------------------------------------------------

    cnf = CNF()

    # Map variables to integer literals
    V = {v: pool.id() for v in range(g.nvertices)}
    E = {(v, w): pool.id() for i, (v, w) in enumerate(zip(g.sources, g.targets))}
    X = {
        (v, p, i): pool.id()
        for v in range(g.nvertices)
        for p in set(g.colors) if p % 2 == 1
        for i in range(calculate_total_nbits(p))
    }
    # First player vertex must be activated
    cnf.append([V[0]])

    # For every EVEN vertex, at least one outgoing edge must be activated
    for v in range(g.nvertices):
        if g.owners[v] == 0:
            cnf.append([-V[v]] + [E[v, w] for w in g.targets if (v, w) in E])

    # For every ODD vertex, each outgoing edge must be activated
    for v in range(g.nvertices):
        if g.owners[v] == 1:
            for w in g.targets:
                if (v, w) in E:
                    cnf.append([-V[v], E[v, w]])

    # For every activated edge, the target vertex must be activated
    for v in range(g.nvertices):
        for w in g.targets:
            if (v, w) in E:
                cnf.append([-E[v, w], V[w]])

    # Progress measure constraints
    for v, w in E:
        q = g.colors[w]

        pm_constraints = []
        for p in set(g.colors):
            if p < q and p % 2 == 1:
                pm_constraints += lexcomp_le(w, v, p, calculate_nbits(p))

        if q % 2 == 0:
            for c in pm_constraints:
                cnf.append([-E[v, w]] + c)
        else:
            for c in lexcomp_sl(w, v, q, calculate_nbits(q)) + pm_constraints:
                cnf.append([-E[v, w]] + c)

    # Initialize SAT solver
    solver = Solver(name="g3")
    solver.append_formula(cnf)

    # Solve the problem
    if solver.solve():
        print("SAT")
        # print("Solution:", solver.get_model())
    else:
        print("UNSAT")

    solver.delete()

# ========================================================================================

# g = Game('./data/game-jurdzinski-2-3.dzn',Game.FIRST0)
g = Game(Game.JURDZINSKI, 2, 3, Game.FIRST0)

solveGame(g)
