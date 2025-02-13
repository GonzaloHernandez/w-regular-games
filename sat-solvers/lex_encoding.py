

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

        cnf.append([+X[v, p, c-i], +X[v, p, s-i]])
        cnf.append([+X[v, p, c-i], -X[v, p, e-i], -X[v, p, c-(i-1)]])
    
    cnf.append([X[v, p, bits]]) # final clause encoding the fact that a <= b

    return cnf

# ----------------------------------------------------------------------------------------

def lexle(w, v, p, bits) :
    if bits > 1:
        cnf = lexeq(w, v, p, bits)      # a <= b  
        cnf.append([-X[v, p, bits*2]])  # not a == b
    else :
        cnf = [[-X[w, p, 0]], [X[v, p, 0]]]
    return cnf

# ----------------------------------------------------------------------------------------

# V = [Bool(f"S_{v}") for v in range(g.nvertices)]

X = {
    (v, 0, i): 0 for v in range(2) for i in range(6)
}

pass