import os; os.system("clear")
import warnings,time,sys,re,json
warnings.filterwarnings("error", message="model inconsistency detected")
arg = sys.argv

# ----------------------------------------------------

nvertices = 0
owners = []
colors = []

nedges = 0
edgesv = []
edgesw = []

with open('./data/game-sat.dzn', 'r') as file:
    for line in file:
        if line.strip() == "" : continue

        key, value = [item.strip() for item in re.sub(r"[;]","",line).split('=')]
        
        if      key=="nvertices"    : nvertices = json.loads(value)
        elif    key=="owners"       : owners    = json.loads(value)
        elif    key=="colors"       : colors    = json.loads(value)
        elif    key=="nedges"       : nedges    = json.loads(value)
        elif    key=="edgesv"       : edgesv    = json.loads(value)
        elif    key=="edgesw"       : edgesw    = json.loads(value)

# ----------------------------------------------------

from minizinc import Instance, Model, Solver

d           = 0
driver      = ["gecode","chuffed"]
solver      = Solver.lookup(driver[d])
model       = Model("./model/reductionsat.mzn")
instance    = Instance(solver, model)

instance["nvertices"]   = nvertices
instance["nedges"]      = nedges
instance["owners"]      = owners
instance["colors"]      = colors
instance["edgesv"]      = edgesv
instance["edgesw"]      = edgesw
instance["start"]       = 1

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
        print("M =",response["pmeasures"])

except Warning as e :
    sat = False
    print("Unsatisfiable (Inconsistency detected)")

print(f"{driver[d]},{t2-t1}")

# print(f"nvertices = {nvertices}")
# print(f"owners = {owners}")
# print(f"colors = {colors}")

# print(f"nedges = {nedges}")
# print(f"edgesv = {edgesv}")
# print(f"edgesw = {edgesw}")
