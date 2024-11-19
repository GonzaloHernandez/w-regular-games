import os; os.system("clear")
import warnings,time,sys,random,math,statistics
warnings.filterwarnings("error", message="model inconsistency detected")
arg = sys.argv

# ----------------------------------------------------

def createGame(n) -> list :

    nvertices   = n
    owners      = []
    colors      = []

    nedges      = n*2
    edgesv      = []
    edgesw      = []

    m = math.ceil(math.sqrt(n))

    for v in range(1,nvertices+1) :
        owners.append(random.randint(0,1))
        colors.append(random.randint(0,m))
        for _ in range(2) :
            edgesv.append(v)
            edgesw.append(random.randint(1,nvertices))

    return [nvertices,owners,colors,nedges,edgesv,edgesw]

# ----------------------------------------------------

from minizinc import Instance, Model, Solver

driver      = ["gecode","chuffed"]

def compute(d,n) :

    solver      = Solver.lookup(driver[d])
    model       = Model("./model/reductionsat.mzn")
    instance    = Instance(solver, model)

    [nvertices,owners,colors,nedges,edgesv,edgesw] = createGame(n)

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
        response = instance.solve(free_search=True)
        t2 = time.time()

        if response.solution is None :
            sat = False
        else :
            sat = True
            # print("V =",response["Va"])
            # print("E =",response["Ea"])
            # print("M =",response["pmeasures"])
    except Warning as e :
        sat = False

    # print(f"nvertices = {nvertices}")
    # print(f"owners = {owners}")
    # print(f"colors = {colors}")

    # print(f"nedges = {nedges}")
    # print(f"edgesv = {edgesv}")
    # print(f"edgesw = {edgesw}")

    return (t2-t1),(1 if sat else 0)

# ----------------------------------------------------

d       = 0 #int(arg[1])
n       = 10 #int(arg[2])

tim,sat = compute(d,n)
print(f"{driver[d]},{n},{1 if sat else 0},{tim}")

# times = []
# sats = []
# for i in range(10) :
#     result = compute(d,n)
#     times.append(result[0])
#     sats.append(result[1])

# print(f"{driver[d]},{n},{sats.count(1)},{sats.count(0)},{min(times)},{statistics.median(times)},{max(times)}")

