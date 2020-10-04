
#include <bits/stdc++.h>
#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#define endl '\n'

using namespace std;

class Song
{   public:
    string nome;
    string cantor;
    int duracao;
};

vector<Song> line;
pthread_mutex_t theMutex = PTHREAD_MUTEX_INITIALIZER;
string entrada = "N";

void* interface(void* arg) {
    while(true) {
        while(pthread_mutex_trylock(&theMutex));

        if (line.size() == 0){
            cout << "Você ainda não tem música na lista do Musicada!"<< endl;
            cout << "Deseja adicionar uma nova música? [S/N]";
            cin >> entrada;
            cout << endl;
        } else {
            cout << "Artista \t Musica \t Duracao" << endl;
            for (auto musica : line) {
                cout << musica.cantor << '\t' << musica.nome << '\t' << musica.duracao << endl;
            }
        }

        pthread_mutex_unlock(&theMutex);
        sleep(1);
    }
}

void* receberInput(void* arg) {
    while(true) {
        while(pthread_mutex_trylock(&theMutex));
        Song musicaLeitura;

        cout << "Digite o nome da musica" << endl;
        cin >> musicaLeitura.nome;

        cout << "Digite o artista" << endl;
        cin >> musicaLeitura.cantor;

        cout << "Digite a duracao em segundos" << endl;
        cin >> musicaLeitura.duracao;

        line.emplace_back(musicaLeitura);
        pthread_mutex_unlock(&theMutex);
    }

}

int main(int argc, char *argv[]) {
    pthread_t threads[2];

    pthread_create(&threads[0], NULL, &interface, NULL);
    pthread_create(&threads[1], NULL, &receberInput, NULL);

    while(true) {
        pthread_join(threads[0], NULL);
        pthread_join(threads[1], NULL);
    }


    return 0;
}

