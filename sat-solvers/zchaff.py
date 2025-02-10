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
    """ Assigns a unique integer ID to each Boolean variable """
    if key not in variable_map:
        variable_map[key] = len(variable_map) + 1
    return variable_map[key]

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
        for i in range(math.ceil(math.log2(sum(1 for w in range(g.nvertices) if g.colors[w] == p) + 1)))
    }

    # Ensure the first player's vertex is activated
    clauses.append([V[0]])

    # Existential player constraints (EVEN)
    for v in range(g.nvertices):
        if g.owners[v] == 0:
            clauses.append([-V[v]] + [E[v, w] for w in g.targets if (v, w) in E])

    # Universal player constraints (ODD)
    for v in range(g.nvertices):
        if g.owners[v] == 1:
            for w in g.targets:
                if (v, w) in E:
                    clauses.append([-V[v], E[v, w]])

    # Edge activation constraints
    for v in range(g.nvertices):
        for w in g.targets:
            if (v, w) in E:
                clauses.append([-E[v, w], V[v]])  # If edge is activated, source must be activated
                clauses.append([-E[v, w], V[w]])  # If edge is activated, target must be activated

    # Progress measure constraints
    def greater_or_equals(v, w, p, bits):
        return [
            [-X[v, p, i], X[w, p, i]] for i in range(bits)
        ]

    def strictly_greater(v, w, p, bits):
        constraints = [[X[v, p, 0], -X[w, p, 0]]]  # Lowest bit must be different
        for i in range(1, bits):
            constraints.append([-X[w, p, i], X[v, p, i]])
            constraints.append([X[w, p, i], -X[v, p, i], *constraints[-1]])
        return constraints

    for v, w in E:
        q = g.colors[w]
        bits = math.ceil(math.log2(sum(1 for x in range(g.nvertices) if g.colors[x] == q) + 1))

        constraints = []
        for p in set(g.colors):
            if p < q and p % 2 == 1:
                constraints += greater_or_equals(v, w, p, bits)

        if q % 2 == 0:
            for c in constraints:
                clauses.append([-E[v, w]] + c)
        else:
            clauses += strictly_greater(v, w, q, bits)
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
        nvertices = 7,
        owners    = [1,0,0,1,0,1,0],
        colors    = [2,1,2,2,4,3,4],
        nedges    = 12,
        sources   = [0,0,1,2,2,2,3,4,5,5,5,6],
        targets   = [1,2,2,0,3,5,2,5,2,4,6,5]
    )

    variable_map, clauses = generate_cnf_for_game(g)
    write_dimacs(variable_map, clauses, "\home\chalo\game.cnf")
    print(f"CNF written to game.cnf with {len(variable_map)} variables and {len(clauses)} clauses.")

if __name__ == "__main__":
    main()
