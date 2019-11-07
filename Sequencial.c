#include <stdio.h>
#include <stdlib.h>


// STRUCTS
typedef struct{
    int * coordenadas;
    int id;
}CENTROIDE;

typedef struct{
    int * coordenadas;
    int id_centroide;
}PONTO;


// GLOBAL
int geraId = 0;
int BASE;
int NUM_CENTROIDES = 0;
int NUM_PONTOS = 0;
CENTROIDE* CENTROIDES;
PONTO* PONTOS;


// Recebe as coordenadas de um Centroide e inicializa o mesmo
CENTROIDE inicializaCentroide(int* coord){
    CENTROIDE centroide;
    centroide.coordenadas = coord;
    centroide.id = geraId++;
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

    int *coordenadas, i, j, num, linhas;
    CENTROIDE centroide;

    linhas = num_linhas(entrada);
    CENTROIDES = (CENTROIDE*)malloc(linhas * sizeof(CENTROIDE));

    fseek(entrada,0,SEEK_SET);
    for(i=0; i<linhas; i++){
        coordenadas = (int*) malloc(BASE * sizeof(int));
        for(j=0; j<BASE; j++){
            fscanf(entrada, "%d", &num);
            coordenadas[j] = num;
            fseek(entrada,1,SEEK_CUR);
        }
        centroide = inicializaCentroide(coordenadas);
        CENTROIDES[i] = centroide;

    }

    printf("%d Centroides importados com sucesso!\n", NUM_CENTROIDES);
}


// Importa PONTOS com base em um arquivo de texto
// Cada linha do arquivo representa as coordenadas de um Ponto
void importa_pontos(FILE* entrada){

    printf("\nImportando Pontos...\n");

    int *coordenadas, i, j, num, linhas;
    PONTO ponto;

    linhas = num_linhas(entrada);
    PONTOS = (CENTROIDE*)malloc(linhas * sizeof(CENTROIDE));

    fseek(entrada,0,SEEK_SET);
    for(i=0; i<linhas; i++){
        coordenadas = (int*) malloc(BASE * sizeof(int));
        for(j=0; j<BASE; j++){
            fscanf(entrada, "%d", &num);
            coordenadas[j] = num;
            fseek(entrada,1,SEEK_CUR);
        }
        ponto = inicializaPonto(coordenadas);
        PONTOS[i] = ponto;

    }

    printf("%d Pontos importados com sucesso!\n", NUM_PONTOS);
}


int main(){

    BASE = 59;
    FILE *arq_centroides, *arq_pontos;

    arq_centroides = fopen("int_bases/int_centroid_59_20.data", "rb");
    arq_pontos = fopen("int_bases/int_base_59.data", "rb");

    importa_centroides(arq_centroides);
    importa_pontos(arq_pontos);

    fclose(arq_centroides);
    fclose(arq_pontos);
    return 0;
}
