#include <bits/stdc++.h>
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
    bool wasPlayed = false;

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
int actualMusic = -1; //música atual. -1 significa que não tem música para tocar
int songDuration;
int executionTime = 0;
bool isShuffleMode = false;
bool isToRenderUI = false;
bool isPaused = false;
bool isUISetted = false;


/*Vector onde as músicas estão armazenadas*/
vector<Songs> songQueue;
vector<bool> wasTheSongPlayed;

/*Mutexes*/
pthread_mutex_t mutexQueue = PTHREAD_MUTEX_INITIALIZER;         //Mutex para fila
pthread_mutex_t mutexExecutionTime = PTHREAD_MUTEX_INITIALIZER; //Mutex para o tempo de execução da música que está sendo reproduzida
pthread_mutex_t mutexIsPaused = PTHREAD_MUTEX_INITIALIZER;      //Mutex para o estado de play/pause da faixa sendo reproduzida
pthread_mutex_t mutexIsToRenderUI = PTHREAD_MUTEX_INITIALIZER;  //Mutex para renderizar a UI

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
        mvwprintw(winActions, 1, columnsWinQueue * 0.14 + 3, "[R]remover");
        mvwprintw(winActions, 1, columnsWinQueue * 0.28 + 3, "[P]play");
        mvwprintw(winActions, 1, columnsWinQueue * 0.42 + 3, "[S]pause");
        mvwprintw(winActions, 1, columnsWinQueue * 0.56 + 3, "[N]proxima");
        mvwprintw(winActions, 1, columnsWinQueue * 0.84 + 18, "[Q]fechar");
        if (isShuffleMode) 
        {
            mvwprintw(winActions, 1, columnsWinQueue * 0.70 + 3, "[K]sequencial");
        } 
        else
        {
             mvwprintw(winActions, 1, columnsWinQueue * 0.70 + 3, "[K]aleatorio");
        }
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
void playSong()
{ //Play na execução da faixa
    while (pthread_mutex_trylock(&mutexIsPaused) == 0)
        ; //Trava modificação no bool isPaused
    isPaused = false;
    pthread_mutex_unlock(&mutexIsPaused); //Libera isPaused para modificações
}

void pauseSong()
{ // Pause na execução da faixa
    while (pthread_mutex_trylock(&mutexIsPaused) == 0)
        ; //Trava modificação no bool isPaused
    isPaused = true;
    pthread_mutex_unlock(&mutexIsPaused); //Libera isPaused para modificações
}

void changeActualMusic()
{ //Muda o nome da música que está sendo exibida na window winPlaying
    drawWinPlayingBox();
    if (actualMusic != -1)
    {
        const char *title = songQueue.at(actualMusic).title.c_str();
        mvwprintw(winPlaying, 1, 3, title); //printa o título da música disponível para ser reproduzida

        const char *singer = songQueue.at(actualMusic).singer.c_str();
        mvwprintw(winPlaying, 2, 3, singer); //printa o artista da música disponível para ser reproduzida
    }
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
    changeActualMusic();
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

    while (pthread_mutex_trylock(&mutexQueue) == 0)
        ; //bloqueia o acesso a modifição em songQueue
    while (pthread_mutex_trylock(&mutexIsPaused) == 0)
        ; //bloqueia o acesso a modifição em isPaused
    while (pthread_mutex_trylock(&mutexIsToRenderUI) == 0)
        ; //bloqueia o acesso a modificação em isToRenderUI

    songQueue.push_back(song);
    if (songQueue.size() == 1) //para dar play na faixa disponível somente se a última música adicionada for a primeira disponível a execução
    {
        isPaused = false;
        actualMusic = 0;
        songQueue.at(actualMusic).wasPlayed = true;
    }

    isToRenderUI = true;

    pthread_cond_signal(&UI_SIGNAL);      //envia sinal para atualizar a UI
    pthread_mutex_unlock(&mutexQueue);    //libera o mutex de modificação a songQueue
    pthread_mutex_unlock(&mutexIsPaused); //libera o mutex de modificação a isPaused
    pthread_mutex_unlock(&mutexIsToRenderUI); //libera o mutex de modificação a isToRenderUI
}

void changePlayingMode() //alterna entre modo sequencial ou modo aleatório
{
    if (isShuffleMode) 
    {
        for (int i = actualMusic + 1; i < songQueue.size(); i++) //faz com que na mudança aleatório -> sequencial as músicas com indíce maior que a atual fiquem com o booleano .wasPlayed verdadeiro
        {
            songQueue.at(i).wasPlayed = false;
        }
    }

    isShuffleMode = !isShuffleMode;
}

void removeSong() //Remove uma faixa de songQueue
{
    while (pthread_mutex_trylock(&mutexQueue) == 0); //bloqueia o acesso a modifição em songQueue
    while (pthread_mutex_trylock(&mutexIsPaused) == 0); //bloqueia o acesso a modificação em isPaused
    while (pthread_mutex_trylock(&mutexIsToRenderUI) == 0); //bloqueia o acesso a modificação em isToRenderUI
    
    int songIndex;
    drawWinActionsBox(false);
    mvwprintw(winActions, 1, 1, "Digite o índice da música a ser removida: ");
    wmove(winActions, 1, 43);
    wscanw(winActions, "%d", &songIndex);
    wrefresh(winActions);

    if (songIndex <= songQueue.size() && songIndex > 0)
    {
        songIndex--;
        songQueue.erase(songQueue.begin() + songIndex); //retira a música da posição indicada pelo usuário
    }

    if (songQueue.empty())
    {
        actualMusic = -1;
        isPaused = true;
        executionTime = 0;
    }

    isToRenderUI = true;

    pthread_mutex_unlock(&mutexIsToRenderUI); //libera o mutex de modificação em songQueue
    pthread_mutex_unlock(&mutexQueue); //libera o mutex de modificação em songQueue
    pthread_cond_signal(&UI_SIGNAL); //sinaliza modificação na UI
}

void nextSongSequential() //avança para a próxima música de maneira sequencial
{
    while (pthread_mutex_trylock(&mutexQueue) == 0)
        ; //bloqueia o acesso a modificação em songQueue
    while (pthread_mutex_trylock(&mutexExecutionTime) == 0)
        ; //bloqueia o acesso a modificação em executionTime
    while (pthread_mutex_trylock(&mutexIsToRenderUI) == 0); //bloqueia o acesso a modificação em isToRenderUI

    if (actualMusic < (songQueue.size() - 1) && actualMusic != -1)
    {
        actualMusic++;
        songQueue.at(actualMusic).wasPlayed = true;
        executionTime = 0;
        //songQueue = vector<Songs>(songQueue.begin() + 1, songQueue.end());
    }
    else //tocou todas as músicas
    {
        actualMusic = -1; //indica que já tocou todas as músicas
        executionTime = 0;
    }

    isToRenderUI = true;

    pthread_mutex_unlock(&mutexIsToRenderUI); //libera o mutex de modificação em isToRenderUI
    pthread_mutex_unlock(&mutexQueue);         //libera o mutex de modificação em songQueue
    pthread_mutex_unlock(&mutexExecutionTime); //libera o mutex de modificação em executionTime
    pthread_cond_signal(&UI_SIGNAL); //sinaliza modificação na UI
}

int findNextAvailableSound(int index) { //função que encontra um som do vector que está disponível para ser tocado
    int auxCount = 0; 
    index++;
    while (auxCount < songQueue.size())
    {
        if (!songQueue.at(index).wasPlayed)
        {
            return index; //posição da música disponível a ser tocada
        } 
        else 
        {
            auxCount++;
            index = (index + 1) % songQueue.size();
        }
    }
    return -1; //caso nenhuma música esteja disponível
}

void nextSongShuffle() //avança para a próxima música de maneira aleatória
{
    
    while (pthread_mutex_trylock(&mutexQueue) == 0); //bloqueia o acesso a modificação em songQueue
    while (pthread_mutex_trylock(&mutexExecutionTime) == 0); //bloqueia o acesso a modificação em executionTime
    while (pthread_mutex_trylock(&mutexIsToRenderUI) == 0); //bloqueia o acesso a modificação em isToRendeUI

    int nextIndexSong = rand() % songQueue.size(); //procura um index aleatório que seja do tamanho do vector songQueue
    if (songQueue.at(nextIndexSong).wasPlayed) //checa se a música daquele index já foi tocado
    {
        nextIndexSong = findNextAvailableSound(nextIndexSong); //caso a música tenha sido tocada, procura a mais perto que esteja disponível
    }

    actualMusic = nextIndexSong; //coloca a música atual para o indíce achado. Caso nenhuma música esteja disponível, recebe -1
    executionTime = 0; //limpa o tempo de execução que estava sendo incrementado

    if (actualMusic != -1)
    {
        songQueue.at(actualMusic).wasPlayed = true; //coloca a música atual como tocada
    }

    isToRenderUI = true; //coloca para a flag de redenrizar a UI como true

    pthread_mutex_unlock(&mutexIsToRenderUI); //libera o mutex de modificação em isToRenderUI
    pthread_mutex_unlock(&mutexQueue);         //libera o mutex de modificação em songQueue
    pthread_mutex_unlock(&mutexExecutionTime); //libera o mutex de modificação em executionTime
    pthread_cond_signal(&UI_SIGNAL); //sinaliza modificação na UI
}

void progressionBar() //realiza a progressão da barra de execução da música
{
    int totalSeconds = 1;

    if (executionTime == 0)
    {
        drawWinPlayingBox();
    }

    if (actualMusic != -1) 
    {
        totalSeconds = songQueue.empty() ? 0 : songQueue.at(actualMusic).totalSeconds(); //pega o tempo total da música que está liberada para ser executada
    }
    

    mvwhline(winPlaying, 1, 30, ACS_CKBOARD, int(((float)executionTime / totalSeconds) * (numColumns - 20))); //desenha a linha da faixa disponível para ser executada
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

        while (!isToRenderUI)
        {
            pthread_cond_wait(&UI_SIGNAL, &mutexIsToRenderUI); //fica esperando pelo sinal de atualização
        }

        addSongsToWinQueue(); //adiciona músicas à fila na window WinQueue
        progressionBar();     //responsável pela atualização da barra de execução
        isToRenderUI = false;
    }
}

void *playingTime(void *arg) //responsável por atualizar o tempo de execução da música
{
    while (true)
    {
        while (pthread_mutex_trylock(&mutexExecutionTime) == 0); //bloqueia modificação em executionTime

        if (!songQueue.empty() && !isPaused && actualMusic != -1) //checa se a lista de sons não está vazia e se não a música não está pausada
        {
            executionTime++; //precisa atualizar a UI
        }

        sleep(1); //espera um segundo para poder seguir com a função, isso faz com que o contador aumente a cada segundo

        while (pthread_mutex_trylock(&mutexIsToRenderUI) == 0) //bloqueia o mutex relacionado ao bool isToRenderUI
            ;

        isToRenderUI = true; 

        pthread_mutex_unlock(&mutexExecutionTime); //desbloqueia modificação em executionTime
        pthread_mutex_unlock(&mutexIsToRenderUI); //desbloqueia o mutex relacionado ao bool isToRenderUI
        pthread_cond_signal(&UI_SIGNAL); //Manda sinal para atualizar a UI 
    }
}

void *changeSong(void *arg) //responsável por mudar a faixa que está sendo executada
{
    while (true) 
    {
        if (actualMusic != -1 && !songQueue.empty() && executionTime == songQueue.at(actualMusic).totalSeconds()) //checa se tem música para tocar, se songQueue não está vazio e se chegou no tempo limite da música
        {   
            if (!isShuffleMode) 
            {
                nextSongSequential(); //chama a função que realiza a troca de música de forma sequencial
            }
            else
            {
                nextSongShuffle(); //chama a função que realiza a troca de música de forma aleatória
            }
            
            while (pthread_mutex_trylock(&mutexIsToRenderUI) == 0) //bloqueia o mutex relacionado ao bool isToRenderUI
                ;
            isToRenderUI = true;
            pthread_mutex_unlock(&mutexIsToRenderUI); //desbloqueia o mutex relacionado ao bool isToRenderUI
            pthread_cond_signal(&UI_SIGNAL); //Manda sinal para atualizar a UI
        }
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
        }
        else if (userInput == 'p' || userInput == 'P')
        {
            playSong();
        }
        else if (userInput == 'n' || userInput == 'N')
        {
            if (isShuffleMode)
            {
                nextSongShuffle();
            }
            else
            {
                nextSongSequential();
            }
        }
        else if (userInput == 'k' || userInput == 'K')
        {
            while(pthread_mutex_trylock(&mutexQueue) == 0);
            changePlayingMode();
            pthread_mutex_unlock(&mutexQueue);
        }
        else if (userInput == 'q' || userInput == 'Q')
        {
            endwin();
            exit(0);
        }
        else
        {
            drawWinActionsBox(true);
        }

        drawWinActionsBox(true);
    }
}

int main()
{
    /*Declaração das threads*/
    pthread_t userInterfaceThread;
    pthread_t watchUserKeyboardThread;
    pthread_t changeSongThread;
    pthread_t playingTimeThread;

    /*Criação das threads*/
    pthread_create(&userInterfaceThread, NULL, &userInterface, NULL);
    pthread_create(&watchUserKeyboardThread, NULL, &watchUserKeyboard, NULL);
    pthread_create(&changeSongThread, NULL, &changeSong, NULL);
    pthread_create(&playingTimeThread, NULL, &playingTime, NULL);

    /*A main precisa esperar até concluir a execução das threads*/
    pthread_join(userInterfaceThread, NULL);
    pthread_join(watchUserKeyboardThread, NULL);
    pthread_join(changeSongThread, NULL);
    pthread_join(playingTimeThread, NULL);

    return 0;
}