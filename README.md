# Assembly code interpreter, with calculation of the percentage of cache hits

## Описание программы
Программа моделирует исполнение ассемблерного кода (архитектура набора комманд RISC-V),
дополнительно посчитывает процент кэш-попаданий с политикой вытеснения LRU и bit-pLRU.

## Параметры программы
В `main.cpp` происходит инициализация констант,
задающих параметры кэша и оперативной памяти.

| Имя параметра | Описание                                               |
|---------------|--------------------------------------------------------|
|     MEM_SIZE           | размер памяти (в байтах)                               |
|     CACHE_SIZE           | размер кэша, без учёта служебной информации (в байтах) |
|     CACHE_LINE_SIZE           | размер кэш-линии (в байтах)                            |
|       CACHE_LINE_COUNT         | кол-во кэш-линий                                       |
|        CACHE_WAY        | ассоциативность                                        |
|       CACHE_SETS         | кол-во блоков кэш-линий                                |
|        ADDR_LEN        | длина адреса (в битах)                                 |
|        CACHE_TAG_LEN       | длина тэга адреса (в битах)                            |
|          CACHE_INDEX_LEN     | длина индекса блока кэш-линий  (в битах)               |


И запуск нужной симуляции в зависимости от параметров командной строки

| Имя параметра | Описание                                                                                                                                                               |
|---------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
|       --replacement <политика_вытеснения>        | 0, 1 или 2. <br> 0 – вывод результата моделирования для всех политик вытеснения. <br> 1 – вывод результата моделирования только для LRU. <br> 2 – только для bit-pLRU. |
|   --asm <имя_файла_с_кодом_ассемблера>            | Параметр, через который передаётся файл с кодом на ассемблере, который необходимо исполнить.                                                                           |
|--bin <имя_файла_с_машинным_кодом> | Параметр, указывающий имя файла, в который нужно записать результат перевода ассемблера в машинный код                                                                 |


## Общая структура кода

|                       |                                                                                                                                                                             |
|-----------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| AddressConfig         | Реализует логику работы с адресами ячеек оперативной памяти. <br> Параметризуется шаблонными параметрами `TagLen`, `IndexLen`, `OffsetLen`.                                 |
| ICache                | Реализует интерфейс кэша процессора. <br> Содержит методы для чтения и записи ячеек памяти. <br> Дополнительно подсчитывает и хранит в себе число запросов и кэш-попаданий. |
| LRUCache и PLRUCache  | Конкретные реализации кэша с политиками вытеснение LRU и Bit-pLru.                                                                                                          |
| MEM                   | Моделирует оперативную память компьютера.                                                                                                                                   |
| Registers             | Моделирует регистры процессора(архитектура RISC-V)                                                                                                                          |
| Interpereter          | Конструируется от набора ассемблерных команд и кэша. И исполняет данные инструкции, с данным кэшом                                                                          |
| CommandLineArgsParser | Простенький парсер аргументов командной строки.                                                                                                                             |
| AsmCommandParser      | Класс для парсинга ассемблерных команд из файла в вектор строк.                                                                                                             |

## Интересные(неочевидные) особенности реализации

### CRTP
Так как Interpreter взаимодействует исключительно с интерфейсом кэша.
Мы легко можем изменять реализацию политик вытеснения, без изменения остального кода.
Статический полиморфизм достигается благодаря CRTP-idiom,
что позволяет избежать медленных виртуальных функций.

### Registers
В архитектуре RISC-V один и тот же регистр имеет разные имена.
Поэтому для их симуляции нужна была структура типо хэш-мапы,
но так, чтобы разные ключи могли ссылаться на одно значение. Поэтому был отдельно создан вектор со значениями регистров. И хэш-мапа у которой:
key - имя регистра, value - итератор на ячейку со значением данного регистра.

Так же в архитектуре RISC-V есть особенный регистр, zero в котором всегда лежит 0, в независимости
от производимых с ним операций. Для симуляции этого в `operator[]` перед возвратом требуемого значения.
Мы предварительно зануляем значение в этом регистре.

### AddressConfig
Вся работа с адресами ячеек оперативной памяти инкапсулирована здесь.
По сути класс представляет собой namespace со статическими константами и
статическими методами. Так что негде не создаются экземпляры этого класса.

### Interpreter
Для исполнения каждой ассемблерной инструкции, создан свой метод.

Также есть статическая хэшмапа у которой: key - имя инструкции,
value - указатель на метод класса для её исполнения.

Такая техника помогает избежать ифов(что увеличивает скорость и читаемость программы),
и обеспечивает простое добавление новых инструкций(нужно просто дописать новый метод,
и положить указатель на него в мапу).

### Шаблоны(параметры кэша и оперативной памяти)
Так как все параметры системы известны на этапе компиляции.
Можно не хранить параметры классов в их полях, а занести их в шаблонные параметры.
Что и было сделано.

Так например `LRUCache` параметризуется следующими шаблонными параметрами:
`template <uint32_t CacheWaysCnt, uint8_t TagLen, uint8_t IndexLen, uint8_t OffsetLen>`

### span
В программе данными владеет лишь класс `MEM`, классы LRUCache и PLRUCache
лишь ссылаются на них с помощью span, который паразитирует на векторе лежащем в MEM.

Это позволяет избежать большого количества лишних копирований данных + нам не нужно
заботиться об обратном копировании данных из кэша в оперативную память, при удалении
какой-то линии из кэша.

### concepts в ASMCommandParser
Так выглядит конструктор ASMCommandParser:
```c++
AsmCommandParser(InIt code_beg, InIt code_end, OutIt out)
  : cur(code_beg), end(code_end), out(out) {
    parse_all_commands();
}
```
Это позволяет пользователю точнее настраивать его поведение под себя.
Например, код необязательно должен лежать в файле, для тестирования можно положить код
в std::isstream и передать в качестве code_beg и code_end, итераторы на начало и конец потока.
Также пользователь может настраивать куда именно мы будем парсить код, с помощью передаваемого
итератора out.

На данные шаблонные параметры для безопасности навешаны концепты.
```c++
template <std::input_iterator InIt, std::output_iterator<std::vector<std::string>> OutIt>
requires requires (InIt it) {{*it} -> std::convertible_to<char>;}
```