import os; os.system("clear")
from game import Game

# ------------------------------------------------------------------------------------

def attractor(V, parity) :

    pass

def zielonka(V) :
    p = min([g.colors[v] for v in V]) % 2
    u = [v for v in V if g.colors[v]==p]
    print(u)
    pass

# ------------------------------------------------------------------------------------

g = Game('./data/game-sat.dzn')
zielonka([v for v in range(g.nvertices)])

