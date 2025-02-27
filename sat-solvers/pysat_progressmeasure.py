import os; os.system("clear")
import sys,math,time,subprocess
from pysat.solvers import Solver
from pysat.formula import CNF
from pysat.formula import IDPool
sys.path.insert(1,".")
from pythonsrc import Game

pool  = IDPool()

# ========================================================================================

def getSAT(g,start=0):

    def calculate_nbits(n) :
        return math.ceil(math.log2(sum(1 for x in range(g.nvertices) if g.colors[x] == n) + 1))

    # ----------------------------------------------------------------------------------------

    def lexcomp_greateorrequals(a, b, p) : # a >= b
        cnf = []

        A = [ X[a, p, i] for i in range(calculate_nbits(p)) ]
        B = [ X[b, p, i] for i in range(calculate_nbits(p)) ]
        C = [ pool.id()  for _ in range(calculate_nbits(p)) ]

        n = calculate_nbits(p) - 1 # -1 to start counting on 0;
        
        cnf.append([-C[n], +A[n], -B[n]])
        cnf.append([+C[n], -A[n]])
        cnf.append([+C[n], +B[n]])
        
        for i in range(n-1, -1, -1): # from n-1 to 0
            cnf.append([-C[i], -B[i], +A[i]])
            cnf.append([-C[i], +A[i], +C[i+1]])
            cnf.append([-C[i], -B[i], +C[i+1]])

            cnf.append([+C[i], +B[i], -A[i]])
            cnf.append([+C[i], +B[i], -A[i], -C[i+1]])
            cnf.append([+C[i], +B[i], -C[i+1]])
            cnf.append([+C[i], -A[i], -C[i+1]])

        cnf.append([C[0]]) # final clause encoding the fact that a >= b

        return cnf

    # ----------------------------------------------------------------------------------------

    def lexcomp_strictlygreater(a, b, p) : # a > b
        cnf = []

        A = [ X[a, p, i] for i in range(calculate_nbits(p)) ]
        B = [ X[b, p, i] for i in range(calculate_nbits(p)) ]
        C = [ pool.id()  for _ in range(calculate_nbits(p)) ]

        n = calculate_nbits(p) - 1 # -1 to start counting on 0;
                
        cnf.append([-C[n], +A[n]])
        cnf.append([-C[n], -B[n]])
        cnf.append([-A[n], +B[n], +C[n]])
        
        for i in range(n-1, -1, -1): # from n-1 to 0
            cnf.append([-C[i], -B[i], +A[i]])
            cnf.append([-C[i], +A[i], +C[i+1]])
            cnf.append([-C[i], -B[i], +C[i+1]])

            cnf.append([+C[i], +B[i], -A[i]])
            cnf.append([+C[i], +B[i], -A[i], -C[i+1]])
            cnf.append([+C[i], +B[i], -C[i+1]])
            cnf.append([+C[i], -A[i], -C[i+1]])

        cnf.append([C[0]]) # final clause encoding the fact that a >= b

        return cnf

    # ----------------------------------------------------------------------------------------

    cnf = []

    # Map variables to integer literals
    V = {v: pool.id() for v in range(g.nvertices)}
    E = {(v, w): pool.id() for i, (v, w) in enumerate(zip(g.sources, g.targets))}
    X = {
        (v, p, i): pool.id()
        for v in range(g.nvertices)
        for p in set(g.colors) if p % 2 == 1
        for i in range(calculate_nbits(p))
    }

    # First player vertex must be activated
    cnf.append([V[start]])

    # For every EVEN vertex, at least one outgoing edge must be activated
    for v in range(g.nvertices):
        if g.owners[v] == 0:
            cnf.append([-V[v]] + [E[v, w] for w in g.targets if (v, w) in E])

    # For every ODD vertex, each outgoing edge must be activated
    for v in range(g.nvertices):
        if g.owners[v] == 1:
            for w in g.targets:
                if (v, w) in E:
                    cnf.append([-V[v], E[v, w]])

    # For every activated edge, the target vertex must be activated
    for v in range(g.nvertices):
        for w in g.targets:
            if (v, w) in E:
                cnf.append([-E[v, w], V[w]])

    # Progress measure constraints
    for v, w in E:
        q = g.colors[w]

        pm_constraints = []
        for p in set(g.colors):
            if p < q and p % 2 == 1:
                pm_constraints += lexcomp_greateorrequals(v, w, p)

        if q % 2 == 0:
            for c in pm_constraints:
                cnf.append([-E[v, w]] + c)
        else:
            for c in lexcomp_strictlygreater(v, w, q) + pm_constraints:
                cnf.append([-E[v, w]] + c)

    return cnf

# ========================================================================================

def write_dimacs(clauses, filename="game.cnf"):
    with open(filename, "w") as f:
        f.write(f"p cnf {pool.top} {len(clauses)}\n")
        for clause in clauses:
            f.write(" ".join(map(str, clause)) + " 0\n")

# ========================================================================================

# filepath = "/home/chalo/Deleteme/bes-benchmarks/gm/"
# filename = "jurdzinskigame_50_50"
# g = Game(filepath+filename+".dzn",Game.FIRST0)

# arg = sys.argv

levels = 80 #int(arg[1])
blocks = 10 #int(arg[2])

g = Game(Game.JURDZINSKI, levels, blocks, Game.FIRST0)

t1 = time.time()
clauses = getSAT(g)
t2 = time.time()

# ----------------------------------------------------------------------------------------
# save dimacs file
# ----------------------------------------------------------------------------------------
write_dimacs(clauses, "/home/chalo/new.cnf")


# result = subprocess.run("~/zchaff /home/chalo/game.cnf | grep 'Total Run Time'", capture_output=True, text=True, shell=True)

# print(f"{t2-t1},{result.stdout}")

# ----------------------------------------------------------------------------------------
# solve
# ----------------------------------------------------------------------------------------
# cnf = CNF()
# cnf.extend(clauses)
# solver = Solver(name="g3")
# solver.append_formula(cnf)

# t1 = time.time()
# if solver.solve():
#     print("SAT",end=",") 
# else:
#     print("UNSAT",end=",")
# t2 = time.time()

# print(t2-t1)
# solver.delete()
