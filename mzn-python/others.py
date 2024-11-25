import os; os.system("clear")
import warnings,time,sys
warnings.filterwarnings("error", message="model inconsistency detected")
arg = sys.argv

# ----------------------------------------------------

# nvertices = 0
# owners = []
# colors = []

# nedges = 0
# sources = []
# targets = []

# with open('./data/game-jurdzinski-4-3.dzn', 'r') as file:
#     for line in file:
#         if line.strip() == "" : continue

#         key, value = [item.strip() for item in re.sub(r"[;]","",line).split('=')]
        
#         if      key=="nvertices"    : nvertices = json.loads(value)
#         elif    key=="owners"       : owners    = json.loads(value)
#         elif    key=="colors"       : colors    = json.loads(value)
#         elif    key=="nedges"       : nedges    = json.loads(value)
#         elif    key=="sources"       : sources    = json.loads(value)
#         elif    key=="targets"       : targets    = json.loads(value)

# ----------------------------------------------------

import game
g = game.Game('./data/game-jurdzinski-4-3.dzn')

from minizinc import Instance, Model, Solver

d           = 0
driver      = ["gecode","chuffed"]
solver      = Solver.lookup(driver[d])
model       = Model("./model/reductionsat-novel.mzn")
instance    = Instance(solver, model)

instance["nvertices"]   = g.nvertices
instance["nedges"]      = g.nedges
instance["owners"]      = g.owners
instance["colors"]      = g.colors
instance["sources"]     = g.sources
instance["targets"]     = g.targets
instance["start"]       = 25

sat = False

try :
    t1 = time.time()
    response = instance.solve()
    t2 = time.time()

    if response.solution is None :
        sat = False
        print("Unsatisfiable (After seraching)")
    else :
        sat = True
        print("V =",response["Va"])
        print("E =",response["Ea"])
        # print("M =",response["pmeasures"])

except Warning as e :
    sat = False
    print("Unsatisfiable (Inconsistency detected)")

print(f"{driver[d]},{t2-t1}")
