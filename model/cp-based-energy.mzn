include "paritygames.mzn";

% -------------------------------------------------------------------

set of int              : vertices  = 1..7;
set of int              : edges     = 1..12;
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

array [vertices] of var vertices union {0} : Vhistory;
array [vertices] of var vertices union {0} : Ehistory;
array [vertices] of var vertices union {0} : Vinfinity;
array [vertices] of var vertices union {0} : Einfinity;

constraint play(vertices,sources,targets,E,start,
				Vhistory,Ehistory, Vinfinity,Einfinity);

% -------------------------------------------------------------------

array[edges] of int : energy = [+5,-10,+3,+8,11,+5,-10,+3,+8,-11,6,9];
int : initialcredit = 30;
var int : EL = initialcredit 
			 + sum(e in edges where e in Ehistory ++ Einfinity)(energy[e]);

constraint
	sum(e in edges where e in Einfinity)(energy[e]) > 0;

% solve maximize EL;

% -------------------------------------------------------------------

output
	[toStr(Vhistory) ++ toStr(Vinfinity) ++ " \(EL)\n"]

	% [toStr(Vhistory) ++ toStr(Vinfinity) ++ "\n"] ++
	% ["energy = \(EL) \n"] ++
	% ["V = " ++ toStr(V) ++ "\n"] ++
	% ["E = " ++ toStr(E) ]
