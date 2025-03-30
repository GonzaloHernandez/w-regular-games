include "paritygames.mzn";

% -------------------------------------------------------------------

int						: nvertices = 7;
int						: nvedges	= 12;
set of int              : vertices  = 1..nvertices;
set of int              : edges     = 1..nvedges;
array [vertices] of int	: owners    = [1,0,0,1,0,1,0];
array [vertices] of int	: colors    = [2,1,2,2,4,3,4];
array [edges] of int	: sources   = [1,1,2,3,3,3,4,5,6,6,6,7];
array [edges] of int	: targets   = [2,3,3,1,4,6,3,6,3,5,7,6];
vertices                : start     = 1;

% -------------------------------------------------------------------

array [vertices]	of var bool	: V;
array [edges]		of var bool	: E;

% -------------------------------------------------------------------

constraint parity_conditions(owners,colors,sources,targets,V,E,start);

% -------------------------------------------------------------------

set of int: steps = 1..nvertices;
array [steps] of var vertices union {0} : Vhistory;
array [steps] of var vertices union {0} : Ehistory;
array [steps] of var vertices union {0} : Vinfinity;
array [steps] of var vertices union {0} : Einfinity;

constraint 
	even_play(	steps,sources,targets,E,start,
				Vhistory,Ehistory, Vinfinity,Einfinity);

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
% int : b = 3;
% array [int] of var int : Vpath
%  = [v | v in Vhistory ++ Vinfinity];
% constraint
% 	forall(i in 1..lenght(Vpath))(
% 		if colors[Vpath[i]] mod 2 == odd then
% 			array[int] of int: chunk = [Vpath[getPos(j)] | j in i..b]
% 			exists(j in i+1..i+b)(
% 				colors[Vpath[j]] mod 2 == even /\ colors[v] < colors[Vpath[i]]
% 			)
% 	);

% -------------------------------------------------------------------

output
	[toStr(Vhistory) ++ toStr(Vinfinity) ++ " \n"] ++ 
	["b = \(getPos(Vhistory,Vinfinity,5))"]

	% [toStr(Vhistory) ++ toStr(Vinfinity) ++ "\n"] ++
	% ["energy = \(EL) \n"] ++
	% ["V = " ++ toStr(V) ++ "\n"] ++
	% ["E = " ++ toStr(E) ]
