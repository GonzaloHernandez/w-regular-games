import os; os.system("clear")

from z3 import *

# Given Parity Game data
nvertices = 7
owners = [1, 0, 0, 1, 0, 1, 0]
colors = [2, 1, 2, 2, 4, 3, 4]
nedges = 12
sources = [0, 0, 1, 2, 2, 2, 3, 4, 5, 5, 5, 6]
targets = [1, 2, 2, 0, 3, 5, 2, 5, 2, 4, 6, 5]

# Create a solver
solver = Solver()

V = [Bool(f"S_{v}") for v in range(nvertices)]
E = {(v, w): Bool(f"T_{v}_{w}") for v, w in zip(sources, targets)}
X = {(v, p): Int(f"X_{v}_{p}") for v in range(nvertices) for p in set(colors) if p % 2 == 1}

# Ensure the initial state belongs to the existential player's winning region
solver.add(V[0])

# For every EVEN vertice, at least one outgoing edge must be activated
for v in range(nvertices):
    if owners[v] == 0:
        solver.add(Or([V[v]==False]+[E[v, w] for w in targets if (v, w) in E]))

# For every ODD vertice, each outgoing edge must be activated
for v in range(nvertices):
    if owners[v] == 1:
        for w in targets:
            if (v, w) in E:
                solver.add(Or([V[v]==False] +  [E[v, w]]))

# For every activated edge, the source vertex must be activated
for v in range(nvertices):
    for w in targets:
        if (v, w) in E:
            solver.add(Or([E[v, w]==False] + [V[v]]))


# For every activated edge, the target vertex must be activated
for v in range(nvertices):
    for w in targets:
        if (v, w) in E:
            solver.add(Or([E[v, w]==False] + [V[w]]))

# Progress measure constraints
for v, w in E:
    p = colors[w]
    if p % 2 == 0:
        solver.add(Implies(E[v, w], And([X[v, q] <= X[w, q] for q in set(colors) if q < p])))
    else:
        solver.add(Implies(E[v, w], And(X[v, p] > X[w, p],
                                       [X[v, q] <= X[w, q] for q in set(colors) if q < p])))


# Solve the formula
if solver.check() == sat:
    model = solver.model()
    print("Winning strategy found!")
    for v, w in E:
        if model.evaluate(E[v, w]):
            print(f"Edge ({v} -> {w}) is selected in strategy.")
else:
    print("No winning strategy found for player ∃.")
