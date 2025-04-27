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
% searching for Plays

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


% predicate dreachability(	int						: nvertices,
% 							array [int] of int		: sources,
% 							array [int] of int		: targets,
% 							array [int] of var bool	: V,
% 							array [int] of var bool	: E,
% 							int				 		: start );

% constraint
%     dreachability(length(owners),sources,targets,V,E,start);

include "globals.mzn";
constraint
	dreachable(nvertices,nedges,sources,targets,start,V,E);

% -------------------------------------------------------------------
% Chanelling path of plays:

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
% Qualifying vertices on the type of play that can force

array[vertices] of var bool : Q;

% If v is even and exists one even edge, from v the play is even
constraint
    forall(v in vertices)(
        (   owners[v] = even /\
            exists(w in vertices,e in edges where sources[e]=v /\ targets[e]=w)(Q[w]=even)
        ) -> Q[v] = even
    );

% If v is odd and exists one odd edge, from v the play is odd
constraint
    forall(v in vertices)(
        (   owners[v] = odd /\
            exists(w in vertices,e in edges where sources[e]=v /\ targets[e]=w)(Q[w]=odd)
        ) -> Q[v] = odd
    );

% If v is even and all edges are odd, from v the play is odd
constraint
    forall(v in vertices)(
        (   owners[v] = even /\
            forall(w in vertices,e in edges where sources[e]=v /\ targets[e]=w)(Q[w]=odd)
        ) -> Q[v] = odd
    );

% If v is odd and all edges are even, from v the play is even
constraint
    forall(v in vertices)(
        (
            owners[v] = odd /\
            forall(w in vertices,e in edges where sources[e]=v /\ targets[e]=w)(Q[w]=even)
        ) -> Q[v] = even
    );

% Vertices and edges not activated do not have winning strategy
% constraint
%     forall(v in vertices)(
%         (not V[v]) -> Q[v] = Q[start]
%     );

% predicate qualify(  array [int] of int		: owners,
%                     array [int] of int		: colors,
%                     array [int] of int		: sources,
%                     array [int] of int		: targets,
%                     array [int] of var bool	: V,
%                     array [int] of var bool	: E,
%                     array [int] of int      : Qe
%                     int				 		: start );

constraint
    Q[P[h]] = (min(v in h..t)(colors[P[v]]) mod 2);

% -------------------------------------------------------------------

constraint
    forall(v in vertices where v in {1,3,5,6})(V[v]) /\
    forall(v in vertices where v in {2})(not V[v]);

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
    "P: " ++ show(P)  ++ " \(h)" ++ " \(t)" ++ "\n" ++
    "Q: " ++ show([bool2int(q) | q in Q]) ++ "\n"
];