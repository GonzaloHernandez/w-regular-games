include "paritygames.mzn";

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

constraint parity_conditions(owners,colors,sources,targets,V,E, 1,0,0);

% -------------------------------------------------------------------

% Synthesis
set of int: steps = 1..nvertices;
array [steps] of var vertices union {0} : Vhistory;
array [steps] of var vertices union {0} : Ehistory;
array [steps] of var vertices union {0} : Vinfinity;
array [steps] of var vertices union {0} : Einfinity;
array [steps] of var vertices union {0} : Vpath;
array [steps] of var vertices union {0} : Epath;

constraint 
	even_play(	steps,sources,targets,E,start,
				Vhistory,Ehistory, Vinfinity,Einfinity, Vpath, Epath );

% -------------------------------------------------------------------

% % Energy objective
% array[edges] of int : energy = [+5,-10,+3,+8,11,+5,-10,+3,+8,-11,6,9];
% int : initialcredit = 30;
% constraint
% 	sum ( e in edges where e in Einfinity )( energy [ e ]) >= 0;

% -------------------------------------------------------------------

% % Prompt objective
% int : b = 3;
% constraint
% 	count(v in vertices)(v in Vhistory ++ Vinfinity) <= b;

% -------------------------------------------------------------------

% % Finitary parity condition
% int : b = 1;
% var int: len = count(v in Vpath)(v!=0)-1;
% constraint
% 	forall(i in 1..len-b)(
% 		if colors[Vpath[i]] mod 2 == odd then
% 			exists(v in [ Vpath[getPos(Vhistory,Vinfinity,j)] | j in i+1..i+b])(
% 				colors[v] mod 2 == even /\ colors[v] < colors[Vpath[i]]
% 			)
% 		endif
% 	);

% -------------------------------------------------------------------

output
	[toStr(V) ++ "\n" ++ toStr(E) ++ " \n"] ++
	["play = " ++ toStr(Vhistory) ++ toStr(Vinfinity) ++ " \n"] ++ 
	[]
