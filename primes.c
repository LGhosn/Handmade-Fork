#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define READ 0
#define WRITE 1

/*
 *@brief Recibe un pipe de lectura por parametro, filtra los numeros que recibe
 *por el pipe y envia los numeros filtrados por otro pipe
 *@param pipeLectura pipe por el que recibe los numeros a filtrar
 *@return void
 */
void
filtrarPrimos(int pipeLectura[])
{
	int pipeEscritura[2];
	if (pipe(pipeEscritura) == -1) {
		perror("Error al crear el pipe");
		_exit(EXIT_FAILURE);
	}

	close(pipeLectura[WRITE]);

	pid_t childPid = fork();
	if (childPid == -1) {
		perror("Error al crear el proceso hijo");
		_exit(EXIT_FAILURE);
	} else if (childPid == 0) {
		// child
		int numPrimo, num;

		// El primer numero que recibo siempre va a ser primo
		int bytesRead =
		        read(pipeLectura[READ], &numPrimo, sizeof(numPrimo));
		if (bytesRead == -1) {
			perror("Error al leer del pipe");
			_exit(EXIT_FAILURE);
		} else if (bytesRead > 0) {
			printf("primo %d\n", numPrimo);
			fflush(stdout);
		} else {
			// Si no lei nada, termino
			close(pipeLectura[READ]);
			return;
		}

		while (bytesRead > 0) {
			bytesRead = read(pipeLectura[READ], &num, sizeof(num));
			if (bytesRead == -1) {
				perror("Error al leer del pipe");
				_exit(EXIT_FAILURE);
			}

			// Si el numero no es multiplo del primo, lo mando por
			// el pipe para que lo reciba el siguiente filtro
			if (num % numPrimo != 0) {
				if (write(pipeEscritura[WRITE], &num, sizeof(num)) ==
				    -1) {
					perror("Error al escribir en el pipe");
					_exit(EXIT_FAILURE);
				}
			}
		}

		close(pipeLectura[READ]);

		filtrarPrimos(pipeEscritura);

		close(pipeEscritura[READ]);
	} else {
		// parent
		// Cierro todo y espero a que mi hijo termine para no dejarlo zombie
		close(pipeLectura[READ]);
		close(pipeEscritura[READ]);
		close(pipeEscritura[WRITE]);

		wait(NULL);
	}

	return;
}

/*
 *@brief Recibe un numero maximo por parametro, crea un pipe y envia los numeros
 *del 2 al numero maximo por el pipe para que sean filtrados
 *@param numMaximo numero maximo a enviar por el pipe
 *@return void
 */
void
listarYFiltrarPrimos(int numMaximo)
{
	int pipeEscritura[2];
	if (pipe(pipeEscritura) == -1) {
		perror("Error al crear el pipe");
		_exit(EXIT_FAILURE);
	}
	for (int i = 2; i <= numMaximo; i++) {
		if (write(pipeEscritura[WRITE], &i, sizeof(i)) == -1) {
			perror("Error al escribir en el pipe");
			_exit(EXIT_FAILURE);
		}
	}
	filtrarPrimos(pipeEscritura);
}


int
main(int argc, char *argv[])
{
	if (argc != 2) {
		perror("Error: Mal uso del programa. Debe proveer un unico "
		       "numero entero como argumento.\n");
		_exit(EXIT_FAILURE);
	}

	int numMaximo = atoi(argv[1]);

	listarYFiltrarPrimos(numMaximo);

	return 0;
}
