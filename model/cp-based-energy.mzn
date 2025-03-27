include "paritygames.mzn";
output["=====SAT=====\n"];

% ----------------------------------------------------------------

set of int              : vertices  = 1..7;
set of int              : edges     = 1..12;
array [vertices] of int	: owners    = [1,0,0,1,0,1,0];
array [vertices] of int	: colors    = [2,1,2,2,4,3,4];
array [edges] of int	: sources   = [1,1,2,3,3,3,4,5,6,6,6,7];
array [edges] of int	: targets   = [2,3,3,1,4,6,3,6,3,5,7,6];
vertices                : start     = 1 ;

% ----------------------------------------------------------------

array [vertices]	of var bool	: V;
array [edges]		of var bool	: E;

% ----------------------------------------------------------------

constraint parity_conditions(owners,colors,sources,targets,V,E,start);

% ----------------------------------------------------------------

array [edges] of int	: energy        = [15,34,-28,-10,5,3,-7,-2,15,34,-28,-10];
int                     : initialcredit = -30;

% PosEnergy
constraint initialcredit + sum(e in edges)(E[e]*energy[e])  >= 0;

% MeanPayoff
