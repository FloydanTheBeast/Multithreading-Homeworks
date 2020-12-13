#include <iostream>
#include <pthread.h>
#include <queue>
#include <unistd.h>
#include <chrono>
#include <thread>

using namespace std;

unsigned int WORKING_TIME = 10;
unsigned int CLIENTS_NUMBER = 10;

pthread_mutex_t queueMutex;
int id = 0;
bool haveClientsEnded = false;

struct Client {
    int id;
    int timeToCome;
};

queue<Client*> clients;

struct Barber {
public:
    bool isBusy = false;

    void makeCut() {
        this_thread::sleep_for(chrono::milliseconds(100));
        auto client = clients.front();

        pthread_mutex_lock(&queueMutex);

        cout << "\tПарикмахер приступил к стижке клиента №" << client->id << endl;
        isBusy = true;
        clients.pop();
        this_thread::sleep_for(chrono::milliseconds(rand() % 300 + 300));
        isBusy = false;
        cout << "\tПаркмахер закончил стрижку клиента №" << client->id << ". Длина очереди - " << clients.size() << endl;

        pthread_mutex_unlock(&queueMutex);
    }
};

void* startWorking(void* barberData) {
    Barber* barber = (Barber*) barberData;

    cout << "Парикхмахер выходит на работу" << endl;
    chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    for (;;) {
        if (chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - begin).count() > WORKING_TIME) {
            cout << "Рабочий день заканчивается, парикмахерская закрывается";
            exit(0);
        }

        if (haveClientsEnded && clients.empty()) {
            cout << "Клиенты закончились, парикмахерская закрывается";
            exit(0);
        }

        if (!barber->isBusy && !clients.empty()) barber->makeCut();
    }
}

void *goToQueue(void* clientData) {
    Client* client = (Client*) clientData;
    this_thread::sleep_for(chrono::milliseconds(client->timeToCome));

    pthread_mutex_lock(&queueMutex);

    clients.push(client);
    cout << "Посетитель №" << client->id << " встаёт в очередь. Длина очереди - " << clients.size() << endl;

    pthread_mutex_unlock(&queueMutex);
};


int main(int argc, char** argv) {
    if (argc > 1)
        CLIENTS_NUMBER = atoi(argv[1]);
    else
        cout << "Количество клиентов не задано, будет использовано значение по умолчанию 10" << endl;

    if (argc > 2) {
        WORKING_TIME = atoi(argv[2]);
        if (WORKING_TIME == 0) {
            cout << "Время работы должно быть положительным числом" << endl;
            exit(1);
        }
    }
    else
        cout << "Максимальное время работы программы не задано, будет использовано значение по умолчанию 10 секунд" << endl;

    cout << "Парикмахерская открывается со следующими параметрами: " << endl;
    cout << "\tМаксимальное количество клиентов за день - " << CLIENTS_NUMBER << endl;
    cout << "\tВремя работы - " << WORKING_TIME << " секунд."<< endl << endl;

    pthread_t barberThread;
    Barber barber;

    pthread_create(&(barberThread), NULL, startWorking, &barber);
    this_thread::sleep_for(chrono::milliseconds(100));

    auto* clientsThreads = (pthread_t*) malloc(CLIENTS_NUMBER * sizeof(pthread_t));
    auto* clientsData = (Client*) malloc(CLIENTS_NUMBER * sizeof(Client));

    for (int i = 0; i < CLIENTS_NUMBER; i++) {
        clientsData[i].id = id++;
        clientsData[i].timeToCome = rand() % (WORKING_TIME * 1000) + 100;

        pthread_create(&(clientsThreads[i]), NULL, goToQueue, &clientsData[i]);
    }

    for(int i = 0; i < CLIENTS_NUMBER; i++) {
        pthread_join(clientsThreads[i], NULL);
    }

    haveClientsEnded = true;

    pthread_join(barberThread, NULL);

    return 0;
}