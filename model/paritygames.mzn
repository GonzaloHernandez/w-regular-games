
include "cyclefilter.mzn";

int : even  = 0;
int : odd   = 1;

% --------------------------------------------------------------------------

predicate nooddcycle(	array [int] of int		: owners,
						array [int] of int		: colors,
						array [int] of int		: sources,
						array [int] of int		: targets,
						array [int] of var bool	: V,
						array [int] of var bool	: E,
						array [int] of int 		: start  );

% --------------------------------------------------------------------------

predicate dreachability(	array [int] of int		: nvertices,
							array [int] of int		: nedges,
							array [int] of int		: sources,
							array [int] of int		: targets,
							array [int] of var bool	: V,
							array [int] of var bool	: E,
							array [int] of int 		: start );

% --------------------------------------------------------------------------

predicate parity_conditions(	array [int] of int		: owners,
								array [int] of int		: colors,
								array [int] of int		: sources,
								array [int] of int		: targets,
								array [int] of var bool	: V,
								array [int] of var bool	: E,
								int				 		: start  ) = 
	let {	set of int : vertices	= 1..length(owners);
			set of int : edges		= 1..length(sources) 	} in 
	
	V[start] = true /\
	
	forall(v in vertices where owners[v]==even)(
		V[v] ->	exists(e in edges where sources[e]==v)(E[e])
	) /\
	
	forall(v in vertices where owners[v]==odd)(
		V[v] ->	forall(e in edges where sources[e]==v)(E[e])
	) /\
	
	forall(w in vertices where w != start)(
		exists(e in edges where targets[e]==w)(E[e]) -> V[w]
	) /\
	
	nooddcycle(owners,colors,sources,targets,V,E,[start]) /\
	
	dreachability([length(owners)],[length(sources)],sources,targets,V,E,[start]);


% --------------------------------------------------------------------------

predicate fixVertices(array [int] of int : vs, array [int] of int : nvs) =
	forall(v in vs)(V[v]) /\ forall(v in nvs)(not V[v]);

% --------------------------------------------------------------------------

predicate fixEdges(array [int] of int : es, array [int] of int: nes) =
	forall(e in es)(E[e]) /\ forall(e in nes)(not E[e]);
