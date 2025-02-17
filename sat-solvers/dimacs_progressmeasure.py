import os; os.system("clear")
import math,sys
from itertools import product
sys.path.insert(1,".")
from pythonsrc import Game

def get_var_id(variable_map, key):
    if key not in variable_map:
        variable_map[key] = len(variable_map) + 1
    return variable_map[key]

def calculate_bits(n) :
    return math.ceil(math.log2(n))*2 + (math.ceil(math.log2(n))-1)*2

# ========================================================================================

def generate_cnf_for_game(g):
    variable_map = {}  # Maps (name, args) -> unique integer ID
    clauses = []

    # Boolean variables
    V = {v: get_var_id(variable_map, ("S", v)) for v in range(g.nvertices)}
    E = {(v, w): get_var_id(variable_map, ("T", v, w)) for v, w in zip(g.sources, g.targets)}
    
    # Convert integer variables to Boolean binary variables
    max_priority = max(g.colors)
    X = {
        (v, p, i): get_var_id(variable_map, ("X", v, p, i))
        for v in range(g.nvertices)
        for p in set(g.colors) if p % 2 == 1
        for i in range(calculate_bits(sum(1 for w in range(g.nvertices) if g.colors[w] == p) + 1))
    }

    # First player vertices must be activated
    clauses.append([V[0]])

    # For every EVEN vertice, at least one outgoing edge must be activated
    for v in range(g.nvertices):
        if g.owners[v] == 0:
            clauses.append([-V[v]] + [E[v, w] for w in g.targets if (v, w) in E])

    # For every ODD vertice, each outgoing edge must be activated
    for v in range(g.nvertices):
        if g.owners[v] == 1:
            for w in g.targets:
                if (v, w) in E:
                    clauses.append([-V[v], E[v, w]])

    # # For every activated edge, the source vertex must be activated
    # for v in range(g.nvertices):
    #     for w in g.targets:
    #         if (v, w) in E:
    #             clauses.append([-E[v, w], V[v]])

    # For every activated edge, the target vertex must be activated
    for v in range(g.nvertices):
        for w in g.targets:
            if (v, w) in E:
                clauses.append([-E[v, w], V[w]]) 

    # ----------------------------------------------------------------------------------------

    def lexcomp_le(w, v, p, bits) :
        cnf = []
        n = bits-1
        c = bits*2-1
        e = bits*2+(bits-1)
        s = bits*2+(bits-1)*2
        
        cnf.append([-X[v, p, c], -X[w, p, n], X[v, p, n]])
        cnf.append([+X[v, p, c], +X[w, p, n]])
        cnf.append([+X[v, p, c], -X[v, p, n]])
        
        for i in range(1, bits):
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
        
        cnf.append([X[v, p, bits]]) # final clause encoding the fact that a <= b

        return cnf

    # ----------------------------------------------------------------------------------------

    def lexcomp_sl(w, v, p, bits) :
        cnf = []
        n = bits-1
        c = bits*2-1
        e = bits*2+(bits-1)
        s = bits*2+(bits-1)*2
        
        cnf.append([+X[w, p, c], -X[v, p, n], +X[v, p, c]])
        cnf.append([-X[v, p, c], -X[w, p, n]])
        cnf.append([-X[v, p, c], +X[v, p, n]])
        
        for i in range(1, bits):
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
        
        cnf.append([X[v, p, bits]]) # final clause encoding the fact that a <= b

        return cnf

    # ----------------------------------------------------------------------------------------

    # Progress measure constraints
    for v, w in E:
        q = g.colors[w]

        pm_constraints = []
        for p in set(g.colors):
            if p < q and p % 2 == 1:
                bits = math.ceil(math.log2(sum(1 for x in range(g.nvertices) if g.colors[x] == p) + 1))
                pm_constraints += lexcomp_le(w, v, p, bits)

        if q % 2 == 0:
            for c in pm_constraints:
                clauses.append([-E[v, w]] + c)
        else:
            bits = math.ceil(math.log2(sum(1 for x in range(g.nvertices) if g.colors[x] == q) + 1))
            for c in lexcomp_sl(w, v, q, bits) + pm_constraints:
                clauses.append([-E[v, w]] + c)

    return variable_map, clauses

# ========================================================================================

def write_dimacs(variable_map, clauses, filename="game.cnf"):
    with open(filename, "w") as f:
        f.write(f"p cnf {len(variable_map)} {len(clauses)}\n")
        for clause in clauses:
            f.write(" ".join(map(str, clause)) + " 0\n")

# ========================================================================================

import subprocess

def main():

    g = Game('./data/game-jurdzinski-2-3.dzn',Game.FIRST0)
    
    # g = Game(Game.JURDZINSKI,2,3,Game.FIRST0)
    
    variable_map, clauses = generate_cnf_for_game(g)
    write_dimacs(variable_map, clauses, "/home/chalo/game.cnf")
    print(f"CNF written to game.cnf with {len(variable_map)} variables and {len(clauses)} clauses.")

    subprocess.run("~/zchaff /home/chalo/game.cnf | grep RESULT", shell=True)
    # subprocess.run("cadical /home/chalo/game.cnf -q", shell=True)

if __name__ == "__main__":
    main()
