% ------------------------------------------------------------------------------------------

function array[int] of int : getoddcolors(array [$$V] of int : colors) =
	let {
		array [int] of int	: original_oddcolors	= [ c | c in colors where c mod 2 == 1 ],
		set of int			: set_oddcolors			= { c | c in original_oddcolors }
	} in
	
	[ c | c in set_oddcolors ];

% ------------------------------------------------------------------------------------------

function $$E: indexof( array [$$E] of $$V: x, $$V: v ) =
	min(i in index_set(x) where x[i] = v)(i);
	
% ------------------------------------------------------------------------------------------

predicate measureprogress(	array [$$V] of int			: colors,
							array [$$V,$$C] of var $$V	: pmeasures,
							$$E 						: e,
							$$V 						: v,
							$$V 						: w ) =

	let {	array [int] of int			: oddcolors		= getoddcolors(colors),
			int 						: mop			= max(oddcolors),
			array [1..mop] of 0..mop 	: oddcolorspos	= [		
				if i in oddcolors then indexof(oddcolors,i) else 0 endif | i in 1..mop],
	} in
	
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

% ------------------------------------------------------------------------------------------

predicate globalmeasureprogress(	array [$$V] of int		: owners,
									array [$$V] of int		: colors,
									array [$$E] of int		: sources,
									array [$$E] of int		: targets,
									array [$$V]	of var bool	: V,
									array [$$E]	of var bool	: E ) =
	
	let {	set of int 			: vertices	= index_set(owners),
			set of int 			: edges		= index_set(sources),
			array [int] of int	: oddcolors	= getoddcolors(colors),
			array [vertices,1..length(oddcolors)] of var vertices : pmeasures
	} in
	
	forall(e in edges)(
		E[e] -> measureprogress(colors,pmeasures,e,sources[e],targets[e])
	);
