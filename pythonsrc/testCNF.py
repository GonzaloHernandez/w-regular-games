import os; os.system("clear")
from pysat.solvers import Minisat22

pool = 0

def getVar():
    global pool
    pool += 1
    return pool

def getVars(length):
    return [getVar() for _ in range(length)]

V = [v1,v2,v3,v4] = getVars(4)
E = [e1,e2,e3,e4] = getVars(4)

p = 1

print(*(f"v{i}" for i in range(1,len(V)+1)), *(f"e{i}" for i in range(1,len(E)+1)))

with Minisat22() as solver:
    solver.append_formula([[v4],[-e1],[e2],[e3],[-e4]]) # to test

    # At least one ------------------------------------------------------------
    # solver.add_clause([-v1*p]+[e*p for e in E])

    # At most one -------------------------------------------------------------
    # i = 0
    # S = getVars(len(E)-1)
    # # First literal
    # solver.add_clause([-E[i]*p,S[0]])
    # i += 1
    # # Middle literals
    # while i < len(E)-1 :
    #     solver.add_clause([-S[i-1],S[i]])
    #     solver.add_clause([-E[i]*p,-S[i-1]])
    #     solver.add_clause([-E[i]*p,S[i]])
    #     i += 1
    # # Last literal
    # solver.add_clause([-E[i]*p,-S[i-1]])
    # -------------------------------------------------------------------------

    # Every edge
    # for e in E:
    #     solver.add_clause([-v2*p,e*p])

    # -------------------------------------------------------------------------
    # Target
    for e,w in zip(E,V):
        solver.add_clause([-e*p,w*p])
    # -------------------------------------------------------------------------

    if solver.solve():
        model = solver.get_model()
        for var in model:
            print(" 1" if var>0 else " 0" ,end=" ")
        print()
    else:
        print("Unsatisfiable")