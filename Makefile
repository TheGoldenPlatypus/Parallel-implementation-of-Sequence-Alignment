build:
	mpicc -fopenmp -o final_proj textFile.c finalProj.c
clean:
	rm -f *.o ./final_proj

run:
	mpiexec -np 5 ./final_proj input.txt output.txt

runOn2:
	mpiexec -np 5 -machinefile mf -map-by node ./final_proj input.txt output.txt