
def convert(filename) :
    # Read the GM file
    with open(filename, "r") as f:
        lines = f.readlines()

    # Extract the number of vertices
    vertices = int(lines[0].split()[1].strip().rstrip(";"))

    # Initialize lists
    owners = []
    colors = []
    edges   = 0
    sources = []
    targets = []

    # Process the lines
    for line in lines[1:]:
        parts = line.strip().rstrip(";").split()
        if len(parts) < 4:
            continue  # Skip malformed lines
        
        v = int(parts[0])  # Vertex ID
        p = int(parts[1])  # Priority (color)
        o = int(parts[2])  # Owner
        successors = parts[3].split(",")  # Successor nodes

        # Store data
        owners.append(o)
        colors.append(p)
        for t in successors:
            sources.append(v)
            targets.append(int(t))
            edges += 1


    # Generate DZN format content
    dzn_content = f"""\
    nvertices = {vertices};
    owners    = {owners};
    colors    = {colors};
    nedges    = {len(edges)};
    sources   = {list(sources)};
    targets   = {list(targets)};
    """

    # Save as DZN file
    with open(f"{filename}.dzn", "w") as f:
        f.write(dzn_content)

    print("DZN file saved as output.dzn")

convert("/home/chalo/Deleteme/bes-benchmarks/randomgame_5000_10_1_20_id_24.gm")