% --- Domain Definitions ---
int : nvertices;
int : nedges;
set of int : vertices	= 1..nvertices;
set of int : edges = 1..nedges;

% --- Graph Data (Adjacency & Properties) ---
array [vertices] of int : owners;
array [vertices] of int : priors;
array [edges] of int : sources;
array [edges] of int : targets;

% --- Game Configuration ---
vertices : init = 1;
int : player = EVEN;
int : reward = MAX;

% --- Decision variables ---
array [vertices] of var bool : V;
array [edges] of var bool : E;

% --- Constraints ---

include "paritygames_chuffed.mzn";

constraint	
    V[init] = true;
	
constraint 
    forall(v in vertices where owners[v]==player)(
        V[v] ->	sum(e in edges where sources[e]==v)(E[e])=1) ;
	
constraint 
    forall(v in vertices where owners[v]==opponent(player))(
		V[v] ->	forall(e in edges where sources[e]==v)(E[e]));

constraint
    forall(e in edges)(
        E[e] -> V[targets[e]]);

constraint
    noc(owners,priors,sources,targets,init,player,reward,V,E);

solve satisfy;

% --- SATISFIABLE if the <player> of interest wins; 
% --- UNSATISFIABLE otherwise

output ["=====SATISFIABLE===== \n"]; 

