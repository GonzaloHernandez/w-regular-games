% Parity type
int: EVEN   = 0;    
int: ODD    = 1;
% Reward type
int: MIN    = 0;    
int: MAX    = 1;

% --------------------------------------------------------------------------

function int: opponent(int:player) =
    if player=EVEN then ODD else EVEN endif
;

% --------------------------------------------------------------------------

predicate noc(	array [int] of int		: owners,
				array [int] of int		: priors,
				array [int] of int		: sources,
				array [int] of int		: targets,
				int				 		: init,
				int				 		: player,
				int				 		: reward,
				array [int] of var bool	: V,
				array [int] of var bool	: E)
;