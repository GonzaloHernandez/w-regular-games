import math
from itertools import product

class ParityGame:
    def __init__(self, nvertices, colors, owners, nedges, sources, targets):
        self.nvertices = nvertices
        self.colors = colors
        self.owners = owners
        self.nedges = nedges
        self.sources = sources
        self.targets = targets



def get_var_id(variable_map, key):
    if key not in variable_map:
        variable_map[key] = len(variable_map) + 1
    return variable_map[key]

def calculate_bits(n) :
    return math.ceil(math.log2(n))*2 + (math.ceil(math.log2(n))-1)*2

# ----------------------------------------------------------------------------------------

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

    # For every activated edge, the source vertex must be activated
    for v in range(g.nvertices):
        for w in g.targets:
            if (v, w) in E:
                clauses.append([-E[v, w], V[v]])

    # For every activated edge, the target vertex must be activated
    for v in range(g.nvertices):
        for w in g.targets:
            if (v, w) in E:
                clauses.append([-E[v, w], V[w]]) 

    # ----------------------------------------------------------------------------------------

    def lexeq(w, v, p, bits) :
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
            cnf.append([-X[v, p, c-i], +X[v, p, s-i], +X[v, p, c-i-1]])

            cnf.append([+X[v, p, c-i], +X[v, p, s-i]])
            cnf.append([+X[v, p, c-i], -X[v, p, e-i], -X[v, p, c-i]])
        
        cnf.append([X[v, p, bits]]) # final clause encoding the fact that a <= b

        return cnf

    # ----------------------------------------------------------------------------------------
    
    def lexle(w, v, p, bits) :
        cnf = lexeq(w, v, p, bits)      # a <= b  
        cnf.append([-X[v, p, bits*2]])  # not a == b
        return cnf

    # ----------------------------------------------------------------------------------------

    for v, w in E:
        q = g.colors[w]

        constraints = []
        for p in set(g.colors):
            if p < q and p % 2 == 1:
                constraints += lexeq(w, v, p, math.ceil(math.log2(sum(1 for x in range(g.nvertices) if g.colors[x] == p) + 1)))

        if q % 2 == 0:
            for c in constraints:
                clauses.append([-E[v, w]] + c)
        else:
            clauses += lexle(w, v, q, math.ceil(math.log2(sum(1 for x in range(g.nvertices) if g.colors[x] == q) + 1)))
            for c in constraints:
                clauses.append([-E[v, w]] + c)

    return variable_map, clauses

def write_dimacs(variable_map, clauses, filename="game.cnf"):
    with open(filename, "w") as f:
        f.write(f"p cnf {len(variable_map)} {len(clauses)}\n")
        for clause in clauses:
            f.write(" ".join(map(str, clause)) + " 0\n")

def main():
    # Example parity game: 4 nodes, edges, priorities, owners
    g = ParityGame(
        nvertices = 19,
        owners    = [1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,0,1,0],
        colors    = [2,1,2,2,1,2,2,4,3,4,4,3,4,4,6,5,6,5,6],
        nedges    = 40,
        sources   = [0,0,1,2,2,2,3,3,3,4,5,5,5,6,7,7,8,9,9,9,10,10,10,11,12,12,12,13,14,15,15,15,15,16,16,17,17,17,17,18],
        targets   = [1,2,2,0,3,15,2,4,5,5,3,6,17,5,8,9,9,7,10,15,9,11,12,12,10,13,17,12,15,14,16,2,9,15,17,16,18,5,12,17],
    )

    variable_map, clauses = generate_cnf_for_game(g)
    write_dimacs(variable_map, clauses, "/home/chalo/game.cnf")
    print(f"CNF written to game.cnf with {len(variable_map)} variables and {len(clauses)} clauses.")

if __name__ == "__main__":
    main()
