1. Clone chuffed solver 
	git clone --branch 0.12.1 <git-url>

2. Copy files into chuffed/support folder
	gnome.cpp and nooddcyclefilter.cpp

3. Patch registry.cpp into chuffed folder
	patch -p1 < nooddcyclefilter.patch

4. Compile chuffed with new factions
	cmake -B build -S . -DCMAKE_INSTALL_PREFIX=/home/chalo/.local
	cmake --build build --target install

4. Copy Minizinc solver configuration
	sonar.msc into ~/.minizinc/solvers

