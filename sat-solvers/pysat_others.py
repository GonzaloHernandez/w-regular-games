import os; os.system("clear")
import time,sys,re,json
arg = sys.argv

sys.path.insert(1,".")
from pythonsrc import Game
g = Game('./data/game-sat.dzn',Game.FIRST0)

# ----------------------------------------------------
from pysat.formula import CNF
from pysat.solvers import Solver
from pysat.formula import IDPool
from pysat.card import CardEnc

pool    = IDPool()
cnf     = CNF()
solver  = Solver()

# -----------------------------------------------------------

def newLitsArray(size) :
    global counter
    lits = []
    for _ in range(size) : 
        lits.append ( pool.id() )
    return lits

def get(lits,p) :
    if isinstance(p,int) :
        return lits[p-1]
    if isinstance(p,list) :
        l = []
        for v in p : 
            l.append(lits[v-1])
        return l

# -----------------------------------------------------------

V = newLitsArray(g.nvertices)
E = newLitsArray(g.nedges)

cnf.extend([[V[0]]])

constraint = []

for v in g.vertices :
    if g.owners[v]==Game.EVEN :
        clause = [ -V[v] ]
        for e in g.edges :
            if g.sources[e]==v : clause.append( E[e] )
        constraint.append(clause)

for v in g.vertices :
    if g.owners[v]==Game.ODD :
        for e in g.edges :
            if g.sources[e]==v : 
                clause = [ -V[v] , E[e] ]
        constraint.append(clause)

for w in g.vertices :
    if w!=g.start :
        for e in g.edges :
            if g.sources[e]==w :
                clause = [ -E[e] , V[v] ]
        constraint.append(clause)

print(V)
print(E)
print(constraint)

# solver.append_formula(cnf)
# solver.solve()
# sol = solver.get_model()

# print(sol)