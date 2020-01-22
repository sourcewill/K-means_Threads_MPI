#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include <stddef.h>

#define TAG 1
#define PROCESSO_CENTRAL 0


// STRUCTS
typedef struct{
    int * coordenadas;
    int id;
    int * soma_pontos_associados;
    int num_associados;
}CENTROIDE;

typedef struct{
    int * coordenadas;
}PONTO;


// GLOBAL
int geraId = 0, BASE, NUM_CENTROIDES = 0, NUM_PONTOS = 0, FLAG_ATUALIZOU = 1, SIZE, RANK;
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

// Processos responsaveis: Servos
// Busca o centroide mais proximo do ponto atual
void encontra_centroide_mais_proximo(int id_ponto, PONTO *ponto){

    int i, distancia_atual, menor_distancia = INT_MAX, id_centroide;

    for(i=0; i<NUM_CENTROIDES; i++){
        distancia_atual = distacia_centroide_ponto(CENTROIDES[i], *ponto);
        if(distancia_atual < menor_distancia){
            menor_distancia = distancia_atual;
            id_centroide = i;
        }
    }

    MPI_Send(&id_ponto, 1, MPI_INT, PROCESSO_CENTRAL, TAG, MPI_COMM_WORLD);
    MPI_Send(&id_centroide, 1, MPI_INT, PROCESSO_CENTRAL, TAG, MPI_COMM_WORLD);

}

// Processos responsavel: PROCESSO_CENTRAL
// Incrementa o campo num_associados do centroide
// Incrementa todo vetor soma_pontos_associados do centroide com as coordenadas do ponto atual
void atualiza_centroide_mais_proximo(int processo){

    MPI_Status st;
    int id_ponto, id_centroide, i;
    PONTO *ponto;
    CENTROIDE* centroide;

    MPI_Recv(&id_ponto, 1, MPI_INT, processo, TAG, MPI_COMM_WORLD, &st);
    MPI_Recv(&id_centroide, 1, MPI_INT, processo, TAG, MPI_COMM_WORLD, &st);

    ponto = &PONTOS[id_ponto];
    centroide = &CENTROIDES[id_centroide];

    centroide->num_associados++;
    for(i=0; i<BASE; i++){
        centroide->soma_pontos_associados[i] += ponto->coordenadas[i];
    }

}


void reinicia_vars_centroide(CENTROIDE* centroide){
    int i;
    centroide->num_associados = 0;
    for(i=0; i<BASE; i++){
        centroide->soma_pontos_associados[i] = 0;
    }
}

// Processos responsaveis: Servos
void recalcula_coordenadas_centroide(int id_centroide, CENTROIDE* centroide){

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

    reinicia_vars_centroide(centroide);

    MPI_Send(&id_centroide, 1, MPI_INT, PROCESSO_CENTRAL, TAG, MPI_COMM_WORLD);
    MPI_Send(&centroide->num_associados, 1, MPI_INT, PROCESSO_CENTRAL, TAG, MPI_COMM_WORLD);
    for(i=0; i<BASE; i++){
        MPI_Send(&centroide->soma_pontos_associados[i], 1, MPI_INT, PROCESSO_CENTRAL, TAG, MPI_COMM_WORLD);
    }
    for(i=0; i<BASE; i++){
        MPI_Send(&centroide->coordenadas[i], 1, MPI_INT, PROCESSO_CENTRAL, TAG, MPI_COMM_WORLD);
    }

    MPI_Send(&atualizou, 1, MPI_INT, PROCESSO_CENTRAL, TAG, MPI_COMM_WORLD);
}

void atualiza_coordenadas_centroide(int processo){

    MPI_Status st;
    int id_centroide, i, atualizou;
    CENTROIDE* centroide;

    MPI_Recv(&id_centroide, 1, MPI_INT, processo, TAG, MPI_COMM_WORLD, &st);

    centroide = &CENTROIDES[id_centroide];

    MPI_Recv(&centroide->num_associados, 1, MPI_INT, processo, TAG, MPI_COMM_WORLD, &st);

    for(i=0; i<BASE; i++){
        MPI_Recv(&centroide->soma_pontos_associados[i], 1, MPI_INT, processo, TAG, MPI_COMM_WORLD, &st);
    }
    for(i=0; i<BASE; i++){
        MPI_Recv(&centroide->coordenadas[i], 1, MPI_INT, processo, TAG, MPI_COMM_WORLD, &st);
    }

    MPI_Recv(&atualizou, 1, MPI_INT, processo, TAG, MPI_COMM_WORLD, &st);
    if(atualizou){
        FLAG_ATUALIZOU = 1;
    }

}


void broadcast_centroides(){
    int i, j;

    for(i=0; i<NUM_CENTROIDES; i++){
        MPI_Bcast(&CENTROIDES[i].num_associados, 1, MPI_INT, PROCESSO_CENTRAL, MPI_COMM_WORLD);
        for(j=0; j<BASE; j++){
            MPI_Bcast(&CENTROIDES[i].soma_pontos_associados[j], 1, MPI_INT, PROCESSO_CENTRAL, MPI_COMM_WORLD);
            MPI_Bcast(&CENTROIDES[i].coordenadas[j], 1, MPI_INT, PROCESSO_CENTRAL, MPI_COMM_WORLD);
        }
    }
}


void K_means(){

    CENTROIDE* centroide;
    MPI_Status st;

    while(FLAG_ATUALIZOU){

        FLAG_ATUALIZOU = 0;

        if(RANK != PROCESSO_CENTRAL){
            for(int i = RANK-1; i<NUM_PONTOS; i += (SIZE-1)){
                encontra_centroide_mais_proximo(i, &PONTOS[i]);
            }
        }

        if (RANK == PROCESSO_CENTRAL){
            for(int processo = 1; processo<SIZE; processo++){
                for(int i = processo-1; i<NUM_PONTOS; i += (SIZE-1)){
                    atualiza_centroide_mais_proximo(processo);
                }
            }
        }

        broadcast_centroides();
        
        MPI_Barrier(MPI_COMM_WORLD);

        // Recalcular coordenadas dos centroides
        if(RANK != PROCESSO_CENTRAL){
            for(int i = RANK-1; i<NUM_CENTROIDES; i += (SIZE-1)){
                centroide = &CENTROIDES[i];
                recalcula_coordenadas_centroide(i, centroide);
            }
        }

        if (RANK == PROCESSO_CENTRAL){
            for(int processo = 1; processo<SIZE; processo++){
                for(int i = processo-1; i<NUM_CENTROIDES; i += (SIZE-1)){
                    atualiza_coordenadas_centroide(processo);
                }
            }
        }

        // Broadcast para atualizar a flag nos processos servos
        MPI_Bcast(&FLAG_ATUALIZOU, 1, MPI_INT, PROCESSO_CENTRAL, MPI_COMM_WORLD);
        
        broadcast_centroides();

        MPI_Barrier(MPI_COMM_WORLD);

    }

}


int main(int argc, char* argv[]){

    MPI_Init(&argc,&argv);
    
    MPI_Comm_rank(MPI_COMM_WORLD,&RANK);
    MPI_Comm_size(MPI_COMM_WORLD,&SIZE);

    if(RANK == PROCESSO_CENTRAL){
        if(SIZE < 2){
            printf("Sao necessarios pelo menos 2 processos para execucao.\n");
            MPI_Finalize();
            exit(1);
        }

        if(argc < 2){
            printf("Sao necessarios 3 prametros:\nbase | arq_centroides | arq_pontos\n");
            MPI_Finalize();
            exit(1);
        }
    }

    BASE = atoi(argv[1]);

    FILE *arq_centroides, *arq_pontos, *arq_saida;
    char nome_arq_saida[50]="out_MPI_centroid_base_";

    arq_centroides = fopen(argv[2], "rb");
    arq_pontos = fopen(argv[3], "rb");

    importa_centroides(arq_centroides);
    importa_pontos(arq_pontos);

    fclose(arq_centroides);
    fclose(arq_pontos);


    MPI_Barrier(MPI_COMM_WORLD);
    
    K_means();

    MPI_Barrier(MPI_COMM_WORLD);

    if(RANK == PROCESSO_CENTRAL){
        strcat(nome_arq_saida, argv[1]);
        arq_saida = fopen(nome_arq_saida, "w");
        escreve_arq_saida(arq_saida);
        printf("\n\nArquivo '%s' criado no atual diretorio.\n\n", nome_arq_saida);
        fclose(arq_saida);
    }

    MPI_Finalize();

    return 0;
}

