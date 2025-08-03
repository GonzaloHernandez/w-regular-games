1. Clone chuffed solver 
	git clone --branch 0.12.1 https://github.com/chuffed/chuffed.git

2. Copy from chuffed-patch files into chuffed/support folder
	game.h and prop_noc.h

3. Patch registry.cpp into chuffed folder (not chuffed/chuffed)
	patch -p1 < flatzinc-NOC.patch

4. Compile chuffed with new features
	cmake -B build -S . -DCMAKE_INSTALL_PREFIX=/home/chalo/.local
	cmake --build build --target install

5. (Optional) Set new solver configuration into Minizinc
	sonar.msc into ~/.minizinc/solvers
	Note: Update sonar.msc file with the same PREFIX used in step 4

