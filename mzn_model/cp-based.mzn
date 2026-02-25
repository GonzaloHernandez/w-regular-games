include "paritygames.mzn";

% -------------------------------------------------------------------

int : nvertices;
int : nedges;
set of int 				: vertices	= 1..nvertices;
set of int 				: edges		= 1..nedges;
array [vertices] of int	: owners;
array [vertices] of int	: priors;
array [edges] of int	: sources;
array [edges] of int	: targets;
	
vertices 				: init = 1 ;

% -------------------------------------------------------------------

array [vertices]	of var bool	: V;
array [edges]		of var bool	: E;

% -------------------------------------------------------------------

constraint
	parity_conditions(	owners,priors,sources,targets,
						init,enum2int(EVEN),enum2int(MIN),
						V,E);
						

% -------------------------------------------------------------------

output
	["=====SAT===== \n"]
