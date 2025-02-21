
def convert(filename) :
    # Read the GM file
    with open(filename+".gm", "r") as f:
        lines = f.readlines()

    # Extract the number of vertices
    # vertices = int(lines[0].split()[1].strip().rstrip(";"))+1

    # Initialize lists
    owners = []
    colors = []
    sources = []
    targets = []

    d = 0
    c = 0

    # Process the lines
    for line in lines[1:]:
        parts = line.strip().rstrip(";").split()
        if len(parts) < 4:
            continue  # Skip malformed lines
        
        v = int(parts[0])  # Vertex ID
        p = int(parts[1])  # Priority (color)
        o = int(parts[2])  # Owner
        successors = parts[3].split(",")  # Successor nodes

        d = v-c

        # Store data
        owners.append(o)
        colors.append(p)
        for t in successors:
            sources.append(v-d+1)
            targets.append(int(t)-d+1)

        c += 1

    # Generate DZN format content
    dzn_content = f"""\
    nvertices = {len(owners)};
    owners    = {owners};
    colors    = {colors};
    nedges    = {len(sources)};
    sources   = {sources};
    targets   = {targets};
    """

    # Save as DZN file
    with open(f"{filename}.dzn", "w") as f:
        f.write(dzn_content)

    print("DZN file saved as output.dzn")


filepath = "/home/chalo/Deleteme/bes-benchmarks/gm/"
filename = "jurdzinskigame_500_500"

convert(filepath+filename)