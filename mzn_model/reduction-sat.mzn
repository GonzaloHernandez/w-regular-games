int : even  = 0;
int : odd   = 1;
int : nvertices;
int : nedges;
set of int : vertices	= 1..nvertices;
set of int : edges		= 1..nedges;
array [vertices] of int	: owners;
array [vertices] of int	: colors;
array [edges] of int	: sources;
array [edges] of int	: targets;

vertices : start = 1;

% ----------------------------------------------------------------------

array [vertices]	of var bool	: V;
array [edges]		of var bool	: E;

% ------v----------------------------------------------------------------

constraint
	V[start] = true;

constraint   
	forall(v in vertices where owners[v]==even)(
		V[v] ->	exists(e in edges where sources[e]==v)(E[e])
	);

constraint
	forall(v in vertices where owners[v]==odd)(
		V[v] ->	forall(e in edges where sources[e]==v)(E[e])
	);

constraint
	forall(w in vertices where w != start)(
		exists(e in edges where targets[e]==w)(E[e])	-> V[w]
	);

constraint
	forall(e in edges)(
		E[e] -> measureprogress(e)
	);

% ----------------------------------------------------------------------

array [int] of int	: original_oddcolors	= [ c | c in colors where c mod 2 == 1 ];
set of int			: set_oddcolors			= { c | c in original_oddcolors };
array [int] of int	: oddcolors				= [ c | c in set_oddcolors ];
int					: mop					= max(oddcolors);

array [1..mop] of 0..mop : oddcolorspos = [		
	if i in oddcolors then indexof(oddcolors,i) else 0 endif | i in 1..mop];

function $$E: indexof( array [$$E] of $$V: x, $$V: v ) =
	min(i in index_set(x) where x[i] = v)(i);
	
array [vertices,1..length(oddcolors)] of var vertices : pmeasures;

% ----------------------------------------------------------------------

predicate measureprogress(edges : e) =

	let {	vertices	: v = sources[e],
			vertices	: w = targets[e] } in
	
	if colors[w] mod 2 == 0 then
		forall(p in oddcolors where p < colors[w])(
			pmeasures[v,oddcolorspos[p]] >= pmeasures[w,oddcolorspos[p]]
		)
	else
		pmeasures[v,oddcolorspos[colors[w]]] > pmeasures[w,oddcolorspos[colors[w]]] /\
		forall(p in oddcolors where p < colors[w])(
			pmeasures[v,oddcolorspos[p]] >= pmeasures[w,oddcolorspos[p]]
		)
	endif;

% ----------------------------------------------------------------------

array [int]	of int : Va ::output_only ::output = [v | v in vertices	where fix(V[v])];
array [int] of int : Ea ::output_only ::output = [e | e in edges 	where fix(E[e])];

solve ::bool_search(V++E,input_order,indomain_max) satisfy ; 

output
% 	["V  = \(V) \n"] ++
% 	["E  = \(E) \n"] ++
	["Va = \(Va) \n"] ++
	["Ea = \(Ea) \n"] ++
% 	["O  = \(oddcolors) \n"] ++
% 	["Op = \(oddcolorspos) \n"] ++
	["P  = "] ++ ["\(pmeasures[v,..]) " | v in vertices] ++
	[]
	
