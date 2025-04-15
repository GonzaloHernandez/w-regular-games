import os; os.system("clear")
from game import Game

EVEN = 0
ODD = 1

# ------------------------------------------------------------------------------------

def explore(path, current) :
    if current in path :
        print(path+[current])
    else :
        for e in [ {'source':g.sources[i],'target':g.targets[i]} for i in range(g.nedges) ] :
            if e['source'] == current :
                explore(path + [current], e['target'])
    
# ------------------------------------------------------------------------------------

def explore(pathV, pathE, currentV, lastE) :
    if currentV in pathV :
        print(pathV+[currentV])
        print(pathE)
        h = pathV.index(currentV)
        print()
        best = min([ g.colors[v] for v in pathV[h: ]])
        b[lastE] = (currentV,best)
    else :
        for e in g.vedges[currentV] :
            explore(pathV + [currentV], pathE + [e], g.targets[e], e)

# ------------------------------------------------------------------------------------

def explorePlays(path, current) :
    try :
        index = path.index(current)
        mincolor = min([ g.colors[v] for v in path[index:]])
        for v in path : 
            if mincolor%2 != g.owners[v] :
                targets = [g.targets[i] for i in range(g.nedges) if g.sources[i]==v]
                differ = list(set(targets).difference(set(targets).intersection(set(path))))
                if len(differ)>0 : 
                    print(" ",path,[current],mincolor)
                    return
        print("*",path,[current],mincolor)
            
    except ValueError :
        for e in [ {'source':g.sources[i],'target':g.targets[i]} for i in range(g.nedges) ] :
            if e['source'] == current :
                explorePlays(path + [current], e['target'])
    
# ------------------------------------------------------------------------------------

def getPlay(type, path, current) -> list :
    if current in path :
        print(path+[current])
        h = path.index(current)
        best = min([ g.colors[v] for v in path[h:]])
        if best%2 == type : 
            return path
        else :
            return []
    else :
        for e in g.vedges[current] :
            if g.owners[current] == type :
                return getPlay(type, path + [current], g.targets[e])
            else :
                for e1 in g.vedges[current] :
                    detourpath = getPlay(1-type, [], g.targets[e1])
                    if detourpath != [] :
                        
                        return getPlay(type, path + [current], g.targets[e1])

# ------------------------------------------------------------------------------------

g = Game('./data/game-jurd-2-1.dzn',Game.FIRST0)
# g = Game(Game.JURDZINSKI,2,1,Game.FIRST0)

path = getPlay(EVEN,[],0)
print(path)
