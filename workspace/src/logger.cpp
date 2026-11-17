#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

int main(int argc, char **argv) {
    int fd;
    int n;
    char buffer[1024];

    // borrar FIFO por si existiera previamente
    unlink("/tmp/FIFO");

    /* crear el FIFO */
    if (mkfifo("/tmp/FIFO",0600)<0) {
        perror("No puede crearse el FIFO");
        return(1);
    }
    /* Abrir el FIFO */
    fd = open("/tmp/FIFO", O_RDONLY);
    if (fd<0) {
        perror("No puede abrirse el FIFO");
        return(1);
    }
    /* Leer de la FIFO */
    while ((n = read(fd, buffer, sizeof(buffer))) > 0) {
	write(1, buffer, n);// X5: Imprimir el texto en la pantalla
    }
    close(fd);
    unlink("/tmp/FIFO");
    return(0);
}
