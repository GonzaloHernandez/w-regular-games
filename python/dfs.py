import os; os.system("clear")
import warnings,sys,re,json
warnings.filterwarnings("error", message="model inconsistency detected")
arg = sys.argv

# ----------------------------------------------------


def getGame(filename) -> list :
    nvertices = 0
    owners = []
    colors = []

    nedges = 0
    edgesv = []
    edgesw = []

    with open(filename, 'r') as file:
        for line in file:
            if line.strip() == "" : continue

            key, value = [item.strip() for item in re.sub(r"[;]","",line).split('=')]
            
            if      key=="nvertices"    : nvertices = json.loads(value)
            elif    key=="owners"       : owners    = json.loads(value)
            elif    key=="colors"       : colors    = json.loads(value)
            elif    key=="nedges"       : nedges    = json.loads(value)
            elif    key=="edgesv"       : edgesv    = json.loads(value)
            elif    key=="edgesw"       : edgesw    = json.loads(value)
    
    return [nvertices,owners,colors,nedges,edgesv,edgesw]

# ------------------------------------------------------------------------------------

class Game :
    def __init__(self,filename) -> None:
        with open(filename, 'r') as file:
            for line in file:
                if line.strip() == "" : continue

                key, value = [item.strip() for item in re.sub(r"[;]","",line).split('=')]
                
                if      key=="nvertices"    : self.nvertices = json.loads(value)
                elif    key=="owners"       : self.owners    = json.loads(value)
                elif    key=="colors"       : self.colors    = json.loads(value)
                elif    key=="nedges"       : self.nedges    = json.loads(value)
                elif    key=="edgesv"       : self.edgesv    = json.loads(value)
                elif    key=="edgesw"       : self.edgesw    = json.loads(value)
        self.fixZeroPosition()
    
    def fixZeroPosition(self) :
        for e in range(self.nedges) :
            self.edgesv[e] -= 1
            self.edgesw[e] -= 1

# ------------------------------------------------------------------------------------

def dfs(game) :
    pass


# ------------------------------------------------------------------------------------

g = Game('./data/game-sat.dzn')

# ------------------------------------------------------------------------------------

def explore(path, current) :
    if current in path :
        print(path+[current])
    else :
        for e in [ {'source':g.edgesv[i],'target':g.edgesw[i]} for i in range(g.nedges) ] :
            if e['source'] == current :
                explore(path + [current], e['target'])
    
# ------------------------------------------------------------------------------------

def explorePlays(path, current) :
    try :
        index = path.index(current)
        mincolor = min([ g.colors[v] for v in path[index:]])
        for v in path : 
            if mincolor%2 != g.owners[v] :
                targets = [g.edgesw[i] for i in range(g.nedges) if g.edgesv[i]==v]
                differ = list(set(targets).difference(set(targets).intersection(set(path))))
                if len(differ)>0 : 
                    print(" ",path,[current], end=" ")
                    print(mincolor)
                    return
        print("*",path,[current], end=" ")
        print(mincolor)
            
    except ValueError :
        for e in [ {'source':g.edgesv[i],'target':g.edgesw[i]} for i in range(g.nedges) ] :
            if e['source'] == current :
                explorePlays(path + [current], e['target'])
    
# ------------------------------------------------------------------------------------

start = 0
explorePlays([],start)
pass
