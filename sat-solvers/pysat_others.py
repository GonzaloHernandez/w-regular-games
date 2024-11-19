import os; os.system("clear")
import time,sys,re,json
arg = sys.argv

# ----------------------------------------------------

nvertices = 0
owners  = []
colors  = []

nedges  = 0
edgesv  = []
edgesw  = []

with open('./data/game-sat.dzn', 'r') as file:
    for line in file:
        if line.strip() == "" : continue

        key, value = [item.strip() for item in re.sub(r"[;]","",line).split('=')]
        
        if      key=="nvertices"    : nvertices = json.loads(value)
        elif    key=="owners"       : owners    = json.loads(value)
        elif    key=="colors"       : colors    = json.loads(value)
        elif    key=="nedges"       : nedges    = json.loads(value)
        elif    key=="edgesv"       : edgesv    = json.loads(value)
        elif    key=="edgesw"       : edgesw    = json.loads(value)

even    = 0
odd     = 1
start   = 1

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

V = newLitsArray(nvertices)
E = newLitsArray(nedges)

cnf.extend([[V[0]]])

constraint = []

for v in range(nvertices) :
    if owners[v]==even :
        clause = [ -V[v] ]
        for e in range(nedges) :
            if edgesv[e]==v : clause.append( E[e] )
        constraint.append(clause)

for v in range(nvertices) :
    if owners[v]==odd :
        for e in range(nedges) :
            if edgesv[e]==v : 
                clause = [ -V[v] , E[e] ]
        constraint.append(clause)

for w in range(nvertices) :
    if w!=start :
        for e in range(nedges) :
            if edgesv[e]==w :
                clause = [ -E[e] , V[v] ]
        constraint.append(clause)

print(V)
print(E)
print(constraint)

# solver.append_formula(cnf)
# solver.solve()
# sol = solver.get_model()

# print(sol)