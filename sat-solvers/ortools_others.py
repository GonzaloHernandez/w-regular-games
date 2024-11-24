import os; os.system("clear")
import time,sys,re,json
arg = sys.argv

from python.game import Game
from ortools.sat.python import cp_model
model = cp_model.CpModel()

# -----------------------------------------------------------

g = Game('./data/game-sat.dzn')

V = [model.NewBoolVar(f'V{v}') for v in range(g.nvertices)]
E = [model.NewBoolVar('') for _ in range(g.nedges)]

# V.insert(0,None)
# E.insert(0,None)
# owners.insert(0,None)
# colors.insert(0,None)
# sources.insert(0,None)
# targets.insert(0,None)

model.AddBoolOr( [ V[g.start] ] )

for v in range(1,g.nvertices+1) :
    if g.owners[v]==g.EVEN :
        clause = [ V[v].Not() ]
        for e in range(1,g.nedges+1) :
            if g.sources[e]==v : clause += [ E[e] ]
        model.AddBoolOr(clause)

for v in range(1,g.nvertices+1) :
    if g.owners[v]==g.ODD :
        for e in range(1,g.nedges+1) :
            if g.sources[e]==v : 
                clause = [ V[v].Not() , E[e] ]
                model.AddBoolOr(clause)

for w in range(1,g.nvertices+1) :
    if w!=g.start :
        for e in range(1,g.nedges+1) :
            if g.sources[e]==w :
                clause = [ E[e].Not() , V[w] ]
                model.AddBoolOr(clause)

oddcolors   = sorted(set(g.colors[1:]))
mop         = max(oddcolors)

oddcolorspos = []
for i in range(mop+1) :
    if i in oddcolors :
        oddcolorspos += [ oddcolors.index(i) ]
    else :
        oddcolorspos += [ 0 ]

ncolors = len(oddcolors)
P = [None]+[[None]+[model.NewIntVar(1,g.nvertices,'') for c in range(ncolors)] for v in range(g.nvertices)]

# -----------------------------------------------------------

solver = cp_model.CpSolver()
status = solver.Solve(model)

if status == cp_model.FEASIBLE or status == cp_model.OPTIMAL :
    V = [print(solver.Value(V[v]),end=" ") for v in range(1,g.nvertices+1)]
    print()
    E = [print(solver.Value(E[e]),end=" ") for e in range(1,g.nedges+1)]
    print()
    P = [print([solver.Value(P[p][c]) for c in range(1,ncolors+1)],end=" ") for p in range(1,g.nvertices+1) ]
    print()

