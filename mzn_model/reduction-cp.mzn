include "paritygames.mzn";

int : nvertices;
int : nedges;
set of int : vertices	= 1..nvertices;
set of int : edges		= 1..nedges;
array [vertices] of int	: owners;
array [vertices] of int	: colors;
array [edges] of int	: sources;
array [edges] of int	: targets;

% -------------------------------------------------------------------------------------

array [vertices]	of var bool	: V;
array [edges]		of var bool	: E;

vertices : start = 1 ;
int	: reward = 0;			% 0=look for MIN priority, 1=look for MAX priority
int : player = 0;			% 0=EVEN player perspective, 1=ODD player perspective
int : opponent = 1-player;

% -------------------------------------------------------------------------------------

predicate fixVertices(array [int] of vertices : vs, array [int] of vertices : nvs) =
	forall(v in vs)(V[v]) /\ forall(v in nvs)(not V[v]);

predicate fixEdges(array [int] of edges : es, array [int] of edges: nes) =
	forall(e in es)(E[e]) /\ forall(e in nes)(not E[e]);

% -------------------------------------------------------------------------------------

% constraint
% 	fixVertices([1,2,3,4],[5,6,7]);

% constraint
% 	fixEdges([1,3,5,7],[2,4,9,6,8,10,11,12]);

constraint
	parity_conditions(owners,colors,sources,targets,V,E, 1,0,0);	

% -------------------------------------------------------------------------------------

array [int]	of int : Va ::output_only ::output = [v | v in vertices	where fix(V[v])];
array [int]	of int : Ea ::output_only ::output = [e | e in edges	where fix(E[e])];

% solve ::bool_search(V++E,input_order,indomain_max) satisfy ; 

output
% 	["V  = \(V) \n"] ++
% 	["E  = \(E) \n"] ++
	["Va = \(Va) \n"] ++
	["Ea = \(Ea) \n"] ++
	[]
