include "paritygames.mzn";

% -------------------------------------------------------------------

int : nvertices;
int : nedges;
set of int 					: vertices	= 1..nvertices;
set of int 					: edges		= 1..nedges;
array [vertices]	of int	: owners;
array [vertices]	of int	: colors;
array [edges]	 	of int	: sources;
array [edges]		of int	: targets;
vertices 					: start = 1 ;

% -------------------------------------------------------------------

array [vertices]	of var bool	: V;
array [edges]		of var bool	: E;

% -------------------------------------------------------------------

constraint
	parity_conditions(owners,colors,sources,targets,V,E, 1,0,0);

% -------------------------------------------------------------------

output
	["=====SAT===== \n"]
