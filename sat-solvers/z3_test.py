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

# Boolean variables S_v indicating if a node belongs to player ∃'s winning set
S = [Bool(f"S_{v}") for v in range(nvertices)]

# Boolean variables T_v,w indicating if an edge is chosen in the strategy
T = {(v, w): Bool(f"T_{v}_{w}") for v, w in zip(sources, targets)}

# Integer variables for progress measures
X = {(v, p): Int(f"X_{v}_{p}") for v in range(nvertices) for p in set(colors) if p % 2 == 1}

# Constraints ensuring every node is assigned a strategy
for v in range(nvertices):
    if owners[v] == 0:  # Player ∃ chooses at least one edge
        solver.add(Or([T[v, w] for w in targets if (v, w) in T]))
    else:  # Player ∀ must allow all outgoing edges
        for w in targets:
            if (v, w) in T:
                solver.add(T[v, w])

# Progress measure constraints
for v, w in T:
    p = colors[w]
    if p % 2 == 0:
        solver.add(Implies(T[v, w], And([X[v, q] <= X[w, q] for q in set(colors) if q < p])))
    else:
        solver.add(Implies(T[v, w], And(X[v, p] > X[w, p],
                                       [X[v, q] <= X[w, q] for q in set(colors) if q < p])))

# Ensure the initial state belongs to the existential player's winning region
solver.add(S[0])

# Solve the formula
if solver.check() == sat:
    model = solver.model()
    print("Winning strategy found!")
    for v, w in T:
        if model.evaluate(T[v, w]):
            print(f"Edge ({v} -> {w}) is selected in strategy.")
else:
    print("No winning strategy found for player ∃.")
