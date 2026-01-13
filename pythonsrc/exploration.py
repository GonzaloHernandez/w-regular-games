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

def explorePlays(pathV, pathE, currentV, lastE) :
    if currentV in pathV :
        print(pathV+[currentV])
        print(pathE)
        h = pathV.index(currentV)
        print()
        best = min([ g.colors[v] for v in pathV[h: ]])
        b[lastE] = (currentV,best)
    else :
        for e in g.vedges[currentV] :
            explorePlays(pathV + [currentV], pathE + [e], g.targets[e], e)

# ------------------------------------------------------------------------------------

def explorePlays2(path, current) :
    try :
        index = path.index(current)
        bestcolor = max([ g.colors[v] for v in path[index:]])
        for v in path : 
            if bestcolor%2 != g.owners[v] :
                targets = [g.targets[i] for i in range(g.nedges) if g.sources[i]==v]
                differ = list(set(targets).difference(set(targets).intersection(set(path))))
                if len(differ)>0 : 
                    print(" ",path,[current],bestcolor)
                    return
        print("*",path,[current],bestcolor)
            
    except ValueError :
        for e in [ {'source':g.sources[i],'target':g.targets[i]} for i in range(g.nedges) ] :
            if e['source'] == current :
                explorePlays2(path + [current], e['target'])
    
# ------------------------------------------------------------------------------------

def getPlay(type, path, current) -> list :
    if current in path :
        h = path.index(current)
        best = max([ g.colors[v] for v in path[h:]])
        print(path+[current],best)
        return best%2, path
    else :
        if g.owners[current] == type :
            detourtype,detourpath = type,None
            for e in g.vedges[current] :
                detourtype,detourpath = getPlay( type, path+[current], g.targets[e] )
                if detourtype == type :
                    return detourtype, detourpath
            return detourtype, detourpath
        else :
            return getPlay( 1-type, path, current )

# ------------------------------------------------------------------------------------

def exploreCalculatingAverages(pathV, pathE, current) :
    if current in pathV :
        print('v:',pathV+[current])
        print('e:',pathE)
        print('w:', [ g.weights[e] for e in pathE ])
        print('m:', sum([ g.weights[e] for e in pathE ]) / len(pathE) )
        print()
    else :
        for e in [ {'id':i,'source':g.sources[i],'target':g.targets[i]} for i in range(g.nedges) ] :
            if e['source'] == current :
                exploreCalculatingAverages(pathV + [current], pathE + [e['id']],  e['target'])
                
# ------------------------------------------------------------------------------------


g = Game('./data/mpg-simple.dzn',Game.FIRST1)

# type,path = getPlay(ODD,[],0)
# print(path,"EVEN" if type==0 else "ODD")

exploreCalculatingAverages([],[],0)

# print(getPlay(ODD,[],0))