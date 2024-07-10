#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#ifndef NARGS
#define NARGS 4
#endif

struct Argumentos {
	char *argv[NARGS + 1];
	int cont;
};

void
ejecutarComando(struct Argumentos *argumentos)
{
	// Asegura que el último elemento sea NULL
	argumentos->argv[argumentos->cont] = NULL;
	pid_t childPid = fork();
	if (childPid == -1) {
		perror("Error al crear el proceso hijo");
		_exit(EXIT_FAILURE);
	} else if (childPid == 0) {
		// child
		execvp(argumentos->argv[0], argumentos->argv);
		perror("Error al ejecutar el comando");
		_exit(EXIT_FAILURE);
	} else {
		// parent
		wait(NULL);
	}
}

void
xargs(char *comando)
{
	struct Argumentos argumentos = { .argv[0] = comando, .cont = 1 };

	char *linea = NULL;
	size_t len = 0;
	ssize_t leido;

	while ((leido = getline(&linea, &len, stdin))) {
		if (leido != EOF) {
			// Elimino el \n del final
			linea[leido - 1] = '\0';
			argumentos.argv[argumentos.cont] = linea;
			argumentos.cont++;
			linea = NULL;
		}
		if (argumentos.cont == NARGS + 1 || leido == EOF) {
			int aux = argumentos.cont;
			ejecutarComando(&argumentos);
			for (int i = 1; i < aux; i++) {
				free(argumentos.argv[i]);
			}
			if (leido == EOF) {
				break;
			}
			argumentos.cont = 1;
		}
	}
}

int
main(int argc, char *argv[])
{
	if (argc < 2) {
		perror("Error: Mal uso del programa. Debe proveer al menos un "
		       "argumento.\n");
		_exit(EXIT_FAILURE);
	}

	xargs(argv[1]);

	return 0;
}
