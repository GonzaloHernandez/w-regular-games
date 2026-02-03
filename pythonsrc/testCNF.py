from pysat.solvers import Minisat22

V = [v1,v2] = [1,2]
E = [e1,e2,e3] = [3,4,5,6]

with Minisat22() as solver:
    solver.add_clause([-v1,e1,e2,e3])

    # At least one


    if solver.solve():
        model = solver.get_model()
        print(f"Raw Model: {model}")
    