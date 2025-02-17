

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
        cnf.append([-X[v, p, c-i], +X[v, p, s-i], +X[v, p, c-(i-1)]])

        cnf.append([+X[v, p, c-i], -X[v, p, s-i]])
        cnf.append([+X[v, p, c-i], -X[v, p, e-i], -X[v, p, c-(i-1)]])
    
    cnf.append([X[v, p, bits]]) # final clause encoding the fact that a <= b

    return cnf

# ----------------------------------------------------------------------------------------

def lexle(w, v, p, bits) :
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
    
def lexle_(w, v, p, bits) :
    if bits > 1:
        cnf = lexeq(w, v, p, bits)      # a <= b  
        cnf.append([-X[v, p, bits*2]])  # not a == b
        # cnf.append([-X[w, p, (bits*2)+(bits-1)-i] for i in range(1,bits)] )  # not a == b
    else :
        cnf = [[-X[w, p, 0]], [X[v, p, 0]]]
    return cnf

# ----------------------------------------------------------------------------------------

def write_dimacs(vars, clauses, filename):
    with open(filename, "w") as f:
        f.write(f"p cnf {vars} {len(clauses)}\n")
        for clause in clauses:
            f.write(" ".join(map(str, clause)) + " 0\n")

# ----------------------------------------------------------------------------------------

id = 0
bits = 1

def getid():
    global id
    id = id+1
    return id

X = {
    (v, 0, i): getid() for v in range(2) for i in range(bits*2+(bits-1)*2)
}

cnf = lexle(0, 1, 0, bits)
cnf.append([+X[0,0,0]])
# cnf.append([+X[0,0,1]])
# cnf.append([-X[0,0,2]])

cnf.append([-X[1,0,0]])
# cnf.append([+X[1,0,1]])
# cnf.append([-X[1,0,2]])

write_dimacs(id, cnf, "/home/chalo/game.cnf")

