import os; os.system("clear")
import warnings,time,sys
warnings.filterwarnings("error", message="model inconsistency detected")
arg = sys.argv

# ----------------------------------------------------

def createGame(levels, blocks) -> list :

    nvertices   = ((blocks*3)+1)*(levels-1) + ((blocks*2)+1)
    owners      = []
    colors      = []

    nedges      = (blocks*6)*(levels-1) + (blocks*4) + (blocks*2*(levels-1))
    edgesv      = []
    edgesw      = []

    e = 1   # even blocks sequence
    o = 0   # odd blocks sequence
    for l in range(1,levels) :
        o = ((blocks*3)+1)*(levels-1)+1

        for b in range(1,blocks+1) : 
            owners += [  1,     0,   0]
            colors += [l*2, l*2-1, l*2]
            
            edgesv += [  e, e+1, e+2,   e, e+2, e+3, e+2, o+1]
            edgesw += [e+1, e+2,   e, e+2, e+3, e+2, o+1, e+2]
            e += 3
            o += 2

        owners += [  1]
        colors += [l*2]
        e += 1

    l = levels
    for b in range(blocks) :
        owners += [  0,     1]
        colors += [l*2, l*2-1]
        
        edgesv += [  e, e+1, e+1, e+2]
        edgesw += [e+1,   e, e+2, e+1]
        e += 2 

    owners += [  0]
    colors += [levels*2]
    return [nvertices,owners,colors,nedges,edgesv,edgesw]

# ----------------------------------------------------

from minizinc import Instance, Model, Solver

driver          = ["gecode","chuffed","cpsatlp"]

def compute(d,levels,blocks) :

    solver      = Solver.lookup(driver[d])

    # solver = Solver(
    #     name="MyChuffed",
    #     version="0.0",
    #     id="org.sonar.mychuffed",
    #     mznlib="/home/chalo/.local/share/minizinc/chuffed",
    #     executable="/home/chalo/.local/bin/fzn-chuffed",
    # )

    # solver = Solver(
    #     name="Gecode",
    #     version="6.3.0",
    #     id="org.gecode.gecode",
    #     executable="/opt/minizinc-ide/bin/fzn-gecode",
    #     # mznlib="/opt/minizinc-ide/share/minizinc/gecode",
    # )

    # solver.stdFlags=["-a", "-n", "-s", "-v", "-r", "-f", "-t"]
    
    model       = Model("./model/reductionsat-novel.mzn")
    instance    = Instance(solver, model)

    [nvertices,owners,colors,nedges,edgesv,edgesw] = createGame(levels,blocks)

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
        response = instance.solve(intermediate_solutions=False)
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

    # print(f"nvertices = {nvertices}")
    # print(f"owners = {owners}")
    # print(f"colors = {colors}")

    # print(f"nedges = {nedges}")
    # print(f"edgesv = {edgesv}")
    # print(f"edgesw = {edgesw}")

    return (t2-t1),(1 if sat else 0)

# ----------------------------------------------------

d       = 0 #int(arg[1])
levels  = 2 #int(arg[2])
blocks  = 1 #int(arg[3])

tim,sat = compute(d,levels,blocks)

print(f"{driver[d]},{levels},{blocks},{1 if sat else 0},{tim}")
