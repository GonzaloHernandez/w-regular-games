
% -------------------------------------------------------------------

int						: nvertices;
int						: nedges;
set of int              : vertices  = 1..nvertices;
set of int              : edges     = 1..nedges;
array [vertices] of int	: owners;
array [vertices] of int	: colors;
array [edges] of int	: sources;
array [edges] of int	: targets;
vertices                : start     = 1;

% -------------------------------------------------------------------

array [vertices]	of var bool	: V;
array [edges]		of var bool	: E;

% -------------------------------------------------------------------

constraint
    V[start] = true;

constraint
    forall(v in vertices)(
        V[v] -> exists(e in edges where sources[e]=v)(E[e])
    );

constraint
    forall(v in vertices)(
        V[v] ->
            forall(e1,e2 in edges where 
            sources[e1]=v /\ sources[e2]=v /\ e1<e2)
            (
                not E[e1] \/ not E[e2]
            )
    );

constraint
    forall(e in edges)(
        E[e] -> V[targets[e]]
    );

constraint
    forall(w in vertices where w != start)(
        exists(e in edges where targets[e]=w)(V[w] -> E[e])
    );


predicate dreachability(	int						: nvertices,
							array [int] of int		: sources,
							array [int] of int		: targets,
							array [int] of var bool	: V,
							array [int] of var bool	: E,
							int				 		: start );

constraint
    dreachability(length(owners),sources,targets,V,E,start);

% -------------------------------------------------------------------
% Chanelling path:

array [vertices] of var vertices union {0}: P;
var vertices: t;
var vertices: h;

constraint
    P[1] = start;

constraint
    forall(i in 2..t)(
        exists(e in edges)(
            E[e] /\ sources[e]=P[i-1] /\ targets[e]=P[i]
        )
    );

constraint
    forall(i in t+1..nvertices)(
        P[i] = 0
    );

constraint
    t = sum(v in vertices)(V[v])
    /\
    exists(e in edges)(
        E[e] /\ sources[e]=P[t] /\ targets[e]=P[h]
    );

    
% -------------------------------------------------------------------

function string : toStr(array [int] of var bool	: X) =
	let {
		array [int]	of int : Xa ::output_only ::output = 
			[x | x in 1..length(X)	where fix(X[x])];
	} in "\(Xa)";

output [
    "V: " ++ toStr(V) ++ "\n" ++ 
    "E: " ++ toStr(E) ++ "\n" ++ 
    "P: " ++ show(P) ++ " \(t)" ++ " \(h)"
];