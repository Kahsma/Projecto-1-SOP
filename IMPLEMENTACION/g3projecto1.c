// Luisa Parra, Fabio Buitrago 6 Camilo
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct BMP
{
    char bm[2];                 //(2 Bytes) BM (Tipo de archivo)
    int tamano;                 //(4 Bytes) Tamaño del archivo en bytes
    int reservado;              //(4 Bytes) Reservado
    int offset;                 //(4 Bytes) offset, distancia en bytes entre la img y los píxeles
    int tamanoMetadatos;        //(4 Bytes) Tamaño de Metadatos (tamaño de esta estructura = 40)
    int alto;                   //(4 Bytes) Ancho (numero de píxeles horizontales)
    int ancho;                  //(4 Bytes) Alto (numero de pixeles verticales)
    short int numeroPlanos;     //(2 Bytes) Numero de planos de color
    short int profundidadColor; //(2 Bytes) Profundidad de color (debe ser 24 para nuestro caso)
    int tipoCompresion;         //(4 Bytes) Tipo de compresión (Vale 0, ya que el bmp es descomprimido)
    int tamanoEstructura;       //(4 Bytes) Tamaño de la estructura Imagen (Paleta)
    int pxmh;                   //(4 Bytes) Píxeles por metro horizontal
    int pxmv;                   //(4 Bytes) Píxeles por metro vertical
    int coloresUsados;          //(4 Bytes) Cantidad de colores usados
    int coloresImportantes;     //(4 Bytes) Cantidad de colores importantes
    unsigned char ***pixel;     // Puntero a una tabla dinamica de caracteres de 3 dimensiones para almacenar los pixeles
} BMP;

//*****************************************************************
// DECLARACIÓN DE FUNCIONES
//*****************************************************************
void abrir_imagen(BMP *imagen, char ruta[]);    // Función para abrir la imagen BMP
void crear_imagen(BMP *imagen, char ruta[]);    // Función para crear una imagen BMP
void convertir_imagen(BMP *imagen, int nhilos); // 2 sera el numero de hilos

int main(int argc, char *argv[])
{
    BMP img;
    if (argc != 9)
    {
        printf("Los argumentos del progrma son los siguientes: ./imgconc –i [Imagen de entrada] –t [Imagen de salida] –o [Opción (1,2 o 3)] –h [Numero de hilos]");
        exit(1);
    }

    if (argv[1] != "-i")
    {
        printf("El argumento para la imagen de entrada es '-i' ");
        exit(1);
    }

    if (argv[3] != "-t")
    {
        printf("El argumento para la imagen de salida es '-t' ");
        exit(1);
    }
    if (argv[5] != "-o")
    {
        printf("El argumento para la opción de filtro es '-o' ");
        exit(1);
    }
    if (argv[7] != "-h")
    {
        printf("El argumento para el numero de hilos es '-h' ");
        exit(1);
    }

    char *nomImagenEntrada = argv[2];
    char *nomImagenSalida = argv[4];
    int fliltro = argv[6];
    int nHilos = argv[8];
}

void abrir_imagen(BMP *imagen, char *ruta)
{
    FILE *archivo; // Puntero FILE para el archivo de imágen a abrir
    int i, j, k;
    unsigned char P[3];

    // Abrir el archivo de imágen
    archivo = fopen(ruta, "rb+");
    if (!archivo)
    {
        // Si la imágen no se encuentra en la ruta dada
        perror("La imágen no se encontro: ");
        exit(1);
    }

    // Leer la cabecera de la imagen y almacenarla en la estructura a la que apunta imagen
    fseek(archivo, 0, SEEK_SET);
    fread(&imagen->bm, sizeof(char), 2, archivo);
    fread(&imagen->tamano, sizeof(int), 1, archivo);
    fread(&imagen->reservado, sizeof(int), 1, archivo);
    fread(&imagen->offset, sizeof(int), 1, archivo);
    fread(&imagen->tamanoMetadatos, sizeof(int), 1, archivo);
    fread(&imagen->alto, sizeof(int), 1, archivo);
    fread(&imagen->ancho, sizeof(int), 1, archivo);
    fread(&imagen->numeroPlanos, sizeof(short int), 1, archivo);
    fread(&imagen->profundidadColor, sizeof(short int), 1, archivo);
    fread(&imagen->tipoCompresion, sizeof(int), 1, archivo);
    fread(&imagen->tamanoEstructura, sizeof(int), 1, archivo);
    fread(&imagen->pxmh, sizeof(int), 1, archivo);
    fread(&imagen->pxmv, sizeof(int), 1, archivo);
    fread(&imagen->coloresUsados, sizeof(int), 1, archivo);
    fread(&imagen->coloresImportantes, sizeof(int), 1, archivo);

    // Validar ciertos datos de la cabecera de la imágen
    if (imagen->bm[0] != 'B' || imagen->bm[1] != 'M')
    {
        printf("La imagen debe ser un bitmap.\n");
        exit(1);
    }
    if (imagen->profundidadColor != 24)
    {
        printf("La imagen debe ser de 24 bits.\n");
        exit(1);
    }

    // Reservar memoria para la matriz de pixels

    imagen->pixel = malloc(imagen->alto * sizeof(char *));
    for (i = 0; i < imagen->alto; i++)
    {
        imagen->pixel[i] = malloc(imagen->ancho * sizeof(char *));
    }

    for (i = 0; i < imagen->alto; i++)
    {
        for (j = 0; j < imagen->ancho; j++)
            imagen->pixel[i][j] = malloc(3 * sizeof(char));
    }

    // Pasar la imágen a el arreglo reservado en escala de grises
    // unsigned char R,B,G;

    for (i = 0; i < imagen->alto; i++)
    {
        for (j = 0; j < imagen->ancho; j++)
        {
            for (k = 0; k < 3; k++)
            {
                fread(&P[k], sizeof(char), 1, archivo);       // Byte Blue del pixel
                imagen->pixel[i][j][k] = (unsigned char)P[k]; // Formula correcta
            }
        }
    }

    // Cerrrar el archivo
    fclose(archivo);
}