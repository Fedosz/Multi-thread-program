#include <iostream>
#include <pthread.h>
#include <vector>
#include <experimental/random>
#include <unistd.h>

class Island {
public:
    explicit Island(int n) {
        areas.resize(n);
        int random = std::experimental::randint(0,n-1); // Генирируется случайное число и в одну из зон прячется сокровище
        areas[random].hideTreasure();
    }
    struct Area {
    public:
        // Метод, который прячет сокровище
        void hideTreasure() {
            containTreasure = true;
        }
        // Метод, который ищет сокровище
        [[nodiscard]] bool findTreasure() const {
            return containTreasure;
        }
    private:
        bool containTreasure = false;
    };
    // Разбиение острова на зоны
    std::vector<Area> areas;
};

pthread_mutex_t mutex;
static int current_area = 0; // Номер текущей зоны
static int amount_of_areas = 0; // Общее количество зон
Island* treasure_island; // Остров сокровищ

struct JohnSilver {
public:
    JohnSilver() {
        treasure_found = false;
        std::cout << "John Silver has arrived to the Island\n";
    }
    void giveInformation(bool information, int groupNumber, int areaNumber) {
        if (information) {
            std::cout << "Group #" << groupNumber << " returned to the boat and reported, that in area #" << areaNumber
                      << " treasure has been found\n";
            treasure_found = information;
        } else {
            std::cout << "Group #" << groupNumber << " returned to the boat and reported, that in area #" << areaNumber
                      << " treasure hasn't been found\n";
        }
    }
    [[nodiscard]] bool isFound() const {
        return treasure_found;
    }

private:
    bool treasure_found;
};

void* searchForTreasure(void* shared_arg) {
    auto *captain = (JohnSilver*) shared_arg; // Указатель на капитана, которому надо доложить информацию
    while (!captain->isFound()) {
        usleep(100000);   // Время, которое тратит группа, чтобы дойти до следующей зоны
        pthread_mutex_lock(&mutex);
        if (current_area < amount_of_areas) {
            captain->giveInformation(treasure_island->areas[current_area].findTreasure(), pthread_self() - 1,
                                     current_area + 1);  // Доклад информации капитану о своей группе, зоне и результатах поиска
        }
        current_area++;
        pthread_mutex_unlock(&mutex);
    }
    return nullptr;
}


int main() {
    int group_counter; // Количество групп
    std::cout << "How many groups should the pirate team be devided into?\n";
    std::cin >> group_counter;
    std::cout << "How many zones should the island be divided into?\n";
    std::cin >> amount_of_areas;
    auto *captain = new JohnSilver(); // Создание нашего капитана в главном потоке, которому все остальные будут докладывать информацию
    treasure_island = new Island(amount_of_areas); // Создание нового острова, на котором спрятано сокровище
    pthread_t pirate_groups[group_counter]; // Массив потоков (каждый поток = группе пиратов)
    for (int i = 0; i < group_counter; i++) {
        pthread_create(&pirate_groups[i], NULL, searchForTreasure, (void *) captain); // Создание потоков
    }
    for (int i = 0; i < group_counter; i++) {
        pthread_join(pirate_groups[i], NULL); // Присваивание потоков к основному
    }
    std::cout << "All groups have returned to the boat, the treasure has been found, all rum is about to be drunk!";
}
