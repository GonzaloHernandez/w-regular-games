import os; os.system("clear")
import time,sys,re,json
arg = sys.argv

# -----------------------------------------------------------

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

# -----------------------------------------------------------

from ortools.sat.python import cp_model
model = cp_model.CpModel()

# -----------------------------------------------------------

V = [model.NewBoolVar(f'V{v}') for v in range(nvertices)]
E = [model.NewBoolVar('') for _ in range(nedges)]

V.insert(0,None)
E.insert(0,None)
owners.insert(0,None)
colors.insert(0,None)
edgesv.insert(0,None)
edgesw.insert(0,None)

model.AddBoolOr( [ V[start] ] )

for v in range(1,nvertices+1) :
    if owners[v]==even :
        clause = [ V[v].Not() ]
        for e in range(1,nedges+1) :
            if edgesv[e]==v : clause += [ E[e] ]
        model.AddBoolOr(clause)

for v in range(1,nvertices+1) :
    if owners[v]==odd :
        for e in range(1,nedges+1) :
            if edgesv[e]==v : 
                clause = [ V[v].Not() , E[e] ]
                model.AddBoolOr(clause)

for w in range(1,nvertices+1) :
    if w!=start :
        for e in range(1,nedges+1) :
            if edgesv[e]==w :
                clause = [ E[e].Not() , V[w] ]
                model.AddBoolOr(clause)

oddcolors   = sorted(set(colors[1:]))
mop         = max(oddcolors)

oddcolorspos = []
for i in range(mop+1) :
    if i in oddcolors :
        oddcolorspos += [ oddcolors.index(i) ]
    else :
        oddcolorspos += [ 0 ]

ncolors = len(oddcolors)
P = [None]+[[None]+[model.NewIntVar(1,nvertices,'') for c in range(ncolors)] for v in range(nvertices)]

# -----------------------------------------------------------

solver = cp_model.CpSolver()
status = solver.Solve(model)

if status == cp_model.FEASIBLE or status == cp_model.OPTIMAL :
    V = [print(solver.Value(V[v]),end=" ") for v in range(1,nvertices+1)]
    print()
    E = [print(solver.Value(E[e]),end=" ") for e in range(1,nedges+1)]
    print()
    P = [print([solver.Value(P[p][c]) for c in range(1,ncolors+1)],end=" ") for p in range(1,nvertices+1) ]
    print()

