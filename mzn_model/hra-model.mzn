int : even  = 0;
int : odd   = 1;

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
% Connecting the graph

% Activating first vertex
constraint
    V[start] = true;

% For every vertice needs to have one outgoing edge activated
constraint
    forall(v in vertices)(
        V[v] -> exists(e in edges where sources[e]=v)(E[e])
    );

% For every activated edge, the target vertex must be activated
constraint
    forall(e in edges)(
        E[e] -> V[targets[e]]
    );

% For every vertice, one incoming edge must be activated
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
% Qualifying vertices on the type of play that can force

array[vertices] of var bool : Q;
array[vertices] of var bool : C;

% If v is EVEN and exists one EVEN edge, from v the play is EVEN
constraint
    forall(v in vertices)(
        (   owners[v] = even /\
            (exists(w in vertices,e in edges where sources[e]=v /\ targets[e]=w)(Q[w]=even) \/ C[v]=even)
        ) -> Q[v] = even
    );

% If v is ODD and exists one ODD edge, from v the play is ODD
constraint
    forall(v in vertices)(
        (   owners[v] = odd /\
            (exists(w in vertices,e in edges where sources[e]=v /\ targets[e]=w)(Q[w]=odd) \/ C[v]=odd)
        ) -> Q[v] = odd
    );

% If v is EVEN and all edges are ODD, from v the play is ODD
constraint
    forall(v in vertices)(
        (   owners[v] = even /\
            forall(w in vertices,e in edges where sources[e]=v /\ targets[e]=w)(Q[w]=odd)
        ) -> Q[v] = odd
    );

% If v is ODD and all edges are EVEN, from v the play is EVEN
constraint
    forall(v in vertices)(
        (
            owners[v] = odd /\
            forall(w in vertices,e in edges where sources[e]=v /\ targets[e]=w)(Q[w]=even)
        ) -> Q[v] = even
    );

% % Vertices not activated do not have qualifying play
% constraint
%     forall(v in vertices)(
%         not V[v] -> (Q[v]=Q[start])
%     );

predicate qualify(  array [int] of int		: owners,
                    array [int] of int		: colors,
                    array [int] of int		: sources,
                    array [int] of int		: targets,
                    array [int] of var bool	: V,
                    array [int] of var bool	: E,
                    array [int] of var bool	: Q,
                    array [int] of var bool	: C,
                    int				 		: start );

constraint
    qualify(owners,colors,sources,targets,V,E,Q,C,start);

% -------------------------------------------------------------------

% constraint
%     forall(v in vertices where v in {1,3,5,6})(V[v]) /\
%     forall(v in vertices where v in {2})(not V[v]);

% constraint
%     Q[3] = 0;

% -------------------------------------------------------------------

function string : toStr(array [int] of var bool	: X) =
	let {
		array [int]	of int : Xa ::output_only ::output = 
			[x | x in 1..length(X)	where fix(X[x])];
	} in "\(Xa)";

output [
    "V: " ++ toStr(V) ++ "\n" ++ 
    "E: " ++ toStr(E) ++ "\n" ++ 
    "Q: " ++ show([bool2int(q) | q in Q]) ++ "\n" ++
    "C: " ++ show([bool2int(c) | c in C]) ++ "\n"
];