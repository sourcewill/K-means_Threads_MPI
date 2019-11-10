#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <time.h>


// STRUCTS
typedef struct{
    int * coordenadas;
    int id;
    int * soma_pontos_associados;
    int num_associados;
}CENTROIDE;

typedef struct{
    int * coordenadas;
    int id_centroide;
}PONTO;


// GLOBAL
int geraId = 0, BASE, NUM_CENTROIDES = 0, NUM_PONTOS = 0, FLAG_ATUALIZOU = 1, NTH;
pthread_mutex_t MUT = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t BARREIRA;
CENTROIDE* CENTROIDES;
PONTO* PONTOS;


// Recebe as coordenadas de um Centroide e inicializa o mesmo
CENTROIDE inicializaCentroide(int* coord, int* soma_pontos){
    CENTROIDE centroide;
    centroide.coordenadas = coord;
    centroide.id = geraId++;
    centroide.soma_pontos_associados = soma_pontos;
    centroide.num_associados = 0;
    NUM_CENTROIDES++;
    return centroide;
}

// Recebe as coordenadas de um Ponto e inicializa o mesmo
PONTO inicializaPonto(int* coord){
    PONTO ponto;
    ponto.coordenadas = coord;
    ponto.id_centroide = -1;
    NUM_PONTOS++;
    return ponto;
}


// Retorna o numero de linhas um arquivo
int num_linhas(FILE* arq){
    char c;
    int num=0;
        while(fread (&c, sizeof(char), 1, arq)){
            if(c == '\n') {
                num++;
            }
        }
    return num;
}


// Importa CENTROIDES com base em um arquivo de texto
// Cada linha do arquivo representa as coordenadas de um Centroide
void importa_centroides(FILE* entrada){

    printf("\nImportando Centroides...\n");

    int *coordenadas, *soma_pontos, i, j, num, linhas;
    CENTROIDE centroide;

    linhas = num_linhas(entrada);
    CENTROIDES = (CENTROIDE*)malloc(linhas * sizeof(CENTROIDE));

    fseek(entrada,0,SEEK_SET);
    for(i=0; i<linhas; i++){
        coordenadas = (int*) malloc(BASE * sizeof(int));
        soma_pontos = (int*) malloc(BASE * sizeof(int));
        for(j=0; j<BASE; j++){
            fscanf(entrada, "%d", &num);
            coordenadas[j] = num;
            fseek(entrada,1,SEEK_CUR);
            soma_pontos[j] = 0;
        }
        centroide = inicializaCentroide(coordenadas, soma_pontos);
        CENTROIDES[i] = centroide;

    }

    printf("%d Centroides importados com sucesso!\n", NUM_CENTROIDES);
}


// Importa PONTOS com base em um arquivo de texto
// Cada linha do arquivo representa as coordenadas de um Ponto separadas por uma virgula
// A ultima linha do arquivo deve estar em branco
void importa_pontos(FILE* entrada){

    printf("\nImportando Pontos...\n");

    int *coordenadas, i, j, num, linhas;
    PONTO ponto;

    linhas = num_linhas(entrada);
    PONTOS = (PONTO*)malloc(linhas * sizeof(PONTO));

    fseek(entrada,0,SEEK_SET);
    for(i=0; i<linhas; i++){
        coordenadas = (int*) malloc(BASE * sizeof(int));
        for(j=0; j<BASE; j++){
            fscanf(entrada, "%d", &num);
            coordenadas[j] = num;
            fseek(entrada,1,SEEK_CUR); // Pula a virgula
        }
        ponto = inicializaPonto(coordenadas);
        PONTOS[i] = ponto;

    }

    printf("%d Pontos importados com sucesso!\n", NUM_PONTOS);
}


// Escreve no arquivo de saida todas as coordenadas atuais dos centroides
void escreve_arq_saida(FILE * arq_saida){

    int i, j;
    for(i=0; i<NUM_CENTROIDES; i++){
        for(j=0; j<BASE; j++){
            fprintf(arq_saida, "%d", CENTROIDES[i].coordenadas[j]);
            if(j<BASE-1){
                fprintf(arq_saida, ",");
            }
        }
        fprintf(arq_saida, "\n");
    }
}


// Retorna a distancia entre um centroide e um ponto
int distacia_centroide_ponto(CENTROIDE centroide, PONTO ponto){

    int i, diferenca, somatorio = 0, raiz;
    for(i=0; i<BASE; i++){
        diferenca = centroide.coordenadas[i] - ponto.coordenadas[i];
        somatorio += (diferenca * diferenca);
    }
    raiz = floor(sqrt(somatorio)); // floor arredonda para baixo
    return raiz;
}


// Busca o centroide mais proximo do ponto atual
// Atualiza o campo id_centroide do ponto com id do centroide mais proximo
// Incrementa o campo num_associados do centroide
// Incrementa todo vetor soma_pontos_associados do centroide com as coordenadas do ponto atual
void atualiza_centroide_mais_proximo(PONTO *ponto){

    CENTROIDE* centroide;
    int i, distancia_atual, menor_distancia = 999999999, indice;

    for(i=0; i<NUM_CENTROIDES; i++){
        distancia_atual = distacia_centroide_ponto(CENTROIDES[i], *ponto);
        if(distancia_atual < menor_distancia){
            menor_distancia = distancia_atual;
            indice = i;
        }
    }
    // Atualiza ponto e centroide
    ponto->id_centroide = indice;
    centroide = &CENTROIDES[indice];

    pthread_mutex_lock(&MUT); // LOCK
    centroide->num_associados++;
    for(i=0; i<BASE; i++){
        centroide->soma_pontos_associados[i] += ponto->coordenadas[i];
    }
    pthread_mutex_unlock(&MUT); // UNLOCK
}


// Retorna a flag informando se houve atualizacao nas coordenadas do centroide
int recalcula_coordenadas_centroide(CENTROIDE* centroide){

    int atualizou = 0, num_associados, i, media;

    num_associados = centroide->num_associados;
    for(i=0; i<BASE; i++){
        if(num_associados > 0){
            media = floor(centroide->soma_pontos_associados[i] / num_associados);
            if(media != centroide->coordenadas[i]){
                atualizou = 1;
                centroide->coordenadas[i] = media;
            }
        }
    }
    return atualizou;
}


void reinicia_vars_centroide(CENTROIDE* centroide){
    int i;
    centroide->num_associados = 0;
    for(i=0; i<BASE; i++){
        centroide->soma_pontos_associados[i] = 0;
    }
}


void* K_means(void* arg){

    int i;
    CENTROIDE* centroide;
    long int id_thread;
    id_thread = (long int) arg;

    printf("\nThread(%ld) processando algoritmo K-means...", id_thread);

    while(FLAG_ATUALIZOU){

        // BARREIRA
        pthread_barrier_wait(&BARREIRA);

        FLAG_ATUALIZOU = 0;

        for(i = id_thread; i<NUM_PONTOS; i += NTH){
            atualiza_centroide_mais_proximo(&PONTOS[i]);
        }

        // BARREIRA
        pthread_barrier_wait(&BARREIRA);

        // Recalcular coordenadas dos centroides
        for(i = id_thread; i<NUM_CENTROIDES; i += NTH){
            centroide = &CENTROIDES[i];
            FLAG_ATUALIZOU = recalcula_coordenadas_centroide(centroide);
            reinicia_vars_centroide(centroide);
        }

        // BARREIRA
        pthread_barrier_wait(&BARREIRA);
    }
    pthread_exit(0);
}


int main(int argc, char* argv[]){

    if(argc < 2){
        printf("Sao necessarios 4 prametros:\nbase | arq_centroides | arq_pontos | num_threads\n");
        exit(1);
    }

    NTH = atoi(argv[4]);
    BASE = atoi(argv[1]);
    FILE *arq_centroides, *arq_pontos, *arq_saida;
    char nome_arq_saida[50]="out_parallel_centroid_base_";

    arq_centroides = fopen(argv[2], "rb");
    arq_pontos = fopen(argv[3], "rb");

    importa_centroides(arq_centroides);
    importa_pontos(arq_pontos);

    fclose(arq_centroides);
    fclose(arq_pontos);

    pthread_barrier_init(&BARREIRA,NULL,NTH);
    pthread_t* threads = (pthread_t *) malloc(NTH * sizeof(pthread_t));

    long int i;
    for(i=0; i<NTH; i++){
        pthread_create(&threads[i],NULL,K_means,(void*)i);
    }

    for(i=0;i<NTH;i++){
        pthread_join(threads[i],NULL);
    }

    strcat(nome_arq_saida, argv[1]);
    arq_saida = fopen(nome_arq_saida, "w");
    escreve_arq_saida(arq_saida);
    printf("\n\nArquivo '%s' criado no atual diretorio.\n\n", nome_arq_saida);
    fclose(arq_saida);

    return 0;
}
