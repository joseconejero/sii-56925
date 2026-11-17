#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "DatosMemCompartida.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc,char* argv[])
{
	int i;
	DatosMemCompartida* pMem;
	int fd=open("/tmp/datosBot.txt", O_RDWR);
	if (fd == -1)
	{
		perror("Error al abrir el archivo"); 
		return 1;
	}
	pMem = (DatosMemCompartida *) mmap(NULL, sizeof(DatosMemCompartida), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	if (pMem == MAP_FAILED)
	{
		perror("Error en la proyeccion mmap"); 
		return 1;
	}

	while(1)
	{
		usleep(25000);
		float centroRaqueta = (pMem->raqueta1.y1 + pMem->raqueta1.y2) / 2;

        	if (pMem->esfera.centro.y > centroRaqueta)
           		pMem->accion = 1;      // arriba
        	else if (pMem->esfera.centro.y < centroRaqueta)
            		pMem->accion = -1;     // abajo
        	else
            		pMem->accion = 0;      // nada
		}
		 munmap(pMem, sizeof(DatosMemCompartida));
	return 0;
}
