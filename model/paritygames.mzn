int : even  = 0;
int : odd   = 1;

% --------------------------------------------------------------------------

predicate nooddcycle(	array [int] of int		: owners,
						array [int] of int		: colors,
						array [int] of int		: sources,
						array [int] of int		: targets,
						array [int] of var bool	: V,
						array [int] of var bool	: E,
						int				 		: start  );

% --------------------------------------------------------------------------

predicate dreachability(	int						: nvertices,
							array [int] of int		: sources,
							array [int] of int		: targets,
							array [int] of var bool	: V,
							array [int] of var bool	: E,
							int				 		: start );

% --------------------------------------------------------------------------

predicate parity_conditions(	array [int] of int		: owners,
								array [int] of int		: colors,
								array [int] of int		: sources,
								array [int] of int		: targets,
								array [int] of var bool	: V,
								array [int] of var bool	: E,
								int				 		: start,
	) = 
	let {	set of int : vertices	= 1..length(owners);
			set of int : edges		= 1..length(sources) 	} in 
	
	V[start] = true 
	
	/\	forall(v in vertices where owners[v]==even)(
			V[v] ->	exists(e in edges where sources[e]==v)(E[e])) 
	
	/\	forall(v in vertices where owners[v]==odd)(
			V[v] ->	forall(e in edges where sources[e]==v)(E[e]))
	
	/\	forall(w in vertices where w != start)(
			exists(e in edges where targets[e]==w)(E[e]) -> V[w]) 
	
	/\	nooddcycle(owners,colors,sources,targets,V,E,start)
	
	/\	dreachability(length(owners),sources,targets,V,E,start)
;

% --------------------------------------------------------------------------

predicate play(	set of int				: steps,
				array [int] of int		: sources,
				array [int] of int		: targets,
				array [int] of var bool	: E,
				int						: start,
				array[int] of var int 	: history_vertices,
				array[int] of var int 	: history_edges,
				array[int] of var int 	: cycle_vertices,
				array[int] of var int 	: cycle_edges,
				) =

	let { 
		array[steps] of var int : pathV,
		array[steps] of var int : pathE,
		var 0..length(steps) : token1,
		var 0..length(steps) : token2,

	} in

	pathV[1] = start 

	/\	forall(s in 2..token2)(
			exists(e in edges)(
				pathE[s-1] = e /\
				pathV[s] = targets[e] /\ 
				sources[e] = pathV[s-1] /\ E[e]
			)
		)
	
	/\ 	forall(s in token2+1..length(steps))(
			pathV[s] = 0 /\ pathE[s] = 0
		)
	
	/\	forall(i,j in 1..token2-1 where i>j)(pathV[i] != pathV[j])
	
	/\	exists(i in 1..token2-1)(
			(token1 = i) /\ (pathV[token1] = pathV[token2])
		)

	/\	forall(s in steps)(
			if s<token1 then
				history_vertices[s] = pathV[s] /\
				history_edges[s] = pathE[s]
			else
				history_vertices[s] = 0 /\
				history_edges[s] = 0
			endif
		)

	/\ 	forall(s in steps)(
			if s<token2-token1+1 then
				cycle_vertices[s] = pathV[s+token1-1] /\
				cycle_edges[s] = pathE[s+token1-1]
			else
				cycle_vertices[s] = 0 /\
				cycle_edges[s] = 0
			endif
		)
;

% --------------------------------------------------------------------------

function string : toStr(array [int] of var bool	: X) =
	let {
		array [int]	of int : Xa ::output_only ::output = 
			[x | x in 1..length(X)	where fix(X[x])];
	} in "\(Xa)";

% --------------------------------------------------------------------------

function string : toStr(array [int] of var int	: N) =
	"\([n | n in N where n>0])";