#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream> 
using namespace std;



void llenararreglo(int arreglo[], int tamanio, int rango) {
	int i;
	// cambia la semilla del rand
	srand(rango+ time(0));
	for (i = 0; i < tamanio; i++) {
		arreglo[i] = rand() % tamanio; 
	}
}
//funcion que imprime un arreglo
void imprimi_arreglo(int arreglo[],int largo){
    int i;
    for (i=0;i<largo;i++){
        cout<<arreglo[i]<<",";
    }
}
int compare (const void * a, const void * b)
{
  return ( *(int*)a - *(int*)b );
}

//funcion que une 2 arreglos de forma ordenada
void unir(int sub_arreglo1[],int sub_arreglo2[],int largo_sub_arreglo,int arreglo_destino[]){
    int a1=0,a2=0,i=0;
    while(i<largo_sub_arreglo*2){

        if(a1>=largo_sub_arreglo && a2 < largo_sub_arreglo){
            arreglo_destino[i]=sub_arreglo2[a2];
            a2++;

        }
        else{
            if(a2>=largo_sub_arreglo && a1<largo_sub_arreglo){
              arreglo_destino[i]=sub_arreglo1[a1];
              a1++;  
            }
            else{
                if(sub_arreglo1[a1]<sub_arreglo2[a2]){
                    arreglo_destino[i]=sub_arreglo1[a1];
                    a1++;
                }
                else{
                    arreglo_destino[i]=sub_arreglo2[a2];
                    a2++;
                }
            }

        }
        i++;
    }
}
void copiar_arreglo(int arreglo_origen[],int arreglo_destino[],int largo_arreglos){
    int i;
    for(i=0;i<largo_arreglos;i++){
        arreglo_destino[i]=arreglo_origen[i];
    }
}

int main(int argc, char *argv[]) 
{ 
    
    int rango, procesos, num_datos;
    int sub_arreglo_tam;
 
    MPI_Init(&argc, &argv);//iniciamos el entorno MPI
    MPI_Comm_rank(MPI_COMM_WORLD,&rango);//obtenemos el identificador del proceso
    MPI_Comm_size(MPI_COMM_WORLD,&procesos);//obtenemos el numero de procesos
 
    // se obtiene el numero de datos como argumento
    num_datos = atoi(argv[1]);
    //se obtiene el tamaño de los sub arreglos al dividir el arreglo a ordenar por el numero de procesos
    sub_arreglo_tam=num_datos/procesos;
    
    // se crea el arreglo a ordenar
    int *Arreglo =new int[num_datos];
    
    //el proceso 0 genera un vector desordenado.
    if(rango==0){
        llenararreglo(Arreglo,num_datos,rango);
    }
    
    //se crea un sub arreglo el cual cada proceso debe ordenar

    int *sub_arreglo =new int[sub_arreglo_tam];
 
    //Repartimos el vector entre todos los procesos.

    MPI_Scatter(Arreglo,sub_arreglo_tam,MPI_INT,sub_arreglo,sub_arreglo_tam,MPI_INT,0,MPI_COMM_WORLD);

    //Cada proceso ordena su parte usando qsort.
    qsort(sub_arreglo,sub_arreglo_tam,sizeof(int), compare);

    MPI_Status status;
    int paso=1;
    int tamanio=sub_arreglo_tam;
    //ahora los subvectores ordenados se unen en un solo vector
    while(paso<procesos){

        if(rango%(2*paso)==0){// verifica si es el vecino izquiero
        
            if(rango+paso<procesos){
                //se crean 2 array uno que contiene el array del vecino y otro que contendra la union del array del vecino y el del sub arreglo
                int *vecino=new int[tamanio];
                int *arreglomasvecino=new int[tamanio*2];
                
                //se recive el array
                MPI_Recv(vecino,tamanio,MPI_INT,rango+paso,0,MPI_COMM_WORLD,&status); 
                
                //se une el sub arreglo del proceso y el sub array del proceso vecino
                unir(sub_arreglo,vecino,tamanio,arreglomasvecino);
                
                //ahora sub arreglo duplica su tamaño para contener la union de ambos arreglos
                sub_arreglo=new int[tamanio*2];
                
                //se copia el arreglo temporal arreglomasvecino en el subarreglo
                copiar_arreglo(arreglomasvecino,sub_arreglo,tamanio*2);
                
                //se duplica el indicador del tamaño del arreglo del proceso
                tamanio=tamanio*2;
                delete[]vecino;
                delete[]arreglomasvecino; 
            }

        }
        else{//vecino derecho envia su array al vecino de la izquierda
            MPI_Send(sub_arreglo,tamanio,MPI_INT,rango-paso,0,MPI_COMM_WORLD);
            break;

        }
        
        //se duplica el paso
        paso=paso*2;
    }
    
    //el proceso 0 imprime el arreglo ordenado
    /* if(rango==0){
        cout<<endl<<"array ordenado"<<endl<<"[";
        imprimi_arreglo(sub_arreglo,num_datos);
        cout<<"]"<<endl;
    } */

    delete[]Arreglo;
    delete[]sub_arreglo; 
    MPI_Finalize();
    return 0;
}
