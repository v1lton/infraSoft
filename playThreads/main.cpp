//#include <bits/stdc++.h>
#include <vector>
#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <iostream>
#define endl '\n'
using namespace std;

/*Classe das músicas. Possui título, cantor e duração como atributos*/
class Songs
{   public:
    string title;
    string singer;
    string duration;
};

/*Variáveis globais do código. songQueue é um vector que armazena as músicas adicionadas pelo usuário.*/
vector<Songs> songQueue;
int numLines, numColumns;
int yWinQueue, xWinQueue, linesWinQueue, columnsWinQueue;
int musicaPosition, artistaPosition, duracaoPosition;
WINDOW *winQueue, *winPlaying, *winActions;




/*Funções auxiliares*/
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

void addSongsToWinQueue() {
    for (int i = 0; i < songQueue.size(); i++) {
        string str = to_string(i + 1);
        const char *c = str.c_str();
        mvwprintw(winQueue, 2 + i, 3, c);
        
        const char *d = songQueue[i].title.c_str();
        mvwprintw(winQueue, 2 + i, musicaPosition, d);
        
        const char *e = songQueue[i].singer.c_str();
        mvwprintw(winQueue, 2 + i, artistaPosition, e);
        
        const char *f = songQueue[i].duration.c_str();
        mvwprintw(winQueue, 2 + i, duracaoPosition, f);
    }
    
    wrefresh(winQueue);
}

void drawWinQueueBox() {
    musicaPosition = columnsWinQueue/4;
    artistaPosition = columnsWinQueue * 0.5 + 7;
    duracaoPosition = columnsWinQueue - 20;
    
    wclear(winQueue);
    box(winQueue, '*', '*');
    mvwprintw(winQueue, 0, 3, "FILA");
    mvwprintw(winQueue, 1, 3, "#");
    mvwprintw(winQueue, 1, musicaPosition, "MUSICA");
    mvwprintw(winQueue, 1, artistaPosition, "ARTISTA");
    mvwprintw(winQueue, 1, duracaoPosition, "DURACAO");
    addSongsToWinQueue();
    wrefresh(winQueue);
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
    
    drawWinActionsBox(true);
    songQueue.push_back(song);
    addSongsToWinQueue();
}


void removeSong() {
    int songIndex;
    drawWinActionsBox(false);
    mvwprintw(winActions, 1, 1, "Digite o índice da música a ser removida: ");
    wmove(winActions, 1, 43);
    wscanw(winActions, "%d", &songIndex);
    wrefresh(winActions);
    
    if (songIndex > songQueue.size() || songIndex <= 0) {
        //todo
    } else {
        songIndex--;
        songQueue.erase(songQueue.begin() + songIndex);
        drawWinQueueBox();
    }
    
    drawWinActionsBox(true);
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
    
    drawWinQueueBox();
    
    box(winPlaying, '*', '*');
    mvwprintw(winPlaying, 0, 3, "MUSICA ATUAL");
    wrefresh(winPlaying);
    
    drawWinActionsBox(true);
    
    while(true) {
        noecho();
        cbreak();
        char userInput = wgetch(winActions);
        echo();
        if (userInput == 'a') {
            addSong();
        } else if (userInput == 'r') {
            removeSong();
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
