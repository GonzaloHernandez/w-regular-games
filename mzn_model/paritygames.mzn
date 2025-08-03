int : even  = 0;
int : odd   = 1;

% --------------------------------------------------------------------------

predicate noc(	array [int] of int		: owners,
				array [int] of int		: colors,
				array [int] of int		: sources,
				array [int] of int		: targets,
				array [int] of var bool	: E,
				int				 		: start,
				int				 		: reward,
				int				 		: player );

% --------------------------------------------------------------------------

predicate parity_conditions(	array [int] of int		: owners,
								array [int] of int		: colors,
								array [int] of int		: sources,
								array [int] of int		: targets,
								array [int] of var bool	: V,
								array [int] of var bool	: E,
								int				 		: start,
								int				 		: reward,
								int				 		: player,
	) = 
	let {	set of int : vertices	= 1..length(owners);
			set of int : edges		= 1..length(sources);
			int : opponent = 1-player; 	} in 
	
	V[start] = true 
	
	/\	forall(v in vertices where owners[v]==player)(
			V[v] ->	sum(e in edges where sources[e]==v)(E[e])=1) 
	
	/\	forall(v in vertices where owners[v]==opponent)(
			V[v] ->	forall(e in edges where sources[e]==v)(E[e]))
	
	/\	forall(w in vertices where w != start)(
			exists(e in edges where targets[e]==w)(E[e]) -> V[w]) 
	
	/\	noc(owners,colors,sources,targets,E,start,reward,player)
;

% --------------------------------------------------------------------------

predicate even_play(set of int				: steps,
					array [int] of int		: sources,
					array [int] of int		: targets,
					array [int] of var bool	: E,
					int						: start,
					array[int] of var int 	: historyV,
					array[int] of var int 	: historyE,
					array[int] of var int 	: infinityV,
					array[int] of var int 	: infinityE,
					array[int] of var int 	: pathV,
					array[int] of var int 	: pathE,
					) =

	let { 
		var 0..length(steps) : token1,
		var 0..length(steps) : token2,

	} in

	pathV[1] = start 

		% Constructing the path 
	/\	forall(s in 2..token2)(
			exists(e in edges)(
				pathE[s-1] = e /\
				pathV[s] = targets[e] /\ 
				sources[e] = pathV[s-1] /\ E[e]
			)
		)

		% Last edge equal to the first edge in the cycle 
	/\	pathE[token2] = pathE[token1]

		% Filling with Zeros in left remaining cells 
	/\ 	forall(s in token2+1..length(steps))(
			pathV[s] = 0 /\ pathE[s] = 0
		)
	
		% Every vertex only ones occurrence except last one
	/\	forall(i,j in 1..token2-1 where i>j)(
			pathV[i] != pathV[j]
		)

		% Finding the cycle	
	/\	exists(i in 1..token2-1)(
			(token1 = i) /\ (pathV[token1] = pathV[token2])
		)

		% Dumping path into history
	/\	forall(s in steps)(
			if s<token1 then
				historyV[s] = pathV[s] /\
				historyE[s] = pathE[s]
			else
				historyV[s] = 0 /\
				historyE[s] = 0
			endif
		)

		% Dumping path into infinity
	/\ 	forall(s in steps)(
			if s<token2-token1+1 then
				infinityV[s] = pathV[s+token1-1] /\
				infinityE[s] = pathE[s+token1-1]
			else
				infinityV[s] = 0 /\
				infinityE[s] = 0
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

% --------------------------------------------------------------------------

function var int : getPos(	
							array[int] of var int: historyV,
							array[int] of var int: infinityV,
							int: n, 
							) =
	let {
		var int: his = count(v in historyV)(v!=0);
		var int: inf = count(v in infinityV)(v!=0);
	} in
	if n <= (his+inf) then
		n
	else
		if ((n-his) mod inf) == 0 then
			his + inf
		else
			his + ((n-his) mod inf)
		endif
	endif
;