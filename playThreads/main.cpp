#include <bits/stdc++.h>
#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#define endl '\n'

using namespace std;

/* Classe das músicas. Possui título, cantor e duração como atributos*/
class Songs
{   public:
    string title;
    string singer;
    int duration;
};

/*Variáveis globais do código. songQueue é um vector que armazena as músicas adicionadas pelo usuário.
 fisrtTime é um booleano que auxilia as boas vindas do usuário.*/
vector<Songs> songQueue;
bool firstTime = true;

/* Função que adiciona uma música em songQueue. Nela, é pedido para o usuário digitar o nome da música, do artista e a duração em segundos da música.*/
void *addSong(void *arg) {
    Songs song;
    
    cout << endl;
    cout << "Digite o nome da musica" << endl;
    cin >> song.title;
    cout << "Digite o artista" << endl;
    cin >> song.singer;
    cout << "Digite a duracao em segundos" << endl;
    cin >> song.duration;
    songQueue.emplace_back(song);
    cout << "A música " << song.title << " foi adicionada com sucesso!" << endl;
    cout << endl;
    
    return NULL;
}

/* Função que lista as músicas adicionadas pelo usuário e que não foram removidas. A lista é apresentada em ordem de inserção da música. (A primeira música colocada na lista é a primeira da lista)*/
void *listSongs(void *arg) {
    
    cout << endl;
    if (songQueue.size() == 0) {
        cout << "Ops... Aparentemente você não tem música na fila." << endl;
    } else {
        cout << "Artista \t Musica \t Duracao" << endl;
        int i = 1;
        for (auto song: songQueue) {
            cout << i << ". " << song.singer << '\t' << song.title << '\t' << song.duration << endl;
            i++;
        }
    }
    
    cout << endl;
    
    return NULL;
}

/*Função que remove uma música escolhida pelo usuário. Ela primeiro apresenta as músicas presentes em songQueue e pede para o usuário escolher uma dessas músicas para remover.*/
void *removeSong(void *arg) {
    int userInput;
    
    cout << endl;
    if (songQueue.size() == 0) {
        cout << "Ops... Aparentemente você não tem música na fila para remover." << endl;
    } else {
        cout << "Selecione o indíce de uma das músicas da fila para remover:" << endl;
        for (int i = 0; i < songQueue.size(); i ++) {
            cout << i+1 << " " << songQueue[i].title << endl;
        }
        cin >> userInput;
        while(userInput > songQueue.size() || userInput <= 0) {
            cout << "Música inválida. Por favor, digite um indíce válido" <<endl;
            cin >> userInput;
        }
        userInput--;
        string musicTitle = songQueue[userInput].title;
        songQueue.erase(songQueue.begin()+userInput);
        cout << "A música " << musicTitle << " foi removida com sucesso!" << endl;
    }
    cout << endl;
    return NULL;
}

//Função auxiliar. Imprime os comandos possíveis e ler qual foi escolhido.
string getUserInput() {
    string userInput;
    
    cout << "Digite algum dos comandos:" << endl;
    cout << "add - Adicionar música"<< endl;
    cout << "rmv - Remover música" << endl;
    cout << "lst - Listar as músicas" << endl;
    cout << "ext - Parar a execução do ThreadPlay" << endl;
    cin >> userInput;
    
    return userInput;
}

void *userInterface(void *arg) {
    //Nessa string será armazenada o comando escolhido pelo usuário
    string userInput;
    
    //Esse condicional serve apenas para dar as boas vindas ao usuário quando o programa inicia
    if (firstTime) {
        cout << "Bem-vinde ao ThreadPlay!" << endl;
        cout << endl;
        firstTime = false;
    }
    
    /* Um laço que roda indefinidamente. Nele, são mostrados os comandos que o usuário pode escolher.
     Esse laço chama getUserInput() para imprimir os comandos e ler o comando escolhido.
     Se o comando for "add", eu crio uma thread que será responsável por adicionar as músicas no vector.
     Se o comando for "lst", eu crio uma thread que será responsável por listar as músicas no vector.
     Se o comando for "rmv", eu crio uma thread que será responsável por remover as músicas no vector.
     Nesse três casos, logo após a criação da thread eu solicito que a thread que está executando (userInterfaceThread) espere a thread recém criada terminar de executar.
     Se o comando for "ext", finalizo o programa.*/
    while (true) {
        userInput = getUserInput();
        
        if (userInput == "add") {
            pthread_t addThread;
            pthread_create(&addThread, NULL, &addSong, NULL);
            pthread_join(addThread, NULL);
        } else if (userInput == "lst") {
            pthread_t listThread;
            pthread_create(&listThread, NULL, &listSongs, NULL);
            pthread_join(listThread, NULL);
        } else if (userInput == "rmv") {
            pthread_t removeThread;
            pthread_create(&removeThread, NULL, &removeSong, NULL);
            pthread_join(removeThread, NULL);
        } else if (userInput == "ext") {
            cout << endl << "Obrigade por usar o ThreadPlay! (:" << endl;
            exit(0);
        } else {
            cout << "Comando inválido. :(" << endl << endl;
        }
    }
    
    return NULL;
}



int main(int argc, char *argv[]) {
    /*Aqui é criado o thread que vai ser responsável por pegar o comando do usuário.
     Em seguida, é dado join para que a main só prossiga depois que o userInterfaceThread acabe de executar*/
    pthread_t userInterfaceThread;

    pthread_create(&userInterfaceThread, NULL, &userInterface, NULL);
    pthread_join(userInterfaceThread, NULL);
    
    return 0;
}
