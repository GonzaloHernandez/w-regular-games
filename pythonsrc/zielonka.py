import os; os.system("clear")
from game import Game

# ------------------------------------------------------------------------------------

def attractor(V, q, g:Game) :
    A = V
    for v in V :
        ownvertices     = {g.sources[e] for e in g.getEdges() if g.targets[e]==v and g.owners[g.sources[e]]==q}
        othervertices   = {g.sources[e] for e in g.getEdges() if g.targets[e]==v and g.owners[g.sources[e]]!=q}
        novertices      = {g.sources[e] for e in g.getEdges() if g.owners[g.sources[e]]!=q and g.sources[e] in othervertices and g.targets[e] not in V}
        localattractor  = ownvertices.union(othervertices.difference(novertices))
        A = A.union(attractor(localattractor, q, g/V))
    return A

# ------------------------------------------------------------------------------------

def zielonka(g) :
    Z = [[],[]]
    if g.getVertices() == [] :
        return [], []
    else :
        m = min([g.colors[v] for v in g.getVertices()])
        q = m % 2
        U = {v for v in g.getVertices() if g.colors[v] == m}
        A = attractor(U, q, g)
        W = zielonka( g/A )
        if W[1-q] == [] :
            Z[q],Z[1-q] = A.union(W[q]), []
        else :
            B = attractor(W[1-q], 1-q, g)
            W = zielonka( g/B )
            Z[q],Z[1-q] = W[q], B.union(W[1-q])
    return Z

# ------------------------------------------------------------------------------------

g = Game('./data/game-jurdzinski-2-1.dzn',Game.FIRST0)
# g = Game(Game.JURDZINSKI,3,2,Game.FIRST0)
# g = Game(Game.RANDOM,10,Game.FIRST0)
# print(g)
W = zielonka(g)
print(W)