# K-Means
Implementação do algoritmo K-means sequencial e concorrente com aplicação de Threads. 

## Montando

### Montando K-Means Sequencial

```
gcc Sequencial.c -o sequencial -lm
```

### Montando K-Means Paralelo

```
gcc Paralelo.c -o paralelo -lm -pthread
```

### Montando K-Means Paralelo MPI

```
mpicc MPI_Kmeans.c -o mpikmeans -lm
```

## Executando

### Executando K-Means Sequencial

```
./sequencial <base> <arq_centroides> <arq_pontos>
```

### Executando K-Means Paralelo

```
./paralelo <base> <arq_centroides> <arq_pontos> <num_threads>
```

### Executando K-Means Paralelo MPI

```
mpirun -np <num_processos> ./mpikmeans <base> <arq_centroides> <arq_pontos>
```

