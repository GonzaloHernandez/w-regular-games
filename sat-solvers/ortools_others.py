import os; os.system("clear")
import sys
arg = sys.argv

sys.path.insert(1,".")
from pythonsrc import Game
from ortools.sat.python import cp_model
model = cp_model.CpModel()

# -----------------------------------------------------------

g = Game('./data/game-sat.dzn',Game.FIRST0)

V = [model.NewBoolVar(f'V{v}') for v in range(g.nvertices)]
E = [model.NewBoolVar('') for _ in range(g.nedges)]

model.AddBoolOr( [ V[g.start] ] )

for v in g.vertices :
    if g.owners[v]==g.EVEN :
        clause = [ V[v].Not() ]
        for e in g.edges :
            if g.sources[e]==v : clause += [ E[e] ]
        model.AddBoolOr(clause)

for v in g.vertices :
    if g.owners[v]==g.ODD :
        for e in g.edges :
            if g.sources[e]==v : 
                clause = [ V[v].Not() , E[e] ]
                model.AddBoolOr(clause)

for w in g.vertices :
    if w!=g.start :
        for e in g.edges :
            if g.sources[e]==w :
                clause = [ E[e].Not() , V[w] ]
                model.AddBoolOr(clause)

oddcolors   = sorted({c for c in g.colors if c%2==1})
mop         = max(oddcolors)

oddcolorspos = []
for i in range(mop+1) :
    if i in oddcolors :
        oddcolorspos += [ oddcolors.index(i) ]
    else :
        oddcolorspos += [ -1 ]

ncolors = len(oddcolors)
P = [[model.NewIntVar(1,g.nvertices,'') for c in range(ncolors)] for v in g.vertices]

# -----------------------------------------------------------

solver = cp_model.CpSolver()
status = solver.Solve(model)

if status == cp_model.FEASIBLE or status == cp_model.OPTIMAL :
    V = [print(solver.Value(V[v]),end=" ") for v in g.vertices]
    print()
    E = [print(solver.Value(E[e]),end=" ") for e in g.edges]
    print()
    P = [print([solver.Value(P[p][c]) for c in range(ncolors)],end=" ") for p in g.vertices ]
    print()

