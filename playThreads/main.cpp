#include <vector>
#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <iostream>
#define endl '\n'
using namespace std;

/*Struct das músicas. Possui título, cantor e duração como atributos*/
struct Songs
{
    string title;
    string singer;
    int minutes;
    int seconds;

    int totalSeconds() 
    { 
        return minutes * 60 + seconds;
    }
};

/****Variáveis globais do código***/
/*Variáveis relacionadas às janelas*/
WINDOW *winQueue, *winPlaying, *winActions;
int numLines, numColumns;
int yWinQueue, xWinQueue, linesWinQueue, columnsWinQueue;
int musicaPosition, artistaPosition, duracaoPosition;

/*Váriaveis de estado*/
bool isUISetted = false;
int songDuration;
bool isPaused = false;
int executionTime = 0;

/*Vector onde as músicas estão armazenadas*/
vector<Songs> songQueue;

/*Mutexes*/
pthread_mutex_t mutexQueue = PTHREAD_MUTEX_INITIALIZER; //Mutex para fila
pthread_mutex_t mutexExecutionTime = PTHREAD_MUTEX_INITIALIZER; //Mutex para o tempo de execução da música que está sendo reproduzida
pthread_mutex_t mutexIsPaused = PTHREAD_MUTEX_INITIALIZER; //Mutex para o estado de play/pause da faixa sendo reproduzida

/*Signals*/
pthread_cond_t UI_SIGNAL = PTHREAD_COND_INITIALIZER; //sinal para indicar atualização na UI

/****Funções****/
/*Funções para desenhar as janelas*/
void drawWinPlayingBox()
{
    wclear(winPlaying);
    box(winPlaying, '*', '*');
    mvwprintw(winPlaying, 0, 3, "MUSICA ATUAL");
    wrefresh(winPlaying);
}

void drawWinActionsBox(bool complete)
{
    if (complete)
    {
        wclear(winActions);
        box(winActions, '*', '*');
        mvwprintw(winActions, 0, 3, "COMANDOS");
        mvwprintw(winActions, 1, 3, "[A]adicionar");
        mvwprintw(winActions, 1, columnsWinQueue * 0.2 + 3, "[R]remover");
        mvwprintw(winActions, 1, columnsWinQueue * 0.4 + 3, "[P]play");
        mvwprintw(winActions, 1, columnsWinQueue * 0.6 + 3, "[S]pause");
        mvwprintw(winActions, 1, columnsWinQueue * 0.8 + 3, "[N]proxima");
        wrefresh(winActions);
    }
    else
    {
        wclear(winActions);
        box(winActions, '*', '*');
        mvwprintw(winActions, 0, 3, "COMANDOS");
    }
}

void drawWinQueueBox()
{
    musicaPosition = columnsWinQueue / 4;
    artistaPosition = columnsWinQueue * 0.5 + 7;
    duracaoPosition = columnsWinQueue - 20;

    wclear(winQueue);
    box(winQueue, '*', '*');
    mvwprintw(winQueue, 0, 3, "FILA");
    mvwprintw(winQueue, 1, 3, "#");
    mvwprintw(winQueue, 1, musicaPosition, "MUSICA");
    mvwprintw(winQueue, 1, artistaPosition, "ARTISTA");
    mvwprintw(winQueue, 1, duracaoPosition, "DURACAO");
    wrefresh(winQueue);
}

/*Funções que ajudam ao funcionamento da reprodução das faixas*/
void playSong() { //Play na execução da faixa
    while(pthread_mutex_trylock(&mutexIsPaused) == 0); //Trava modificação no bool isPaused
    isPaused = false;
    pthread_mutex_unlock(&mutexIsPaused); //Libera isPaused para modificações
}

void pauseSong() { // Pause na execução da faixa
    while(pthread_mutex_trylock(&mutexIsPaused) == 0); //Trava modificação no bool isPaused
    isPaused = true;
    pthread_mutex_unlock(&mutexIsPaused); //Libera isPaused para modificações
}

void changeActualMusic() { //Muda o nome da música que está sendo exibida na window winPlaying
    drawWinPlayingBox();
    const char *title = songQueue.at(0).title.c_str();
    mvwprintw(winPlaying, 1, 3, title); //printa o título da música disponível para ser reproduzida
    
    const char *singer = songQueue.at(0).singer.c_str();
    mvwprintw(winPlaying, 2, 3, singer); //printa o artista da música disponível para ser reproduzida

    wrefresh(winPlaying);
}

void addSongsToWinQueue() //Adiciona as músicas que estão no vector songsQueue para a window WinQueue
{
    drawWinQueueBox();
    for (int i = 0; i < songQueue.size(); i++) //loop que passa por todas as songs presente em songsQueue.
    {
        string str = to_string(i + 1);
        const char *c = str.c_str();
        mvwprintw(winQueue, 2 + i, 3, c);

        const char *d = songQueue[i].title.c_str();
        mvwprintw(winQueue, 2 + i, musicaPosition, d);

        const char *e = songQueue[i].singer.c_str();
        mvwprintw(winQueue, 2 + i, artistaPosition, e);

        mvwprintw(winQueue, 2 + i, duracaoPosition, "%d%", songQueue[i].minutes);

        mvwprintw(winQueue, 2 + i, duracaoPosition + 2, "%d", songQueue[i].seconds);
    }
    wrefresh(winQueue);

    if (!songQueue.empty()) { //Coloca a música disponível a ser tocada como a música 0
        changeActualMusic();
    }

}

void addSong() //Pede informações sobre a música nova a ser adicionada na songsQueue
{
   
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
    mvwprintw(winActions, 1, 1, "Digite os minutos da música: ");
    wrefresh(winActions);
    wmove(winActions, 1, 30);
    wscanw(winActions, "%d", &song.minutes);

    drawWinActionsBox(false);
    mvwprintw(winActions, 1, 1, "Digite os segundos da música: ");
    wrefresh(winActions);
    wmove(winActions, 1, 31);
    wscanw(winActions, "%d", &song.seconds);

    drawWinActionsBox(true);

    while (pthread_mutex_trylock(&mutexQueue) == 0); //bloqueia o acesso a modifição em songQueue
    while (pthread_mutex_trylock(&mutexIsPaused) == 0); //bloqueia o acesso a modifição em isPaused

    songQueue.push_back(song);
    if (songQueue.size() == 1) //para dar play na faixa disponível somente se a última música adicionada for a primeira disponível a execução
    {
        isPaused = false;
    }

    pthread_cond_signal(&UI_SIGNAL); //envia sinal para atualizar a UI
    pthread_mutex_unlock(&mutexQueue); //libera o mutex de modificação a songQueue
    pthread_mutex_unlock(&mutexIsPaused);//libera o mutex de modificação a isPaused
}

void removeSong() //Remove uma faixa de songQueue
{
    while (pthread_mutex_trylock(&mutexQueue) == 0); //bloqueia o acesso a modifição em songQueue
    int songIndex;
    drawWinActionsBox(false);
    mvwprintw(winActions, 1, 1, "Digite o índice da música a ser removida: ");
    wmove(winActions, 1, 43);
    wscanw(winActions, "%d", &songIndex);
    wrefresh(winActions);

    if (songIndex > songQueue.size() || songIndex <= 0)
    {
        //Não faz nenhuma ação nesse caso
    }
    else
    {
        songIndex--;
        songQueue.erase(songQueue.begin() + songIndex); //retira a música da posição indicada pelo usuário
    }

    drawWinActionsBox(true);
    pthread_cond_signal(&UI_SIGNAL); //sinaliza modificação na UI
    pthread_mutex_unlock(&mutexQueue); //libera o mutex de modificação em songQueue
}

void nextSong() //avança para a próxima música
{
    while (pthread_mutex_trylock(&mutexQueue) == 0); //bloqueia o acesso a modificação em songQueue
    while (pthread_mutex_trylock(&mutexExecutionTime) == 0); //bloqueia o acesso a modificação em executionTime
    if (!songQueue.empty())
    {
        executionTime = 0;
        songQueue = vector<Songs>(songQueue.begin() + 1, songQueue.end());
    }

    pthread_cond_signal(&UI_SIGNAL); //sinaliza modificação na UI
    pthread_mutex_unlock(&mutexQueue); //libera o mutex de modificação em songQueue
    pthread_mutex_unlock(&mutexExecutionTime); //libera o mutex de modificação em executionTime
}

void progressionBar() //realiza a progressão da barra de execução da música
{
    if (executionTime == 0) {
        drawWinPlayingBox();
    }

    int totalSeconds = songQueue.empty() ? 0: songQueue.at(0).totalSeconds(); //pega o tempo total da música que está liberada para ser executada

    mvwhline(winPlaying, 1, 30, ACS_CKBOARD, int(((float) executionTime / totalSeconds ) * (numColumns - 20))); //desenha a linha da faixa disponível para ser executada
    wrefresh(winPlaying);
}

void setUserInterface() //relação com ncurses. Desenha a interface para o usuário
{
    if (!isUISetted)
    {
        initscr();
        int yWinPlaying, xWinPlaying, linesWinPlaying, columnsWinPlaying;
        int yWinActions, xWinActions, linesWinActions, columnsWinActions;

        getmaxyx(stdscr, numLines, numColumns);

        yWinQueue = 0;
        xWinQueue = 0;
        linesWinQueue = 0.75 * numLines;
        columnsWinQueue = numColumns;

        yWinPlaying = (yWinQueue + linesWinQueue) + 2; 
        xWinPlaying = 0;
        linesWinPlaying = 4;
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

        drawWinPlayingBox();

        drawWinActionsBox(true);
    }
}

/*Threads Functions*/
void *userInterface(void *arg) //responsável por montar e atualizar a UI
{
    setUserInterface(); //monta a UI
    isUISetted = true;

    while (true)
    {
        pthread_cond_wait(&UI_SIGNAL, &mutexQueue); //fica esperando pelo sinal de atualização

        addSongsToWinQueue(); //adiciona músicas à fila na window WinQueue
        progressionBar(); //responsável pela atualização da barra de execução
    }
}

void *playingSongTime(void *arg) //responsável pelo tempo de execução da música
{
    while (true)
    {
        while (pthread_mutex_trylock(&mutexExecutionTime) == 0); //bloqueia modificação em executionTime
        if (!isPaused && !songQueue.empty() && executionTime < songQueue.at(0).totalSeconds()) 
        {
            executionTime++;
        }
        else if (!songQueue.empty() && executionTime == songQueue.at(0).totalSeconds())
        {
            nextSong();
        }
        else if (songQueue.empty())
        {
            executionTime = 0;
            isPaused = true;
        }
        sleep(1);
        pthread_cond_signal(&UI_SIGNAL); //sinaliza a UI para modificação
        pthread_mutex_unlock(&mutexExecutionTime); //libera modificação em executionTime
    }
}

void *watchUserKeyboard(void *arg) //responsável por observar os comandos escolhidos pelo usuário
{
    while (true)
    {
        char userInput = wgetch(winActions);
        if (userInput == 'a' || userInput == 'A')
        {
            addSong();
        }
        else if (userInput == 'r' || userInput == 'R')
        {
            removeSong();
        }
        else if (userInput == 's' || userInput == 'S')
        {
            pauseSong(); 
            drawWinActionsBox(true);
        }
        else if (userInput == 'p' || userInput == 'P')
        {
            playSong();
            drawWinActionsBox(true);
        }
        else if (userInput == 'n' || userInput == 'N')
        {
            nextSong();
            drawWinActionsBox(true);
        } else {
            drawWinActionsBox(true);
        }
    }
}

int main()
{
    /*Declaração das threads*/
    pthread_t userInterfaceThread;
    pthread_t playingSongTimeThread;
    pthread_t watchUserKeyboardThread;

    /*Criação das threads*/
    pthread_create(&userInterfaceThread, NULL, &userInterface, NULL);
    pthread_create(&playingSongTimeThread, NULL, &playingSongTime, NULL);
    pthread_create(&watchUserKeyboardThread, NULL, &watchUserKeyboard, NULL);

    /*A main precisa esperar até concluir a execução das threads*/
    pthread_join(userInterfaceThread, NULL);
    pthread_join(playingSongTimeThread, NULL);
    pthread_join(watchUserKeyboardThread, NULL);

    return 0;
}