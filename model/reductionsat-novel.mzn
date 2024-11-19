int : even  = 0;
int : odd   = 1;
int : nvertices;
int : nedges;
set of int : vertices	= 1..nvertices;
set of int : edges		= 1..nedges;
array [vertices] of int	: owners;
array [vertices] of int	: colors;
array [edges] of int	: edgesv;
array [edges] of int	: edgesw;

vertices : start = 1;

% -------------------------------------------------------------------------------------

array [vertices]	of var bool	: V;
array [edges]		of var bool	: E;

% -------------------------------------------------------------------------------------

predicate fixVertices(array [int] of vertices : vs, array [int] of vertices : nvs) =
	forall(v in vs)(V[v]) /\ forall(v in nvs)(not V[v]);

predicate fixEdges(array [int] of edges : es, array [int] of edges: nes) =
	forall(e in es)(E[e]) /\ forall(e in nes)(not E[e]);

% -------------------------------------------------------------------------------------

% constraint
% 	fixVertices([1,2,4],[3,5,6,7]);

% constraint
% 	fixEdges([1,2,3,7],[4,5,6]);

constraint
	V[start] = true;

constraint   
	forall(v in vertices where owners[v]==even)(
		V[v] ->	exists(e in edges where edgesv[e]==v)(E[e])
	);

constraint
	forall(v in vertices where owners[v]==odd)(
		V[v] ->	forall(e in edges where edgesv[e]==v)(E[e])
	);

constraint
	forall(w in vertices where w != start)(
		exists(e in edges where edgesw[e]==w)(E[e]) -> V[w]
	);

include "cyclefilter.mzn";

constraint
	globalmeasureprogress(owners,colors,edgesv,edgesw,V,E);

% constraint
% 	cycle_filter(owners,colors,edgesv,edgesw,V,E);

% -------------------------------------------------------------------------------------

array [int]	of int : Va ::output_only ::output = [v | v in vertices	where fix(V[v])];
array [int]	of int : Ea ::output_only ::output = [e | e in edges		where fix(E[e])];

% solve ::bool_search(V++E,input_order,indomain_max) satisfy ; 

output
% 	["V  = \(V) \n"] ++
% 	["E  = \(E) \n"] ++
	["Va = \(Va) \n"] ++
	["Ea = \(Ea) \n"] ++
	[]
