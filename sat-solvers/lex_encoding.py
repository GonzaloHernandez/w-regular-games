import os; os.system("clear")
from  pysat.formula import IDPool
from pysat.solvers import Solver
from pysat.formula import CNF

pool = IDPool()

# ----------------------------------------------------------------------------------------

def lexcomp_greateorrequals(a, b, p, nbits) : # a >= b
    cnf = []

    A = [ X[a, p, i] for i in range(nbits) ]
    B = [ X[b, p, i] for i in range(nbits) ]
    C = [ pool.id()  for _ in range(nbits) ]
    Q = [ pool.id()  for _ in range(nbits-1) ]
    S = [ pool.id()  for _ in range(nbits-1) ]

    n = nbits - 1 # -1 to start counting on 0;
    
    cnf.append([-C[n], +A[n], -B[n]])
    cnf.append([+C[n], -A[n]])
    cnf.append([+C[n], +B[n]])
    
    for i in range(n-1, -1, -1): # from n-1 to 0
        cnf.append([-Q[i], -A[i], +B[i]])
        cnf.append([-Q[i], +A[i], -B[i]])
        cnf.append([+Q[i], +A[i], +B[i]])
        cnf.append([+Q[i], -A[i], -B[i]])

        cnf.append([-S[i], +A[i]])
        cnf.append([-S[i], -B[i]])
        cnf.append([+S[i], -A[i], +B[i]])

        cnf.append([-C[i], +S[i], +Q[i  ]])
        cnf.append([-C[i], +S[i], +C[i+1]])
        cnf.append([+C[i], -S[i]         ])
        cnf.append([+C[i], -Q[i], -C[i+1]])
    
    cnf.append([C[0]]) # final clause encoding the fact that a >= b

    return cnf

# ----------------------------------------------------------------------------------------

def lexcomp_strictlygreater(a, b, p, nbits) : # a > b
    cnf = []

    A = [ X[a, p, i] for i in range(nbits) ]
    B = [ X[b, p, i] for i in range(nbits) ]
    C = [ pool.id()  for _ in range(nbits) ]
    Q = [ pool.id()  for _ in range(nbits-1) ]
    S = [ pool.id()  for _ in range(nbits-1) ]

    n = nbits - 1 # -1 to start counting on 0;
    
    cnf.append([-C[n], +A[n]])
    cnf.append([-C[n], -B[n]])
    cnf.append([+C[n], -A[n], +B[n]])
    
    for i in range(n-1, -1, -1): # from n-1 to 0
        cnf.append([-Q[i], -A[i], +B[i]])
        cnf.append([-Q[i], +A[i], -B[i]])
        cnf.append([+Q[i], +A[i], +B[i]])
        cnf.append([+Q[i], -A[i], -B[i]])

        cnf.append([-S[i], +A[i]])
        cnf.append([-S[i], -B[i]])
        cnf.append([+S[i], -A[i], +B[i]])

        cnf.append([-C[i], +S[i], +Q[i  ]])
        cnf.append([-C[i], +S[i], +C[i+1]])
        cnf.append([+C[i], -S[i]         ])
        cnf.append([+C[i], -Q[i], -C[i+1]])
    
    cnf.append([C[0]]) # final clause encoding the fact that a > b

    return cnf

# ----------------------------------------------------------------------------------------

def ge(a,b,p,nbits) :
    cnf = []

    A = [ X[a, p, i] for i in range(nbits) ]
    B = [ X[b, p, i] for i in range(nbits) ]
    C = [ pool.id()  for _ in range(nbits) ]

    n = nbits - 1 # -1 to start counting on 0;
    
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

def sg(a,b,p,nbits) :
    cnf = []

    A = [ X[a, p, i] for i in range(nbits) ]
    B = [ X[b, p, i] for i in range(nbits) ]
    C = [ pool.id()  for _ in range(nbits) ]

    n = nbits - 1 # -1 to start counting on 0;
    
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

def greaterorequal_improved(a,b,p,nbits) :
    cnf = []

    A = [ X[a, p, i] for i in range(nbits) ]
    B = [ X[b, p, i] for i in range(nbits) ]
    C = [ pool.id()  for _ in range(nbits) ]
    Q = [ pool.id()  for _ in range(nbits-1) ]

    n = nbits - 1 # -1 to start counting on 0;
    
    cnf.append([-C[n], +A[n], -B[n]])
    cnf.append([+C[n], -A[n]])
    cnf.append([+C[n], +B[n]])
    
    for i in range(n-1, -1, -1): # from n-1 to 0
        cnf.append([-Q[i], -A[i], +B[i]])
        cnf.append([+A[i], +Q[i]])
        cnf.append([-B[i], +Q[i]])

        cnf.append([-C[i], -B[i], +A[i]])
        cnf.append([-C[i], -Q[i], +C[i+1]])

        cnf.append([+B[i], +Q[i],   C[i]])
        cnf.append([+B[i], -C[i+1], C[i]])
        cnf.append([-A[i], +Q[i],   C[i]])
        cnf.append([-A[i], -C[i+1], C[i]])

    cnf.append([C[0]]) # final clause encoding the fact that a >= b

    return cnf

# ----------------------------------------------------------------------------------------

def strictlygreater_improved(a,b,p,nbits) :
    cnf = []

    A = [ X[a, p, i] for i in range(nbits) ]
    B = [ X[b, p, i] for i in range(nbits) ]
    C = [ pool.id()  for _ in range(nbits) ]
    Q = [ pool.id()  for _ in range(nbits-1) ]

    n = nbits - 1 # -1 to start counting on 0;
    
    cnf.append([-C[n], +A[n]])
    cnf.append([-C[n], -B[n]])
    cnf.append([-A[n], +B[n], +C[n]])
    
    for i in range(n-1, -1, -1): # from n-1 to 0
        cnf.append([-Q[i], -A[i], +B[i]])
        cnf.append([+A[i], +Q[i]])
        cnf.append([-B[i], +Q[i]])

        cnf.append([-C[i], -B[i], +A[i]])
        cnf.append([-C[i], -Q[i], +C[i+1]])

        cnf.append([+B[i], +Q[i],   C[i]])
        cnf.append([+B[i], -C[i+1], C[i]])
        cnf.append([-A[i], +Q[i],   C[i]])
        cnf.append([-A[i], -C[i+1], C[i]])

    cnf.append([C[0]]) # final clause encoding the fact that a >= b

    return cnf

# ----------------------------------------------------------------------------------------

def write_dimacs(clauses, filename):
    with open(filename, "w") as f:
        f.write(f"p cnf {pool.top} {len(clauses)}\n")
        for clause in clauses:
            f.write(" ".join(map(str, clause)) + " 0\n")

# ----------------------------------------------------------------------------------------

nbits = 4

X = {
    (v, 0, i): pool.id() for v in range(2) for i in range(nbits)
}

cnf = []
cnf = strictlygreater_improved(0, 1, 0, nbits)

# import subprocess

# cnf.append([+X[0,0,0]])
# cnf.append([-X[0,0,1]])
# cnf.append([-X[0,0,2]])

# cnf.append([-X[1,0,0]])
# cnf.append([+X[1,0,1]])
# cnf.append([-X[1,0,2]])

# write_dimacs(cnf, "/home/chalo/g.cnf")
# subprocess.run("zchaff /home/chalo/g.cnf | grep RESULT", shell=True)

from itertools import product
for bits in product([-1, 1], repeat=nbits*2):
    cnf_temp = cnf.copy()
    n1 = 0
    for i in range(nbits) :
        cnf_temp.append([X[0,0,i]*bits[i]])
        if (bits[i]>0) :
            n1 += pow(2,nbits-1-i)

    n2 = 0
    for i in range(nbits,nbits*2) :
        cnf_temp.append([X[1,0,i-nbits]*bits[i]])
        if (bits[i]>0) :
            n2 += pow(2,nbits-1-(i-nbits))

    pycnf = CNF()
    pycnf.extend(cnf_temp)
    solver = Solver(name="g3")
    solver.append_formula(pycnf)

    r = solver.solve()
    o = n1>n2
    print(f"{' ' if r==o else '-'} {int(r)} {int(o)} {n1}>{n2} {[1 if b[0]>0 else 0 for b in cnf_temp[-(nbits*2):]]}")
