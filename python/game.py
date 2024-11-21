import re,json

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