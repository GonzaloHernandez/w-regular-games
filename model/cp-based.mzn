int : nvertices;
int : nedges;
set of int : vertices	= 1..nvertices;
set of int : edges		= 1..nedges;
array [vertices]	of int	: owners;
array [vertices]	of int	: colors;
array [edges]	 	of int	: sources;
array [edges]		of int	: targets;

vertices : start = 1 ;

% -------------------------------------------------------------------------------------

array [vertices]	of var bool	: V;
array [edges]		of var bool	: E;

% -------------------------------------------------------------------------------------

include "paritygames.mzn";
constraint
	parity_conditions(owners,colors,sources,targets,V,E,start);

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
