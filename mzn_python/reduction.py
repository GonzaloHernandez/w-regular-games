import os; os.system("clear")
import warnings,time,sys
warnings.filterwarnings("error", message="model inconsistency detected")
arg = sys.argv

# ----------------------------------------------------

sys.path.insert(1,".")
from pythonsrc import Game
from minizinc import Instance, Model, Solver

driver          = ["gecode","chuffed","cpsatlp","chuffed-noc"]

def compute(d,levels,blocks) :

    solver      = Solver.lookup(driver[d])

    # model       = Model("./mzn_model/reduction-cp.mzn")
    model       = Model("./mzn_model/reduction-sat.mzn")

    instance    = Instance(solver, model)

    instance["nvertices"]   = g.nvertices
    instance["nedges"]      = g.nedges
    instance["owners"]      = g.owners
    instance["colors"]      = g.colors
    instance["sources"]     = g.sources
    instance["targets"]     = g.targets
    instance["start"]       = g.start

    sat = False

    t1 = time.time()
    t2 = time.time()
    try :
        response = instance.solve()
        t2 = time.time()

        if response.solution is None :
            sat = False
            print("Unsatisfiable (After seraching)")
        else :
            sat = True
            # print(response.statistics)
            print("V =",response["Va"])
            print("E =",response["Ea"])
            # print("M =",response["pmeasures"])
    except Warning as e :
        sat = False
        print("Unsatisfiable (Inconsistency detected)")

    return (t2-t1),(1 if sat else 0)

# ----------------------------------------------------

# g = Game(Game.RANDOM,10)
# g = Game(Game.JURDZINSKI,3,2)
g = Game('./data/jurd-2-1.dzn')

# print(g)
g.start = 1
d       = 1 #int(arg[1])
levels  = 2 #int(arg[2])
blocks  = 1 #int(arg[3])

tim,sat = compute(d,levels,blocks)

print(f"{driver[d]},{levels},{blocks},{1 if sat else 0},{tim}")
