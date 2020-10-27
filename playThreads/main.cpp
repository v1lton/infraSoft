//#include <bits/stdc++.h>
#include <vector>
#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <iostream>
#define endl '\n'
using namespace std;


//
///* Função que adiciona uma música em songQueue. Nela, é pedido para o usuário digitar o nome da música, do artista e a duração em segundos da música.*/
//void *addSong(void *arg) {
//    Songs song;
//
//    cout << endl;
//    cout << "Digite o nome da musica" << endl;
//    cin >> song.title;
//    cout << "Digite o artista" << endl;
//    cin >> song.singer;
//    cout << "Digite a duracao em segundos" << endl;
//    cin >> song.duration;
//    songQueue.emplace_back(song);
//    cout << "A música " << song.title << " foi adicionada com sucesso!" << endl;
//    cout << endl;
//
//    return NULL;
//}
//
///* Função que lista as músicas adicionadas pelo usuário e que não foram removidas. A lista é apresentada em ordem de inserção da música. (A primeira música colocada na lista é a primeira da lista)*/
//void *listSongs(void *arg) {
//
//    cout << endl;
//    if (songQueue.size() == 0) {
//        cout << "Ops... Aparentemente você não tem música na fila." << endl;
//    } else {
//        cout << "Artista \t Musica \t Duracao" << endl;
//        int i = 1;
//        for (auto song: songQueue) {
//            cout << i << ". " << song.singer << '\t' << song.title << '\t' << song.duration << endl;
//            i++;
//        }
//    }
//
//    cout << endl;
//
//    return NULL;
//}
//
///*Função que remove uma música escolhida pelo usuário. Ela primeiro apresenta as músicas presentes em songQueue e pede para o usuário escolher uma dessas músicas para remover.*/
//void *removeSong(void *arg) {
//    int userInput;
//
//    cout << endl;
//    if (songQueue.size() == 0) {
//        cout << "Ops... Aparentemente você não tem música na fila para remover." << endl;
//    } else {
//        cout << "Selecione o indíce de uma das músicas da fila para remover:" << endl;
//        for (int i = 0; i < songQueue.size(); i ++) {
//            cout << i+1 << " " << songQueue[i].title << endl;
//        }
//        cin >> userInput;
//        while(userInput > songQueue.size() || userInput <= 0) {
//            cout << "Música inválida. Por favor, digite um indíce válido" <<endl;
//            cin >> userInput;
//        }
//        userInput--;
//        string musicTitle = songQueue[userInput].title;
//        songQueue.erase(songQueue.begin()+userInput);
//        cout << "A música " << musicTitle << " foi removida com sucesso!" << endl;
//    }
//    cout << endl;
//    return NULL;
//}
//
////Função auxiliar. Imprime os comandos possíveis e lê qual foi escolhido.
//string getUserInput() {
//    string userInput;
//
//    cout << "Digite algum dos comandos:" << endl;
//    cout << "add - Adicionar música"<< endl;
//    cout << "rmv - Remover música" << endl;
//    cout << "lst - Listar as músicas" << endl;
//    cout << "ext - Parar a execução do ThreadPlay" << endl;
//    cin >> userInput;
//
//    return userInput;
//}
//
//void *userInterface(void *arg) {
//    //Nessa string será armazenada o comando escolhido pelo usuário
//    string userInput;
//
//    //Esse condicional serve apenas para dar as boas vindas ao usuário quando o programa inicia
//    if (firstTime) {
//        cout << "Bem-vinde ao ThreadPlay!" << endl;
//        cout << endl;
//        firstTime = false;
//    }
//
//    /* Um laço que roda indefinidamente. Nele, são mostrados os comandos que o usuário pode escolher.
//     Esse laço chama getUserInput() para imprimir os comandos e ler o comando escolhido.
//     Se o comando for "add", eu crio uma thread que será responsável por adicionar as músicas no vector.
//     Se o comando for "lst", eu crio uma thread que será responsável por listar as músicas no vector.
//     Se o comando for "rmv", eu crio uma thread que será responsável por remover as músicas no vector.
//     Nesse três casos, logo após a criação da thread eu solicito que a thread que está executando (userInterfaceThread) espere a thread recém criada terminar de executar.
//     Se o comando for "ext", finalizo o programa.*/
//    while (true) {
//        userInput = getUserInput();
//
//        if (userInput == "add") {
//            pthread_t addThread;
//            pthread_create(&addThread, NULL, &addSong, NULL);
//            pthread_join(addThread, NULL);
//        } else if (userInput == "lst") {
//            pthread_t listThread;
//            pthread_create(&listThread, NULL, &listSongs, NULL);
//            pthread_join(listThread, NULL);
//        } else if (userInput == "rmv") {
//            pthread_t removeThread;
//            pthread_create(&removeThread, NULL, &removeSong, NULL);
//            pthread_join(removeThread, NULL);
//        } else if (userInput == "ext") {
//            cout << endl << "Obrigade por usar o ThreadPlay! (:" << endl;
//            exit(0);
//        } else {
//            cout << "Comando inválido. :(" << endl << endl;
//        }
//    }
//
//    return NULL;
//}
//
//
//
//int main(int argc, char *argv[]) {
//    /*Aqui é criado o thread que vai ser responsável por pegar o comando do usuário.
//     Em seguida, é dado join para que a main só prossiga depois que o userInterfaceThread acabe de executar*/
//    pthread_t userInterfaceThread;
//
//    pthread_create(&userInterfaceThread, NULL, &userInterface, NULL);
//    pthread_join(userInterfaceThread, NULL);
//
//    return 0;
//}

/*Classe das músicas. Possui título, cantor e duração como atributos*/
class Songs
{   public:
    string title;
    string singer;
    string duration;
};

/*Variáveis globais do código. songQueue é um vector que armazena as músicas adicionadas pelo usuário.*/
vector<Songs> songQueue;
int numLines, numColumns, musicaPosition, artistaPosition, duracaoPosition;
int yWinQueue, xWinQueue, linesWinQueue, columnsWinQueue;
WINDOW *winQueue, *winPlaying, *winActions;

/*Funções auxiliares*/
void refreshWinQueue() {
    for (int i = 0; i < songQueue.size(); i++) {
        string str = to_string(i);
        const char *c = str.c_str();
        mvwprintw(winQueue, 2 + i, 3, c);
        
        const char *d = songQueue[i].title.c_str();
        mvwprintw(winQueue, 2 + i, musicaPosition, d);
        
        const char *e = songQueue[i].singer.c_str();
        mvwprintw(winQueue, 2 + i, artistaPosition, e);
        
        const char *f = songQueue[i].duration.c_str();
        mvwprintw(winQueue, 2 + i, duracaoPosition, f);
        
        wrefresh(winQueue);
    }
}

void drawWinActionsBox(bool complete) {
    if (complete) {
        wclear(winActions);
        box(winActions, '*', '*');
        mvwprintw(winActions, 0, 3, "COMANDOS");
        mvwprintw(winActions, 1, 3, "[A]adicionar");
        mvwprintw(winActions, 1, columnsWinQueue * 0.2 + 3, "[R]remover");
        mvwprintw(winActions, 1, columnsWinQueue * 0.4 + 3, "[P]play");
        mvwprintw(winActions, 1, columnsWinQueue * 0.6 + 3, "[S]pause");
        mvwprintw(winActions, 1, columnsWinQueue * 0.8 + 3, "[N]proxima");
        wrefresh(winActions);
    } else {
        wclear(winActions);
        box(winActions, '*', '*');
        mvwprintw(winActions, 0, 3, "COMANDOS");
    }
}



/*Funções principais*/
void addSong() {
    char input[100];
    Songs song;
    
    drawWinActionsBox(false);
    mvwprintw(winActions, 1, 1, "Digite o nome da música: ");
    wrefresh(winActions);
    wmove(winActions, 1, 26);
    wgetstr(winActions, input);
    song.title = input;
    
    drawWinActionsBox(false);
    mvwprintw(winActions, 1, 1, "Digite o nome do artista: ");
    wrefresh(winActions);
    wmove(winActions, 1, 27);
    wgetstr(winActions, input);
    song.singer = input;
    
    drawWinActionsBox(false);
    mvwprintw(winActions, 1, 1, "Digite a duração da música: ");
    wrefresh(winActions);
    wmove(winActions, 1, 29);
    wgetstr(winActions, input);
    song.duration = input;
    
    songQueue.push_back(song);
    refreshWinQueue();
}

void removeSong(int songIndex) {
    
}

void setUserInterface() {
    initscr();
    
    
    int yWinPlaying, xWinPlaying, linesWinPlaying, columnsWinPlaying;
    int yWinActions, xWinActions, linesWinActions, columnsWinActions;
    
    getmaxyx(stdscr, numLines, numColumns);
    
    yWinQueue = 0;
    xWinQueue = 0;
    linesWinQueue = 0.75 * numLines;
    columnsWinQueue = numColumns;
    
    yWinPlaying = (yWinQueue + linesWinQueue) + 2; //Pega a última posição y de winArtists e soma dois
    xWinPlaying = 0;
    linesWinPlaying = ((numLines * 0.25) - 2) / 2;
    columnsWinPlaying = numColumns;
    
    yWinActions = (yWinPlaying + linesWinPlaying) + 1;
    xWinActions = 0;
    linesWinActions = 3;
    columnsWinActions = numColumns;
    
    winQueue = newwin(linesWinQueue, columnsWinQueue, yWinQueue, xWinQueue);
    winPlaying = newwin(linesWinPlaying, columnsWinPlaying, yWinPlaying, xWinPlaying);
    winActions = newwin(linesWinActions, columnsWinActions, yWinActions, xWinActions);
    refresh();
    
    box(winQueue, '*', '*');
    /*Eu super tentei fazer algo bonito, mas não deu certo*/
    musicaPosition = columnsWinQueue/4;
    artistaPosition = columnsWinQueue * 0.5 + 7;
    duracaoPosition = columnsWinQueue - 20;
    mvwprintw(winQueue, 0, 3, "FILA");
    mvwprintw(winQueue, 1, 3, "#");
    mvwprintw(winQueue, 1, musicaPosition, "MUSICA");
    mvwprintw(winQueue, 1, artistaPosition, "ARTISTA");
    mvwprintw(winQueue, 1, duracaoPosition, "DURACAO");
    wrefresh(winQueue);
    
    box(winPlaying, '*', '*');
    mvwprintw(winPlaying, 0, 3, "MUSICA ATUAL");
    wrefresh(winPlaying);
    
    drawWinActionsBox(true);
    
    while(true) {
        cbreak();
        char userInput = wgetch(winActions);
        if (userInput == 'a') {
            addSong();
        } else if (userInput == 'b') {
            drawWinActionsBox(false);
            mvwprintw(winActions, 1, 1, "Digite o índice da música a ser removida: ");
            userInput = wgetch(winActions);
            int songIndex = (int)(userInput);
            removeSong(songIndex);
        }
    }
    
    getch();
    endwin();
}

void *userInterface(void *arg) {
    setUserInterface();
    return NULL;
}

int main() {
    pthread_t userInterfaceThread;
    
    pthread_create(&userInterfaceThread, NULL, &userInterface, NULL);
    pthread_join(userInterfaceThread, NULL);
    
    return 0;
}
