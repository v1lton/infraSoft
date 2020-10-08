#include <bits/stdc++.h>
#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#define endl '\n'

using namespace std;

class Songs
{   public:
    string title;
    string singer;
    int duration;
};

vector<Songs> songQueue;
bool firstTime = true;


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
        userInput--;
        string musicTitle = songQueue[userInput].title;
        songQueue.erase(songQueue.begin()+userInput);
        cout << "A música " << musicTitle << " foi removida com sucesso!" << endl;
    }
    cout << endl;
    return NULL;
}

//Auxiliar funcs to userInterface
string getUserInput() {
    string userInput;
    
    cout << "Digite algum dos comandos:" << endl;
    cout << "add - Adicionar música"<< endl;
    cout << "rmv - Remover música" << endl;
    cout << "lst - Listar as músicas" << endl;
    cin >> userInput;
    
    return userInput;
}

void *userInterface(void *arg) {
    string userInput;
    
    if (firstTime) {
        cout << "Bem-vinde ao ThreadPlay!" << endl;
        cout << endl;
        firstTime = false;
    }
    
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
        } else {
            cout << "Comando inválido. :(" << endl << endl;
        }
    }
    
    return NULL;
}



int main(int argc, char *argv[]) {
    pthread_t userInterfaceThread;
    
    pthread_create(&userInterfaceThread, NULL, &userInterface, NULL);
    pthread_join(userInterfaceThread, NULL);
    
    return 0;
}
