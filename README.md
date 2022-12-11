# ИДЗ 4, вариант 30, Жуков Фёдор Сергеевич, БПИ218

## Критерии выполненны на 5 баллов

Условие:
Первая задача об Острове Сокровищ. Шайка пиратов под предводительством Джона Сильвера высадилась на берег Острова Сокровищ. Не смотря на добытую карту старого Флинта, местоположение сокровищ по-прежнему остается загадкой, поэтому искать клад приходится практически на ощупь. Так как Сильвер ходит на деревянной ноге, то самому бродить по джунглям ему не с руки. Джон Сильвер поделил остров на участки, а пиратов на небольшие группы. Каждой группе поручается искать клад на одном из участков, а сам Сильвер ждет на берегу. Пираты, обшарив свою часть острова, возвращаются к Сильверу и докладывают о результатах. Требуется создать многопоточное приложение с управляющим потоком, моделирующее действия Сильвера и пиратов. Примечание. Количество участков превышает число поисковых групп.

Объяснение решения и работы программы:
На вход мы получаем два значения:
Количество групп и количество зон на острове. 
Сценарий работы такой:
В нашем управляющем потоке создается капитан (Джон Сильвер), который хранит информацию о том, нашли сокровище или нет. Затем группы пиратов начинают обыскивать остров. Перед тем как в очередной раз отправиться на поиски клада, они узнают у капитана не нашли ли еще сокровище, и, в случае если сокровище не найдено, идут в путь. Наши пираты очень быстрые и путь от капитана до новой зоны занимает всего 0.1 секунды (сделано для того, чтобы всем потокам нужно было допроверить свой участок, даже если сокровище найдено). Когда они проверили участок, они возвращаются с информацией к капитану, чтобы тот составил отчет. Они говорят ему номер своей группы, номер проверенной зоны и информацию о том было ли найдено сокровище. Как только сокровище было найдено, капитан ждет возвращения всех групп и начинает вскрывать ящики с ромом, чтобы начать праздник.

### Код на C++ 
###  [avs30.cpp](https://github.com/Fedosz/Multi-thread-program/blob/main/avs30.cpp)

```c
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

```

Примеры входных данных и выходных данных:
--
##### input: 
3 
10
##### output:
John Silver has arrived to the Island
Group #2 returned to the boat and reported, that in area #1 treasure hasn't been found

Group #1 returned to the boat and reported, that in area #2 treasure hasn't been found

Group #3 returned to the boat and reported, that in area #3 treasure hasn't been found

Group #1 returned to the boat and reported, that in area #4 treasure hasn't been found

Group #3 returned to the boat and reported, that in area #5 treasure has been found

Group #2 returned to the boat and reported, that in area #6 treasure hasn't been found

Group #1 returned to the boat and reported, that in area #7 treasure hasn't been found

All groups have returned to the boat, the treasure has been found, all rum is about to be drunk!
##### input: 
6 
13
##### output:
John Silver has arrived to the Island
Group #1 returned to the boat and reported, that in area #1 treasure hasn't been found

Group #4 returned to the boat and reported, that in area #2 treasure hasn't been found

Group #3 returned to the boat and reported, that in area #3 treasure hasn't been found

Group #6 returned to the boat and reported, that in area #4 treasure hasn't been found

Group #5 returned to the boat and reported, that in area #5 treasure hasn't been found

Group #2 returned to the boat and reported, that in area #6 treasure hasn't been found

Group #5 returned to the boat and reported, that in area #7 treasure hasn't been found

Group #6 returned to the boat and reported, that in area #8 treasure hasn't been found

Group #1 returned to the boat and reported, that in area #9 treasure has been found

Group #2 returned to the boat and reported, that in area #10 treasure hasn't been found

Group #4 returned to the boat and reported, that in area #11 treasure hasn't been found

Group #3 returned to the boat and reported, that in area #12 treasure hasn't been found

Group #6 returned to the boat and reported, that in area #13 treasure hasn't been found

All groups have returned to the boat, the treasure has been found, all rum is about to be drunk!
##### input: 
4 
20
##### output:
John Silver has arrived to the Island
Group #1 returned to the boat and reported, that in area #1 treasure hasn't been found

Group #4 returned to the boat and reported, that in area #2 treasure hasn't been found

Group #3 returned to the boat and reported, that in area #3 treasure has been found

Group #2 returned to the boat and reported, that in area #4 treasure hasn't been found

Group #1 returned to the boat and reported, that in area #5 treasure hasn't been found

Group #4 returned to the boat and reported, that in area #6 treasure hasn't been found

All groups have returned to the boat, the treasure has been found, all rum is about to be drunk!
