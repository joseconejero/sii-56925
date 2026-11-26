// Mundo.cpp: implementation of the CMundo class.
//
//////////////////////////////////////////////////////////////////////
#include <fstream>
#include "MundoServidor.h"
#include "glut.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

void* hilo_comandos(void* d);
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMundo::CMundo()
{
	Init();
}

CMundo::~CMundo()
{
	close(fifo);        // logger
    	close(fifoSC);
    	unlink("/tmp/FIFO_SC");
    	if (fifoCS >= 0) close(fifoCS);
}

void CMundo::InitGL()
{
	//Habilitamos las luces, la renderizacion y el color de los materiales
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	
	glMatrixMode(GL_PROJECTION);
	gluPerspective( 40.0, 800/600.0f, 0.1, 150);
}

void print(char *mensaje, int x, int y, float r, float g, float b)
{
	glDisable (GL_LIGHTING);

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT) );

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glColor3f(r,g,b);
	glRasterPos3f(x, glutGet(GLUT_WINDOW_HEIGHT)-18-y, 0);
	int len = strlen (mensaje );
	for (int i = 0; i < len; i++) 
		glutBitmapCharacter (GLUT_BITMAP_HELVETICA_18, mensaje[i] );
		
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glEnable( GL_DEPTH_TEST );
}
void CMundo::OnDraw()
{
	//Borrado de la pantalla	
   	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Para definir el punto de vista
	glMatrixMode(GL_MODELVIEW);	
	glLoadIdentity();
	
	gluLookAt(0.0, 0, 17,  // posicion del ojo
		0.0, 0.0, 0.0,      // hacia que punto mira  (0,0,0) 
		0.0, 1.0, 0.0);      // definimos hacia arriba (eje Y)    

	/////////////////
	///////////
	//		AQUI EMPIEZA MI DIBUJO
	char cad[100];
	sprintf(cad,"Jugador1: %d",puntos1);
	print(cad,10,0,1,1,1);
	sprintf(cad,"Jugador2: %d",puntos2);
	print(cad,650,0,1,1,1);
	int i;
	for(i=0;i<paredes.size();i++)
		paredes[i].Dibuja();

	fondo_izq.Dibuja();
	fondo_dcho.Dibuja();
	jugador1.Dibuja();
	jugador2.Dibuja();
	esfera.Dibuja();

	/////////////////
	///////////
	//		AQUI TERMINA MI DIBUJO
	////////////////////////////

	//Al final, cambiar el buffer
	glutSwapBuffers();
}

void CMundo::OnTimer(int value)
{	
	jugador1.Mueve(0.025f);
	jugador2.Mueve(0.025f);
	esfera.Mueve(0.025f);
	int i;
	for(i=0;i<paredes.size();i++)
	{
		paredes[i].Rebota(esfera);
		paredes[i].Rebota(jugador1);
		paredes[i].Rebota(jugador2);
	}

	jugador1.Rebota(esfera);
	jugador2.Rebota(esfera);
	if(jugador1.Rebota(esfera)){
		esfera.velocidad.x=4+2*rand()/(float)RAND_MAX;
		esfera.velocidad.y=2+2*rand()/(float)RAND_MAX;
	}
	if(jugador2.Rebota(esfera)){
		esfera.velocidad.x=-4-2*rand()/(float)RAND_MAX;
		esfera.velocidad.y=-2-2*rand()/(float)RAND_MAX;
	}
	if(fondo_izq.Rebota(esfera))
	{
		esfera.centro.x=0;
		esfera.centro.y=rand()/(float)RAND_MAX;
		esfera.velocidad.x=2+2*rand()/(float)RAND_MAX;
		esfera.velocidad.y=2+2*rand()/(float)RAND_MAX;
		puntos2++;
		char cad[200];
    		snprintf(cad, sizeof(cad),
 		"Jugador 2 marca 1 punto, lleva un total de %d puntos.\n", puntos2);
   		 write(fifo, cad, strlen(cad));
	}

	if(fondo_dcho.Rebota(esfera))
	{
		esfera.centro.x=0;
		esfera.centro.y=rand()/(float)RAND_MAX;
		esfera.velocidad.x=-2-2*rand()/(float)RAND_MAX;
		esfera.velocidad.y=-2-2*rand()/(float)RAND_MAX;
		puntos1++;
		char cad[200];
   		snprintf(cad, sizeof(cad),
             	"Jugador 1 marca 1 punto, lleva un total de %d puntos.\n", puntos1);
    		write(fifo, cad, strlen(cad));
	}
	if (puntos1 >= 3) {
    		close(fifo);
   		exit(0);
	}
	if (puntos2 >= 3) {
    		close(fifo);
    		exit(0);
	}
	/*
	// Actualiza datos del bot
	pMemCompartida->esfera = esfera;
	pMemCompartida->raqueta1 = jugador1;

	// Aplica la accion decidida por el bot
	switch (pMemCompartida->accion) {
	    case 1:  OnKeyboardDown('w', 0, 0); break;   // arriba
	    case -1: OnKeyboardDown('s', 0, 0); break;   // abajo
	    case 0:  jugador1.velocidad.y = 0;  break;   // estático
	}
	*/
	char cad[200];
	sprintf(cad, "%f %f %f %f %f %f %f %f %f %f %d %d",
        esfera.centro.x, esfera.centro.y,
        jugador1.x1, jugador1.y1, jugador1.x2, jugador1.y2,
        jugador2.x1, jugador2.y1, jugador2.x2, jugador2.y2,
        puntos1, puntos2);
	write(fifoSC, cad, strlen(cad) + 1);
}

void CMundo::OnKeyboardDown(unsigned char key, int x, int y)
{
	switch(key)
	{

	}
}

void CMundo::Init()
{
	Plano p;
//pared inferior
	p.x1=-7;p.y1=-5;
	p.x2=7;p.y2=-5;
	paredes.push_back(p);

//superior
	p.x1=-7;p.y1=5;
	p.x2=7;p.y2=5;
	paredes.push_back(p);

	fondo_izq.r=0;
	fondo_izq.x1=-7;fondo_izq.y1=-5;
	fondo_izq.x2=-7;fondo_izq.y2=5;

	fondo_dcho.r=0;
	fondo_dcho.x1=7;fondo_dcho.y1=-5;
	fondo_dcho.x2=7;fondo_dcho.y2=5;

	//a la izq
	jugador1.g=0;
	jugador1.x1=-6;jugador1.y1=-1;
	jugador1.x2=-6;jugador1.y2=1;

	//a la dcha
	jugador2.g=0;
	jugador2.x1=6;jugador2.y1=-1;
	jugador2.x2=6;jugador2.y2=1;
	fifo = open("/tmp/FIFO", O_WRONLY);
	if (fifo < 0) {
	    perror("No puede abrirse el FIFO");
	}
	// Tubería servidor -> cliente
	unlink("/tmp/FIFO_SC");
	if (mkfifo("/tmp/FIFO_SC", 0600) < 0) {
	    perror("No puede crearse FIFO_SC");
	    exit(1);
	}
	unlink("/tmp/FIFO_CS");    // por si quedó de una ejecución anterior
	fifoCS = -1;
	fifoSC = open("/tmp/FIFO_SC", O_WRONLY);   // espera a que arranque el cliente
	if (fifoSC < 0) {
	    perror("No puede abrirse FIFO_SC");
	    exit(1);
	}

	// Hilo que recibe las teclas
	pthread_create(&thid1, NULL, hilo_comandos, this);
}
void* hilo_comandos(void* d)
{
    CMundo* p = (CMundo*) d;
    p->RecibeComandosJugador();
    return NULL;
}

void CMundo::RecibeComandosJugador()
{
    // el cliente crea FIFO_CS; esperamos a que exista
    int fd;
    while ((fd = open("/tmp/FIFO_CS", O_RDONLY)) < 0)
        usleep(100000);
    fifoCS = fd;

    while (1) {
        unsigned char key;
        if (read(fifoCS, &key, 1) <= 0) break;   // el cliente se ha cerrado
        if (key == 's') jugador1.velocidad.y = -4;
        if (key == 'w') jugador1.velocidad.y = 4;
        if (key == 'l') jugador2.velocidad.y = -4;
        if (key == 'o') jugador2.velocidad.y = 4;
    }
}
