import os; os.system("clear")

# ----------------------------------------------------------------------------------------

def lexcomp_le(w, v, p, nbits) :
    cnf = []
    n = nbits-1
    c = nbits*2-1
    e = nbits*2+(nbits-1)
    s = nbits*2+(nbits-1)*2
    
    cnf.append([-X[v, p, c], -X[w, p, n], X[v, p, n]])
    cnf.append([+X[v, p, c], +X[w, p, n]])
    cnf.append([+X[v, p, c], -X[v, p, n]])
    
    for i in range(1, nbits):
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
    
    cnf.append([X[v, p, nbits]]) # final clause encoding the fact that a <= b

    return cnf

# ----------------------------------------------------------------------------------------

def lexcomp_sl(w, v, p, nbits) :
    cnf = []
    n = nbits-1
    c = nbits*2-1
    e = nbits*2+(nbits-1)
    s = nbits*2+(nbits-1)*2
    
    cnf.append([+X[w, p, c], -X[v, p, n], +X[v, p, c]])
    cnf.append([-X[v, p, c], -X[w, p, n]])
    cnf.append([-X[v, p, c], +X[v, p, n]])
    
    for i in range(1, nbits):
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
    
    cnf.append([X[v, p, nbits]]) # final clause encoding the fact that a <= b

    return cnf

# ----------------------------------------------------------------------------------------

def write_dimacs(vars, clauses, filename):
    with open(filename, "w") as f:
        f.write(f"p cnf {vars} {len(clauses)}\n")
        for clause in clauses:
            f.write(" ".join(map(str, clause)) + " 0\n")

# ----------------------------------------------------------------------------------------

id = 0
nbits = 2

def getid():
    global id
    id = id+1
    return id

X = {
    (v, 0, i): getid() for v in range(2) for i in range(nbits*2+(nbits-1)*2)
}

cnf = lexcomp_sl(0, 1, 0, nbits)
cnf.append([-X[0,0,0]])
cnf.append([+X[0,0,1]])
# cnf.append([-X[0,0,2]])

cnf.append([-X[1,0,0]])
cnf.append([+X[1,0,1]])
# cnf.append([-X[1,0,2]])

write_dimacs(id, cnf, "/home/chalo/g.cnf")

import subprocess

subprocess.run("~/zchaff /home/chalo/g.cnf | grep RESULT", shell=True)