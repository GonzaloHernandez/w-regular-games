import re,json
import copy

# ------------------------------------------------------------------------------------

class Game :
    FIRST0      = True
    FIRST1      = False

    JURDZINSKI  = 11
    RANDOM      = 12

    EVEN    = 0
    ODD     = 1
    
    def __init__(self, *args, **kwargs) -> None:

        # ---------------------------------------------------------
        if len(args) == 0 and not kwargs :
            self.nvertices  = 1
            self.vertices   = [0]
            self.owners     = [0]
            self.colors     = [0]
            self.nedges     = [0]
            self.edges      = []
            self.sources    = []
            self.targets    = []

        # ---------------------------------------------------------
        elif len(args) <= 2 and isinstance(args[0],str):
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
            fixZero = args[1] if len(args)==2 else self.FIRST1
            if fixZero :
                self.vertices   = [v for v in range(self.nvertices)]
                self.edges      = [e for e in range(self.nedges)]
                self.start      = 0
                self.fixZeroPosition()
            else :
                self.vertices   = [v for v in range(1,self.nvertices+1)]
                self.edges      = [e for e in range(1,self.nedges+1)]
                self.start      = 1

        # ---------------------------------------------------------
        elif len(args) <= 3 and args[0] == self.RANDOM :
            import math,random

            n  = args[1]

            self.nvertices   = n
            self.owners      = []
            self.colors      = []

            self.nedges      = n*2
            self.sources     = []
            self.targets     = []

            m = math.ceil(math.sqrt(n))

            fixZero = args[2] if len(args)==3 else self.FIRST1
            if fixZero :
                self.vertices   = [v for v in range(self.nvertices)]
                self.edges      = [e for e in range(self.nedges)]
                self.start      = 0
                self.fixZeroPosition()
            else :
                self.vertices   = [v for v in range(1,self.nvertices+1)]
                self.edges      = [e for e in range(1,self.nedges+1)]
                self.start      = 1

            for v in self.vertices :
                self.owners.append(random.randint(0,1))
                self.colors.append(random.randint(0,m))
                for _ in range(2) :
                    self.sources.append(v)
                    self.targets.append(random.randint(1,self.nvertices))


        # ---------------------------------------------------------
        elif len(args) <= 4 and args[0] == self.JURDZINSKI :
            levels  = args[1]
            blocks  = args[2]
            self.nvertices   = ((blocks*3)+1)*(levels-1) + ((blocks*2)+1)
            self.owners      = []
            self.colors      = []

            self.nedges      = (blocks*6)*(levels-1) + (blocks*4) + (blocks*2*(levels-1))
            self.sources     = []
            self.targets     = []

            es = 1  # even blocks sequence
            os = 0  # odd blocks sequence
            for l in range(1,levels) :
                os = ((blocks*3)+1)*(levels-1)+1

                for b in range(blocks) : 
                    self.owners += [  1,     0,   0]
                    self.colors += [l*2, l*2-1, l*2]
                    
                    self.sources += [  es,   es, es+1, es+2, es+2, es+3, es+2, os+1]
                    self.targets += [es+1, es+2, es+2,   es, es+3, es+2, os+1, es+2]
                    es += 3
                    os += 2

                self.owners += [  1]
                self.colors += [l*2]
                es += 1

            l = levels
            for b in range(blocks) :
                self.owners += [  0,     1]
                self.colors += [l*2, l*2-1]
                
                self.sources += [  es, es+1, es+1, es+2]
                self.targets += [es+1,   es, es+2, es+1]
                es += 2 

            self.owners += [  0]
            self.colors += [levels*2]

            fixZero = args[3] if len(args)==4 else self.FIRST1
            if fixZero :
                self.vertices   = [v for v in range(self.nvertices)]
                self.edges      = [e for e in range(self.nedges)]
                self.start      = 0
                self.fixZeroPosition()
            else :
                self.vertices   = [v for v in range(1,self.nvertices+1)]
                self.edges      = [e for e in range(1,self.nedges+1)]
                self.start      = 1

        # ---------------------------------------------------------
        elif len(args) == 8 :
            game = copy.deepcopy(args)
            self.nvertices  = game[0]
            self.vertices   = game[1]
            self.owners     = game[2]
            self.colors     = game[3]
            self.nedges     = game[4]
            self.edges      = game[5]
            self.sources    = game[6]
            self.targets    = game[7]
            self.start      = 0

    def __str__(self) -> str:
        text = ""
        text += f"nvertices = {self.nvertices}\n"
        text += f"vertices  = {self.vertices}\n"
        text += f"owners    = {self.owners}\n"
        text += f"colors    = {self.colors}\n"
        text += f"nedges    = {self.nedges}\n"
        text += f"edges     = {self.edges}\n"
        text += f"sources   = {self.sources}\n"
        text += f"targets   = {self.targets}\n"
        return text

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
        return [e for e in self.edges if e != None and self.sources[e] in vs and self.targets[e] in vs]

    def fixZeroPosition(self) :
        for e in range(self.nedges) :
            self.sources[e] -= 1
            self.targets[e] -= 1
    
    def __truediv__(self, novertices) :
        g = self.copy()
        for v in novertices :
            g.vertices[v] = None
            for e in g.edges :
                if e != None :
                    if g.sources[e] == v or g.targets[e] == v :
                        g.edges[e]  = None
        return g
