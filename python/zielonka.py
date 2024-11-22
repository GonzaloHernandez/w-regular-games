import os; os.system("clear")
from game import Game

# ------------------------------------------------------------------------------------

def attractor(V, p, g) :
    for v in V :
        ownvertices     = {vi for vi in g.vertices if g.owners[vi]==p and vi in {g.edgesv[e] for e in g.edges if g.edgesw[e]==vi} }
        othervertices   = {vi for vi in g.vertices if g.owners[vi]!=p and vi in {g.edgesv[e] for e in g.edges if g.edgesw[e]==vi} }
        # othervertices   = {i for i in g.vertices if g.edgesw[i]==v and g.owners[i]!=p}
        # novertices      = {i for i in othervertices if g.edgesw[i]!=v}
        # A  = ownvertices.union(othervertices.difference(novertices))
        more = attractor(A,p,g)
    return V.union(A).union(more)

def zielonka(g) :
    p = min([g.colors[v] for v in g.vertices]) % 2
    U = {v for v in g.vertices if g.colors[v]==p}
    A = attractor(U,p,g)
    pass

# ------------------------------------------------------------------------------------

g = Game('./data/game-sat.dzn')
zielonka(g)

