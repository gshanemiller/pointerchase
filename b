set -x
g++ -I. -O3 -g -c -DNDEBUG main.cpp -o main.o
g++ -I. -O3 -g -c -DNDEBUG intel_pmu_stats.cpp -o intel_pmu_stats.o
g++ -I. -O3 -g -c -DNDEBUG intel_skylake_pmu.cpp -o intel_skylake_pmu.o
g++ *.o -o pc.tsk -lpthread
