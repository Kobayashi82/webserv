#include <ncurses.h>
#include <cstdlib>
#include <cstdio>

void execute_command(const char* cmd) {
    char buffer[128];
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return;
    
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        printw("%s", buffer);
    }
    pclose(pipe);
}

void init_colors() {
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_WHITE, COLOR_BLUE);    // Título
        init_pair(2, COLOR_BLACK, COLOR_CYAN);    // Comandos
        init_pair(3, COLOR_BLACK, COLOR_YELLOW);  // Resultados
        init_pair(4, COLOR_BLACK, COLOR_RED);     // Error o alerta
        init_pair(5, COLOR_WHITE, COLOR_GREEN);   // Fondo comandos
    }
}

int main() {
    // Iniciar ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    // Inicializar colores
    init_colors();

    int ch;
    
    // Título con fondo azul
    attron(COLOR_PAIR(1));
    printw("------ Interfaz para ejecutar comandos ------\n");
    attroff(COLOR_PAIR(1));
    printw("\n");

    // Comandos en cyan con fondo verde

    attron(COLOR_PAIR(2));
    printw("1) Prueba de estrés\n\n");
    printw("2) Verificar procesos zombie\n\n");
    printw("3) Acceso al servidor por defecto\n\n");
    printw("4) Acceso al servidor secundario\n\n");
    printw("5) Sin servidor (usar servidor por defecto)\n\n");
    printw("6) Prueba de 'Body Size' (HTML cargado)\n\n");
    printw("7) Prueba de 'Body Size' (Payload too large)\n\n");
    printw("8) Prueba de método HTTP desconocido\n\n");
    printw("9) Prueba de método POST con formulario\n\n");
    printw("0) Prueba de método GET con parámetros CGI\n\n");
    printw("-) Prueba de método DELETE\n\n");
    printw("q) Salir\n\n");
    attroff(COLOR_PAIR(2));

    while ((ch = getch()) != 'q') {
        clear();
        switch(ch) {
            case 's':
                attron(COLOR_PAIR(3));
                printw("Ejecutando prueba de estrés...\n\n");
                attroff(COLOR_PAIR(3));
				refresh();
                execute_command("siege -b -c 255 -t 10S 127.0.0.1:8090");
                break;
            case '1':
                attron(COLOR_PAIR(3));
                printw("Verificando procesos zombie...\n\n");
                attroff(COLOR_PAIR(3));
                execute_command("ps --ppid $(pgrep webserv) -o pid,stat,cmd");
                break;
            case '2':
                attron(COLOR_PAIR(3));
                printw("Accediendo al servidor por defecto...\n\n");
                attroff(COLOR_PAIR(3));
                execute_command("curl --resolve default:8081:127.0.0.1 http://default:8081/support.html | tail");
                break;
            case '3':
                attron(COLOR_PAIR(3));
                printw("Accediendo al servidor secundario...\n\n");
                attroff(COLOR_PAIR(3));
                execute_command("curl --resolve secundary:8081:127.0.0.1 http://secundary:8081 | tail");
                break;
            case '4':
                attron(COLOR_PAIR(3));
                printw("Probando sin servidor (servidor por defecto)...\n\n");
                attroff(COLOR_PAIR(3));
                execute_command("curl --resolve non_valid:8081:127.0.0.1 http://non_valid:8081/support.html | tail");
                break;
            case '5':
                attron(COLOR_PAIR(3));
                printw("Probando 'Body Size' (HTML cargado)...\n\n");
                attroff(COLOR_PAIR(3));
                execute_command("curl -X POST -H \"Content-Type: plain/text\" --data \"123456789\" 127.0.0.1:8090/index.py | tail");
                break;
            case '6':
                attron(COLOR_PAIR(3));
                printw("Probando 'Body Size' (Payload too large)...\n\n");
                attroff(COLOR_PAIR(3));
                execute_command("curl -X POST -H \"Content-Type: plain/text\" --data \"1234567890\" 127.0.0.1:8090/index.py | tail");
                break;
            case '7':
                attron(COLOR_PAIR(3));
                printw("Probando método HTTP desconocido...\n\n");
                attroff(COLOR_PAIR(3));
                execute_command("curl -X \"UNKNOWN\" http://127.0.0.1:8081 | tail");
                break;
            case '8':
                attron(COLOR_PAIR(3));
                printw("Probando método POST con datos de formulario...\n\n");
                attroff(COLOR_PAIR(3));
                execute_command("curl -X POST -H \"Content-Type: application/x-www-form-urlencoded\" --data \"u=a@a.a&p=a&remember_me=off\" 127.0.0.1:8081/login.php | tail");
                break;
            case '9':
                attron(COLOR_PAIR(3));
                printw("Probando método GET con parámetros CGI...\n\n");
                attroff(COLOR_PAIR(3));
                execute_command("curl -s http://127.0.0.1:8081/cgi/sample.cgi?algunos_datos | grep algunos_datos | tail");
                break;
            case '0':
                attron(COLOR_PAIR(3));
                printw("Probando método DELETE...\n\n");
                attroff(COLOR_PAIR(3));
                execute_command("curl -sL -X \"DELETE\" http://127.0.0.1:8081/delete.php?file=some_file | head");
				printw("\n...\n");
				printw("\n<!-- Sin iniciar sesion, delete.php redirecciona a login.php -->\n");
                break;
            default:
                goto start;
        }
        
        printw("\nPresiona cualquier tecla para continuar...\n");
        getch();
		start:
        clear();
		// Título con fondo azul
    	attron(COLOR_PAIR(1));
    	printw("------ Interfaz para ejecutar comandos ------\n");
    	attroff(COLOR_PAIR(1));
    	printw("\n");
		attron(COLOR_PAIR(2));
		printw("1) Prueba de estrés\n\n");
		printw("2) Verificar procesos zombie\n\n");
		printw("3) Acceso al servidor por defecto\n\n");
		printw("4) Acceso al servidor secundario\n\n");
		printw("5) Sin servidor (usar servidor por defecto)\n\n");
		printw("6) Prueba de 'Body Size' (HTML cargado)\n\n");
		printw("7) Prueba de 'Body Size' (Payload too large)\n\n");
		printw("8) Prueba de método HTTP desconocido\n\n");
		printw("9) Prueba de método POST con formulario\n\n");
		printw("0) Prueba de método GET con parámetros CGI\n\n");
		printw("-) Prueba de método DELETE\n\n");
		printw("q) Salir\n\n");
        attroff(COLOR_PAIR(2));
    }

    // Terminar ncurses
    endwin();
    return 0;
}
