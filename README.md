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

## Executando

### Executando K-Means Sequencial

```
./sequencial <arq_centroides> <arq_pontos>
```

### Executando K-Means Paralelo

```
./paralelo <arq_centroides> <arq_pontos> <num_threads>
```
