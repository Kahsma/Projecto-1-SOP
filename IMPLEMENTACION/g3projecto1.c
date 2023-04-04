// Luisa Parra, Fabio Buitrago 6 Camilo
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>


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

typedef struct datos_hilo_t {
    BMP *imagen;
    int filtro;
    int inicio;
    int fin;
    int opcion;
} datos_hilo_t;

BMP img;

//*****************************************************************
// DECLARACIÓN DE FUNCIONES
//*****************************************************************
void abrir_imagen(BMP *imagen, char ruta[]);    // Función para abrir la imagen BMP
void crear_imagen(BMP *imagen, char ruta[]);    // Función para crear una imagen BMP
void convertir_imagen(BMP *imagen, int nhilos,int opcion); // 2 sera el numero de hilos

void *filtro(void *arg);


int main(int argc, char *argv[])
{

    if (argc != 9)
    {
        printf("Los argumentos del progrma son los siguientes: ./imgconc –i [Imagen de entrada] –t [Imagen de salida] –o [Opción (1,2 o 3)] –h [Numero de hilos]");
        exit(1);
    }

    if ((strcmp(argv[1], "-i")!= 0) )
    {
        printf("El argumento para la imagen de entrada es '-i' ");
        exit(1);
    }

    if ((strcmp(argv[3], "-t") != 0) )
    {
        printf("El argumento para la imagen de salida es '-t' ");
        exit(1);
    }
    if ((strcmp(argv[5], "-o") != 0))
    {
        printf("El argumento para la opción de filtro es '-o' ");
        exit(1);
    }
    if ((strcmp(argv[7], "-h") != 0))
    {
        printf("El argumento para el numero de hilos es '-h' ");
        exit(1);
    }

    char *nomImagenEntrada = argv[2];
    char *nomImagenSalida = argv[4];
    int filtro = atoi(argv[6]);
    int nHilos = atoi(argv[8]);
    abrir_imagen(&img,nomImagenEntrada);
    printf("\n*************************************************************************");
	printf("\nIMAGEN: %s",nomImagenEntrada);
	printf("\n*************************************************************************");
	printf("\nDimensiones de la imágen:\tAlto=%d\tAncho=%d\n",img.alto,img.ancho);
    convertir_imagen(&img,nHilos,filtro); //2 sera el numero de hilos		

    	//***************************************************************************************************************************
	//1 Crear la imágen BMP a partir del arreglo img.pixel[][]
	//***************************************************************************************************************************	
	crear_imagen(&img, nomImagenSalida);
	printf("\nImágen BMP tratada en el archivo: %s\n",nomImagenSalida);
	
	//Terminar programa normalmente	
	exit (0);	




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

//********************************************************************w*******************************************************************************
//Función para crear una imagen BMP, a partir de la estructura imagen imagen (Arreglo de bytes de alto*ancho  --- 1 Byte por pixel 0-255)
//Parametros de entrada: Referencia a un BMP (Estructura BMP), Referencia a la cadena ruta char ruta[]=char *ruta
//Parametro que devuelve: Ninguno
//****************************************************************************************************************************************************



// void convertir_imagen(BMP *imagen, int nhilos, int opcion)
// {
//     int t, rc;



//     pthread_t threads[nhilos]; 

//     BMP *imgArr [nhilos];


//     for (int i = 0; i < imagen->alto; i++)
//     {
//         for (int j = 0; j < imagen->ancho; j++)
//         {
            
//         }
        
//     }
    

//     for (t = 0; t < nhilos; t++)
//     {
 
//         if (opcion == 1)
//         {
//             rc = pthread_create(&threads[t], NULL, filtro1, (void *)&imagen);
//             if (rc)
//             {
//                 printf("ERROR: código de retorno de pthread_create() es %d\n", rc);
//                 exit(-1);
//             }
//         }

//         else if(opcion == 2)
//         {
//             rc = pthread_create(&threads[t], NULL, filtro2, (void *)&imagen);
//             if (rc)
//             {
//                 printf("ERROR: código de retorno de pthread_create() es %d\n", rc);
//                 exit(-1);
//             }
//         }
//         else{
//             rc = pthread_create(&threads[t], NULL, filtro3, (void *)&imagen);
//             if (rc)
//             {
//                 printf("ERROR: código de retorno de pthread_create() es %d\n", rc);
//                 exit(-1);
//             }
//         }
//         }
    
//     for (t = 0; t < nhilos; t++)
//     {
//         pthread_join(threads[t], NULL);
//     }
// }


void convertir_imagen(BMP *imagen, int nHilos, int opcion)
{
    pthread_t hilos[nHilos];
    int altura_seccion = imagen->alto / nHilos;
    int resto = imagen->alto % nHilos;

    for (int i = 0; i < nHilos; i++)
    {
        int inicio = i * altura_seccion;
        int fin = inicio + altura_seccion;
        if (i == nHilos - 1) // En el último hilo, procesamos también el resto
        {
            fin += resto;
        }
        // Creamos una estructura de datos que contendrá los datos necesarios
        // para que el hilo procese su sección de la imagen
        datos_hilo_t *datos_hilo = malloc(sizeof(datos_hilo_t));
        datos_hilo->imagen = imagen;
        datos_hilo->inicio = inicio;
        datos_hilo->fin = fin;
        datos_hilo->opcion = opcion;
        pthread_create(&hilos[i], NULL, filtro, datos_hilo);
    }

    for (int i = 0; i < nHilos; i++)
    {
        pthread_join(hilos[i], NULL);
    }
}



void *filtro(void *arg)
{
    datos_hilo_t *datos_hilo = (datos_hilo_t *)arg;
    BMP *imagen = datos_hilo->imagen;
    int inicio = datos_hilo->inicio;
    int fin = datos_hilo->fin;
    int opcion = datos_hilo->opcion;
    unsigned char temp;
    int k;

    for (int i = inicio; i < fin; i++)
    {
        for (int j = 0; j < imagen->ancho; j++)
        {
            if (opcion==1)
            {
                temp = (unsigned char)((imagen->pixel[i][j][2]*0.3)+(imagen->pixel[i][j][1]*0.59)+ (imagen->pixel[i][j][0]*0.11));
                for (k=0;k<3;k++) imagen->pixel[i][j][k]= (unsigned char)temp; 	//Formula correcta

            }if (opcion==2)
            {
                temp = (unsigned char)(((imagen->pixel[i][j][2])+(imagen->pixel[i][j][1])+ (imagen->pixel[i][j][0]))/3);
                for (k=0;k<3;k++) imagen->pixel[i][j][k]= (unsigned char)temp;
                
            }if(opcion == 3){

                
                for (k=0;k<3;k++) imagen->pixel[i][j][k]= 255-imagen->pixel[i][j][k]; 

            }
            
            

            // Procesar pixel (i,j) de la imagen según la opción de filtro elegida
            // ...
        }
    }

    free(datos_hilo);
    pthread_exit(NULL);
}

void crear_imagen(BMP *imagen, char ruta[])
{
	FILE *archivo;	//Puntero FILE para el archivo de imágen a abrir

	int i,j,k;

	//Abrir el archivo de imágen
	archivo = fopen( ruta, "wb+" );
	if(!archivo)
	{ 
		//Si la imágen no se encuentra en la ruta dada
		printf( "La imágen %s no se pudo crear\n",ruta);
		exit(1);
	}
	
	//Escribir la cabecera de la imagen en el archivo
	fseek( archivo,0, SEEK_SET);
	fwrite(&imagen->bm,sizeof(char),2, archivo);
	fwrite(&imagen->tamano,sizeof(int),1, archivo);	
	fwrite(&imagen->reservado,sizeof(int),1, archivo);	
	fwrite(&imagen->offset,sizeof(int),1, archivo);	
	fwrite(&imagen->tamanoMetadatos,sizeof(int),1, archivo);	
	fwrite(&imagen->alto,sizeof(int),1, archivo);	
	fwrite(&imagen->ancho,sizeof(int),1, archivo);	
	fwrite(&imagen->numeroPlanos,sizeof(short int),1, archivo);	
	fwrite(&imagen->profundidadColor,sizeof(short int),1, archivo);	
	fwrite(&imagen->tipoCompresion,sizeof(int),1, archivo);
	fwrite(&imagen->tamanoEstructura,sizeof(int),1, archivo);
	fwrite(&imagen->pxmh,sizeof(int),1, archivo);
	fwrite(&imagen->pxmv,sizeof(int),1, archivo);
	fwrite(&imagen->coloresUsados,sizeof(int),1, archivo);
	fwrite(&imagen->coloresImportantes,sizeof(int),1, archivo);	
			
	//Pasar la imágen del arreglo reservado en escala de grises a el archivo (Deben escribirse los valores BGR)
	for (i=0;i<imagen->alto;i++)
	{
		for (j=0;j<imagen->ancho;j++)
		{  

                    for (k=0;k<3;k++)
		       fwrite(&imagen->pixel[i][j][k],sizeof(char),1, archivo);  //Escribir el Byte Blue del pixel 
		    
		    
		}   
	}
	//Cerrrar el archivo
	fclose(archivo);
}


// void *filtro1(BMP *imagen){

//     int i,j,k;  
//     unsigned char temp;

//         for (i=0;i<imagen->alto;i++) {
//             for (j=0;j<imagen->ancho;j++) {  
//                 temp = (unsigned char)((imagen->pixel[i][j][2]*0.3)+(imagen->pixel[i][j][1]*0.59)+ (imagen->pixel[i][j][0]*0.11));
//                 for (k=0;k<3;k++) imagen->pixel[i][j][k]= (unsigned char)temp; 	//Formula correcta
//             }   
//         }

    

// }

// void *filtro2(BMP *imagen){

//     int i,j,k;  
//     unsigned char temp;

// }


// void *filtro3(BMP *imagen){

//     int i,j,k;  
//     unsigned char temp;

// }
