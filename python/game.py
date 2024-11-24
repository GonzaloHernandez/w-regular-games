import re,json



# ------------------------------------------------------------------------------------

class Game :
    EVEN    = 0
    ODD     = 1
    
    def __init__(self, *args, **kwargs) -> None:

        if len(args) == 0 and not kwargs :
            self.nvertices  = 1
            self.vertices   = [0]
            self.owners     = [0]
            self.colors     = [0]
            self.nedges     = [0]
            self.edges      = []
            self.sources    = []
            self.targets    = []

        elif len(args) == 1 and isinstance(args[0],str):
            filename = args[0]
            with open(filename, 'r') as file:
                for line in file:
                    if line.strip() == "" : continue

                    key, value = [item.strip() for item in re.sub(r"[;]","",line).split('=')]
                    
                    if      key=="nvertices"    : self.nvertices = json.loads(value)
                    elif    key=="owners"       : self.owners    = json.loads(value)
                    elif    key=="colors"       : self.colors    = json.loads(value)
                    elif    key=="nedges"       : self.nedges    = json.loads(value)
                    elif    key=="sources"      : self.sources   = json.loads(value)
                    elif    key=="targets"      : self.targets   = json.loads(value)
            self.vertices   = [v for v in range(self.nvertices)]
            self.edges      = [e for e in range(self.nedges)]
            self.start      = 0
            self.fixZeroPosition()

        elif len(args) == 8 :
            self.nvertices  = args[0]
            self.vertices   = args[1]
            self.owners     = args[2]
            self.colors     = args[3]
            self.nedges     = args[4]
            self.edges      = args[5]
            self.sources    = args[6]
            self.targets    = args[7]
            self.start      = 0

    def copy(self) :
        return Game(self.nvertices,
                    self.vertices,
                    self.owners,
                    self.colors,
                    self.nedges,
                    self.edges,
                    self.sources,
                    self.targets)
    
    def getVertices(self) -> list :
        return [v for v in self.vertices if v != None]

    def getEdges(self) -> list :
        vs = self.getVertices()
        return [e for e in self.edges if self.sources[e] in vs and self.targets[e] in vs]

    def fixZeroPosition(self) :
        for e in range(self.nedges) :
            self.sources[e] -= 1
            self.targets[e] -= 1
    
    def __truediv__(self, novertices) :
        g = self.copy()
        for v in novertices :
            g.vertices[v] = None
            g.owners[v] = None
            g.colors[v] = None
            for e in g.edges :
                if e != None :
                    if g.sources[e] == v :
                        g.sources[e] = None
                        g.targets[e] = None
                    if g.targets[e] == v :                    
                        g.sources[e] = None
                        g.targets[e] = None
        return g
