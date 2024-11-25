import os; os.system("clear")
from game import Game

# ------------------------------------------------------------------------------------

def attractor(V, p, g:Game) :
    A = V
    for v in V :
        ownvertices     = {g.sources[e] for e in g.getEdges() if g.targets[e]==v and g.owners[g.sources[e]]==p}
        othervertices   = {g.sources[e] for e in g.getEdges() if g.targets[e]==v and g.owners[g.sources[e]]!=p}
        novertices      = {g.sources[e] for e in g.getEdges() if g.owners[g.sources[e]]!=p and g.sources[e] in othervertices and g.targets[e] != v}
        localattractor  = ownvertices.union(othervertices.difference(novertices))
        A = A.union(attractor(localattractor,p,g/V))
    return A

def zielonka(g) :
    p = min([g.colors[v] for v in g.getVertices()]) % 2
    U = {v for v in g.getVertices() if g.colors[v]==p}
    A = attractor(U,p,g)
    zielonka(g/A)
    pass

# ------------------------------------------------------------------------------------

g = Game('./data/game-sat.dzn')
zielonka(g)

