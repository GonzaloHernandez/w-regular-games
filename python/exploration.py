import os; os.system("clear")
from game import Game

# ------------------------------------------------------------------------------------

def explore(path, current) :
    if current in path :
        print(path+[current])
    else :
        for e in [ {'source':g.sources[i],'target':g.targets[i]} for i in range(g.nedges) ] :
            if e['source'] == current :
                explore(path + [current], e['target'])
    
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

g = Game('./data/game-sat.dzn')
explorePlays([],0)

