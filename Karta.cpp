#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <map>
#include <fstream> //Для файлов
#include <sstream> //Для файлов
#include <windows.h>   //чтобы центерпринт работало
#include <algorithm>
#include <cctype>
//#include <typeinfo>
//setlocale(LC_ALL, "Russian");
using namespace std;
//Принты
void print(int chislo)
{
    std::cout << chislo << std::endl;
}
void print(std::string fraza)
{
    std::cout << fraza << std::endl;
}
void r_print(std::string fraza)
{
    std::cout << fraza;
}
void center_print(const std::string& text) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    int consoleWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int padding = (consoleWidth - text.length()) / 2;
    std::cout << std::string(padding, ' ') << text << std::endl;
}
//Консты
const int Cours_USD = 80;
const int Cours_EUD = 90;
const double Comissiya = (rand() % 5) * 0.01 + 0.94;
const string Visa = "VISA";
const string MasterCard = "MasterCard";
const string Mir = "Мир";
const string Vash_balanc_msg = "| Ваш баланс: ";
const string Val[] = { " руб.\n", " евро\n" , " дол.\n"};

// Структура для записи операций
struct Operation {
    string type;        // тип операции: "popolnenie", "snatie", "convertacia"
    double amount;      // сумма операции
    string valuta;      // валюта операции
    string details;     // дополнительные детали
};

// Структура для хранения данных карты
struct Card {
    string number;
    string pin;
    double balance_RUB = 0;
    double balance_EUD = 0;
    double balance_USD = 0;
    vector<Operation> operations; // история операций
};

// Структура для банковского счета
struct BankAccount {
    string accountNumber;    // номер счета
    double balance_RUB = 0;  // баланс в рублях
    double balance_EUD = 0;  // баланс в евро
    double balance_USD = 0;  // баланс в долларах
    vector<Operation> operations; // история операций для этого счета
};

// Структура для клиента банка
struct Client {
    string name;                   // имя клиента
    string clientId;               // уникальный идентификатор клиента
    vector<string> cards;          // список карт клиента
    vector<string> accounts;       // список счетов клиента
    string mainAccount;            // основной счет клиента
};

// Глобальные переменные для хранения данных
map<string, Card> cards;           // номер карты -> данные карты
map<string, Client> clients;       // clientId -> данные клиента
map<string, BankAccount> accounts; // номер счета -> данные счета
string loggedInClient;             // текущий авторизованный клиент
string loggedInCard;               // текущая авторизованная карта

// Функция, которая проверяет все ли символы - числа
bool allDigits(const string& str) {
    return !str.empty() and all_of(str.begin(), str.end(), [](unsigned char c) {
        return isdigit(c);
        });
}

// Функция, которая проверяет все ли символы из английского алфавита
bool Eng_Alpha(const string& str)
{
    return !str.empty() && all_of(str.begin(), str.end(), [](unsigned char c)
        {
            return isalpha(c) and !(c >= 0xC0 && c <= 0xFF); // английские буквы
        });
}

// Функция, которая проверяет все ли символы из русского алфавита
bool Ru_Alpha(const string& str) {
    return !str.empty() && all_of(str.begin(), str.end(), [](unsigned char c)
        {
            return (c >= 0xC0 && c <= 0xFF); // русские буквы
        });
}

// Функция для настройки консоли
void SetConsoleRussian() {
    SetConsoleCP(1251);        // Устанавливаем кодировку ввода
    SetConsoleOutputCP(1251);  // Устанавливаем кодировку вывода
    setlocale(LC_ALL, "Russian");
}

// Функция для резервного сохранения операций
void rezervnoe_sohranenie(const Operation& newOperation) {
    const string filename = "rezervnoe_sohranenye.txt";
    vector<Operation> sushestvuyushie_operacii;

    // Чтение существующих операций из файла
    ifstream inFile(filename); //Открытие для чтения с файла
    if (inFile.is_open()) {
        string line;
        while (getline(inFile, line)) {
            if (!line.empty()) {
                Operation op;  //Вызов структуры "Операции"
                //print(line); 
                stringstream ss(line);                                                                  /////////
                //print(line);
                string part;

                // Парсинг строки формата: type|amount|valuta|details
                // Строка разбивается по "|"
                getline(ss, op.type, '|');

                getline(ss, part, '|');
                op.amount = stod(part); //преобразует string в double

                getline(ss, op.valuta, '|');
                getline(ss, op.details);

                sushestvuyushie_operacii.push_back(op); //Добавляет в конец вектора
            }
        }
        inFile.close();
    }

    // Проверка на дубликат
    bool isDuplicate = false;
    for (const auto& op : sushestvuyushie_operacii) {
        if (op.type == newOperation.type &&
            op.amount == newOperation.amount &&
            op.valuta == newOperation.valuta &&
            op.details == newOperation.details) {
            isDuplicate = true;
            break;
        }
    }

    // Если операция не дубликат, добавляем в файл
    if (!isDuplicate) {
        ofstream outFile(filename, ios::app); //Открыл файл для записи
        if (outFile.is_open()) {
            outFile << newOperation.type << "|"
                << newOperation.amount << "|"
                << newOperation.valuta << "|"
                << newOperation.details << endl;
            outFile.close();
        }
    }
}

// Функция добавления операции в историю
void addOperation(Card& card, const string& type, double amount, const string& valuta, const string& details = "") {
    Operation op; //Вызов структуры "Операции"
    op.type = type;
    op.amount = amount;
    op.valuta = valuta;
    op.details = details;
    card.operations.push_back(op);  //Добавить в конец
    rezervnoe_sohranenie(op); // Резервное сохранение операции
}

// Перегруженная функция добавления операции для банковского счета
void addOperation(BankAccount& account, const string& type, double amount,const string& valuta, const string& details = "") {
    Operation op; // создаем новую операцию
    op.type = type; // устанавливаем тип операции
    op.amount = amount; // устанавливаем сумму операции
    op.valuta = valuta; // устанавливаем валюту операции
    op.details = details; // устанавливаем дополнительные детали
    account.operations.push_back(op); // добавляем операцию в историю операций счета
    rezervnoe_sohranenie(op); // сохраняем операцию в резервный файл
}

// Функция генерации номера карты по формату
string generateCardNumber(string type) {                                //заменить на 16
    string number;

    if (type == Visa) {
        number = "4";
        for (int i = 0; i < 1; i++) {
            number += to_string(rand() % 10);
        }
    }
    else if (type == MasterCard) {
        number = "5";
        for (int i = 0; i < 1; i++) {
            number += to_string(rand() % 10);
        }
    }
    else if (type == Mir) {
        number = "2";
        for (int i = 0; i < 1; i++) {
            number += to_string(rand() % 10);
        }
    }
    return number;
}

// Функция генерации PIN-кода
string generatePIN() {
    string pin;
    for (int i = 0; i < 1; i++) {                                           //заменить на 4
        pin += to_string(rand() % 10);
    }
    return pin;
}

// Функция генерации номера банковского счета
string generateAccountNumber() {
    string accountNumber = "40817"; // Такие у рублёвых счетов
    for (int i = 0; i < 15; i++) { 
        accountNumber += to_string(rand() % 10);
    }
    return accountNumber;
}

// Функция создания нового клиента и карты
void generateNewKC() {
    SetConsoleRussian();
    Client newClient;       // добавить нового клиента
    Card newCard;           // добавить новую карту
    BankAccount newAccount; // добавить новый счёт
    print("Введите ваше имя, оно дожно состоять только из букв русского или английского языка");
    r_print("Моё имя: ");
    string name = "";
    cin >> name;
    if ((Ru_Alpha(name) or Eng_Alpha(name))==0) {
            return print("Некорректная запись");
    }

    newClient.name = name;
    //cin.ignore(); // очистить буфер ввода
    //getline(cin, newClient.name); // считываю полное имя клиента
 
    // Генерируем уникальный ID клиента
    newClient.clientId = to_string(rand() % 900000 + 100000);

    // Выбираем тип карты
    print("Выберите тип карты:");
    print("1. VISA");
    print("2. MasterCard");
    print("3. Мир");

    char choice;
    cin >> choice;

    string type;
    switch (choice) {
    case '1': type = Visa; break;
    case '2': type = MasterCard; break;
    case '3': type = Mir; break;
    default:
        print("Неверный выбор!");
        return;
    }

    // Генерирую данные карты
    newCard.number = generateCardNumber(type); // генерирую номер карты
    newCard.pin = generatePIN();               // генерирую PIN-код

    // Создаю банковский счет
    newAccount.accountNumber = generateAccountNumber(); // генерирую номер счета
    newAccount.balance_RUB = 10000;

    // Связываю карту со счетом
    newCard.balance_RUB = 0;
    newCard.balance_EUD = 0; 
    newCard.balance_USD = 0;

    // Настраиваем клиента
    newClient.cards.push_back(newCard.number);              // добавляю карту клиенту
    newClient.accounts.push_back(newAccount.accountNumber); // добавляю счет клиенту
    newClient.mainAccount = newAccount.accountNumber;       // устанавливаю основной счет

    // Сохраняем все данные
    cards[newCard.number] = newCard;                   // сохранение карты
    accounts[newAccount.accountNumber] = newAccount;   // сохранение счёта
    clients[newClient.clientId] = newClient;           // сохранение клиента

    // Выводим информацию пользователю
    cout << "Карта и счет успешно созданы!\n";
    cout << "Ваш ID клиента: " << newClient.clientId << endl;
    cout << "Номер карты: " << newCard.number << endl;
    cout << "PIN-код: " << newCard.pin << endl;
    cout << "Номер счета: " << newAccount.accountNumber << endl;
    cout << "Запомните эти данные!\n";
}

// Функция авторизации в системе
void login() {
    string cardNumber, pin;

    r_print("Введите номер карты: ");
    cin >> cardNumber;
    r_print("Введите PIN-код: ");cin >> pin;

    // Проверяю существование карты
    if (cards.find(cardNumber) == cards.end()) {
        print("Карта не найдена!");
        return;
    }

    // Проверяем PIN-код
    if (cards[cardNumber].pin != pin) { // если PIN не совпадает
        cout << "Неверный PIN-код!\n";
        return; // выходим из функции
    }

    // Находим клиента по карте
    string clientId = "";
    for (const auto& client : clients) { // перебираем всех клиентов
        for (const auto& card : client.second.cards) { // перебираем карты клиента
            if (card == cardNumber) { // если нашли нужную карту
                clientId = client.first; // запоминаем ID клиента
                break; // выходим из внутреннего цикла
            }
        }
        if (!clientId.empty()) break; // выходим из внешнего цикла если нашли
    }

    if (clientId.empty()) { // если клиент не найден
        cout << "Ошибка: клиент не найден!\n";
        return;
    }

    loggedInClient = clientId; // сохраняем авторизованного клиента
    loggedInCard = cardNumber; // сохраняем авторизованную карту
    cout << "Авторизация успешна! Добро пожаловать, " << clients[clientId].name << "!\n";
}

// Функция смены PIN-кода
void changePIN() {
    string oldPIN, newPIN1, newPIN2;

    r_print("Введите старый PIN-код: ");
    cin >> oldPIN;
    // Проверяем старый PIN
    if (cards[loggedInCard].pin != oldPIN) {
        print("Неверный старый PIN-код!");
        return;
    }

    r_print("Введите новый PIN-код: ");
    cin >> newPIN1;
    // Проверяем длину и значения PIN-кода
    if (newPIN1.length() != 4 or allDigits(newPIN1) == false){
        print("PIN-код должен состоять из 4 цифр!");
        return;
    }
    r_print("Повторите новый PIN-код: ");
    cin >> newPIN2;

    // Проверяем совпадение новых PIN-кодов
    if (newPIN1 != newPIN2) {
        print("PIN-коды не совпадают!");
        return;
    }
    cards[loggedInCard].pin = newPIN1;
    print("PIN-код успешно изменен!");
}

// Функция просмотра баланса
void checkbalance() {
    cout << Vash_balanc_msg << cards[loggedInCard].balance_RUB << Val[0];
    cout << Vash_balanc_msg << cards[loggedInCard].balance_EUD << Val[1];
    cout << Vash_balanc_msg << cards[loggedInCard].balance_USD << Val[2];
}

// Функция снятия наличных
void tuda() {
    double summa;
    cout << "Введите сумму для снятия: ";
    cin >> summa;

    if (summa <= 0) { // проверяем что сумма положительная
        cout << "Неверная сумма!\n";
        return;
    }

    Card& card = cards[loggedInCard]; // получаем данные карты

    if (summa > card.balance_RUB) { // проверяем достаточно ли денег на карте
        cout << "Недостаточно средств на карте!\n";
        return;
    }

    card.balance_RUB -= summa; // снимаем деньги с карты

    // Добавляем запись о операции
    addOperation(card, "snatie", summa, "RUB", "");

    cout << "Снято " << summa << Val[0];
    cout << "Новый баланс на карте: " << card.balance_RUB << Val[0];
}

// Функция пополнения счета
void suda() {
    double summa;
    cout << "Введите сумму рублей для пополнения счета: ";
    cin >> summa;

    if (summa <= 0) {
        cout << "Неверная сумма!\n";
        return;
    }

    double sum_after_com = summa * Comissiya;

    Client& client = clients[loggedInClient]; // получаем данные текущего клиента
    BankAccount& account = accounts[client.mainAccount]; // получаем основной счет клиента

    account.balance_RUB += sum_after_com;

    // Добавляем запись о операции (используем перегруженную функцию для счета)
    addOperation(account, "popolnenie_scheta", summa, "RUB",
        "После комиссии: " + to_string(sum_after_com) + " RUB");

    cout << "Счет успешно пополнен на " << sum_after_com << Val[0];
    cout << "Новый баланс на счете: " << account.balance_RUB << Val[0];
}

// Функция показа аналитики операций
void analitika() {
    if (loggedInClient.empty()) { // проверяем что клиент авторизован
        cout << "Ошибка: клиент не авторизован!\n";
        return; // выходим если клиент не авторизован
    }

    Client& client = clients[loggedInClient]; // получаем данные текущего клиента
    Card& card = cards[loggedInCard]; // получаем данные текущей карты
    BankAccount& account = accounts[client.mainAccount]; // получаем основной счет клиента

    // Объединяем операции с карты и со счета
    vector<Operation> allOperations; // создаем вектор для всех операций
    allOperations.insert(allOperations.end(), card.operations.begin(), card.operations.end()); // добавляем операции с карты
    allOperations.insert(allOperations.end(), account.operations.begin(), account.operations.end()); // добавляем операции со счета

    if (allOperations.empty()) { // проверяем есть ли операции вообще
        cout << "История операций пуста.\n";
        return; // выходим если операций нет
    }

    center_print("=== АНАЛИТИКА ОПЕРАЦИЙ ==="); // выводим заголовок по центру

    // Статистика по типам операций
    map<string, int> operationCount; //Подчёт количества операций
    /*
    "popolnenie":  5,  // 5 операций пополнения
    "snatie":      3,  // 3 операции снятия
    "convertacia": 2   // 2 операции конвертации
    */

    map<string, double> operationAmount; //Подсчёт суммы операций
    /*
    "popolnenie":  25000.0,  // всего пополнено на 25000 руб
    "snatie":      15000.0,  // всего снято на 15000 руб
    "convertacia": 5000.0    // всего сконвертировано на 5000 руб
    */

    for (const auto& op : allOperations) { // перебираем все операции
        operationCount[op.type]++; // увеличиваем счетчик для этого типа операции
        operationAmount[op.type] += op.amount; // добавляем сумму к остальной этого типа
    }

    cout << "Статистика по операциям:\n";
    for (const auto& stat : operationCount) { // перебираем статистику по типам операций
        string typeName; // тип операции
        if (stat.first == "popolnenie") typeName = "Пополнение карты"; // если тип - пополнение карты
        else if (stat.first == "popolnenie_scheta") typeName = "Пополнение счета"; // если тип - пополнение счета
        else if (stat.first == "snatie") typeName = "Снятие наличных"; // если тип - снятие наличных
        else if (stat.first == "convertacia") typeName = "Конвертация валют"; // если тип - конвертация
        else if (stat.first == "perevod_na_kartu") typeName = "Перевод на карту"; // если тип - перевод на карту
        else if (stat.first == "perevod_so_scheta") typeName = "Перевод со счета"; // если тип - перевод со счета
        else typeName = stat.first; // если тип неизвестен, используем оригинальное название

        cout << "  " << typeName << ": " << stat.second << " операций на сумму " << operationAmount[stat.first] << " RUB\n"; // выводим статистику
    }

    // Подробная история операций
    cout << "\n=== ПОДРОБНАЯ ИСТОРИЯ ОПЕРАЦИЙ ===\n";
    for (size_t i = 0; i < allOperations.size(); ++i) { // перебираем все операции по порядку
        const auto& op = allOperations[i]; // получаем текущую операцию
        string typeName; // переменная для читаемого названия типа операции
        string source; // переменная для указания источника операции

        // Определяем читаемое название типа операции
        if (op.type == "popolnenie") typeName = "Пополнение карты";
        else if (op.type == "popolnenie_scheta") typeName = "Пополнение счета";
        else if (op.type == "snatie") typeName = "Снятие наличных";
        else if (op.type == "convertacia") typeName = "Конвертация валют";
        else if (op.type == "perevod_na_kartu") typeName = "Перевод на карту";
        else if (op.type == "perevod_so_scheta") typeName = "Перевод со счета";
        else typeName = op.type; // если тип неизвестен

        // Определяем источник операции (карта или счет)
        bool fromCard = false; // флаг что операция с карты
        for (const auto& cardOp : card.operations) { // проверяем есть ли эта операция в истории карты
            if (cardOp.type == op.type && cardOp.amount == op.amount && cardOp.details == op.details) {
                fromCard = true; // если нашли, устанавливаем флаг
                break; // выходим из цикла
            }
        }

        source = fromCard ? "[КАРТА]" : "[СЧЕТ]"; // определяем источник операции

        cout << i + 1 << ". " << source << " " << typeName << ": " << op.amount << " " << op.valuta; // выводим основную информацию
        if (!op.details.empty()) { // если есть дополнительные детали
            cout << " (" << op.details << ")"; // выводим детали
        }
        cout << endl;
    }

    cout << "\n=== ОБЩАЯ СТАТИСТИКА ===\n";
    cout << "|Всего операций: " << allOperations.size() << endl; // выводим общее количество операций
    cout << "|Операций по карте: " << card.operations.size() << endl; // выводим количество операций по карте
    cout << "|Операций по счету: " << account.operations.size() << endl; // выводим количество операций по счету

    double totalIncome = 0; // переменная для общего дохода
    double totalExpense = 0; // переменная для общих расходов

    for (const auto& op : allOperations) { // перебираем все операции для подсчета доходов и расходов
        if (op.type == "popolnenie"  or op.type == "popolnenie_scheta" or op.type == "perevod_so_scheta") {
            totalIncome += op.amount; // добавляем к доходам
        }
        else if (op.type == "snatie" or op.type == "perevod_na_kartu") {
            totalExpense += op.amount; // добавляем к расходам
        }
        // операции "convertacia" не учитываются в доходах/расходах, так как это просто перевод между валютами
    }

    cout << "|Общий доход: " << totalIncome << " RUB\n";
    cout << "|Общий расход: " << totalExpense << " RUB\n";
    cout << "|Баланс операций: " << (totalIncome - totalExpense) << " RUB\n";
}

// Функция просмотра информации о клиенте и счетах
void clientInfo() {
    Client& client = clients[loggedInClient]; // получаем данные клиента
    Card& card = cards[loggedInCard];         // получаем данные карты

    cout << "=== ИНФОРМАЦИЯ О КЛИЕНТЕ ===\n";
    cout << "|Имя: " << client.name << endl;
    cout << "|ID клиента: " << client.clientId << endl;
    cout << "|Основной счет: " << client.mainAccount << endl;

    // Показываем балансы на счетах
    BankAccount& mainAccount = accounts[client.mainAccount];
    cout << "\n=== БАЛАНСЫ НА СЧЕТАХ ===\n";
    cout << "На основном счете:\n";
    cout << "|  RUB: " << mainAccount.balance_RUB << Val[0];
    cout << "|  EUD: " << mainAccount.balance_EUD << Val[1];
    cout << "|  USD: " << mainAccount.balance_USD << Val[2];

    cout << "\nНа карте:\n";
    cout << "|  RUB: " << card.balance_RUB << Val[0];
    cout << "|  EUD: " << card.balance_EUD << Val[1];
    cout << "|  USD: " << card.balance_USD << Val[2];
}

// Функция перевода денег между счетом и картой
void transferAccountToCard() {
    double amount;
    cout << "Введите сумму для перевода со счета на карту: ";
    cin >> amount; // считываем сумму перевода

    if (amount <= 0) { // проверяем что сумма положительная
        cout << "Неверная сумма!\n";
        return;
    }

    Client& client = clients[loggedInClient]; // получаем данные клиента
    BankAccount& account = accounts[client.mainAccount]; // получаем основной счет
    Card& card = cards[loggedInCard]; // получаем данные карты

    if (amount > account.balance_RUB) { // проверяем достаточно ли денег на счете
        cout << "Недостаточно средств на счете!\n";
        return;
    }

    // Выполняем перевод
    account.balance_RUB -= amount; // снимаем со счета
    card.balance_RUB += amount;    // зачисляем на карту

    // Добавляем записи об операциях
    addOperation(card, "perevod_na_kartu", amount, "RUB",
        "Перевод с счета " + client.mainAccount);

    cout << "Перевод выполнен успешно!\n";
    cout << "Новый баланс на счете: " << account.balance_RUB << Val[0];
    cout << "Новый баланс на карте: " << card.balance_RUB << Val[0];
}

//Функция обмена
void obmen() {
    print("В какую валюту?");
    print("1:RUB—>EUD");
    print("2:RUB—>USD");
    print("3:RUB<-EUD");
    print("4:RUB<-USD");
    int nomer_valuti;
    cin >> nomer_valuti;
    int want_summa = -1;

    switch (nomer_valuti) {
        int max_posle_perevoda;
    case 1: {
        max_posle_perevoda = cards[loggedInCard].balance_RUB / Cours_EUD;
        cout << "Можете перевести в: " << max_posle_perevoda << Val[1];
        while (want_summa > max_posle_perevoda or want_summa < 0) {
            print("Сумма должна быть натуральной и не большей указанной");
            cin >> want_summa;
        }
        double rub_amount = want_summa * Cours_EUD;
        cards[loggedInCard].balance_RUB -= rub_amount;
        cards[loggedInCard].balance_EUD += want_summa;

        // Добавляем запись о операции конвертации
        addOperation(cards[loggedInCard], "convertacia", rub_amount, "RUB",
            "Конвертация в " + to_string(want_summa) + " EUD");
        break;
    }
    case 2: {
        max_posle_perevoda = cards[loggedInCard].balance_RUB / Cours_USD;
        cout << "Можете перевести в: " << max_posle_perevoda << Val[2];
        while (want_summa > max_posle_perevoda or want_summa < 0) {
            print("Сумма должна быть натуральной и не большей указанной");
            cin >> want_summa;
        }
        double rub_amount = want_summa * Cours_USD;
        cards[loggedInCard].balance_RUB -= rub_amount;
        cards[loggedInCard].balance_USD += want_summa;

        // Добавляем запись о операции конвертации
        addOperation(cards[loggedInCard], "convertacia", rub_amount, "RUB",
            "Конвертация в " + to_string(want_summa) + " USD");
        break;
    }
    case 3: {
        max_posle_perevoda = cards[loggedInCard].balance_EUD * Cours_EUD;
        cout << "Можете перевести: " << cards[loggedInCard].balance_EUD << " евро.\n";
        while (want_summa > max_posle_perevoda or want_summa < 0) {
            print("Сумма должна быть натуральной и не большей указанной");
            cin >> want_summa;
        }
        double rub_amount = want_summa * Cours_EUD;
        cards[loggedInCard].balance_EUD -= want_summa;
        cards[loggedInCard].balance_RUB += rub_amount;

        // Добавляем запись о операции конвертации
        addOperation(cards[loggedInCard], "convertacia", want_summa, "EUD",
            "Конвертация в " + to_string(rub_amount) + " RUB");
        break;
    }
    case 4: {
        max_posle_perevoda = cards[loggedInCard].balance_USD * Cours_USD;
        cout << "Можете перевести: " << max_posle_perevoda << Val[2];
        while (want_summa > max_posle_perevoda or want_summa < 0) {
            print("Сумма должна быть натуральной и не большей указанной");
            cin >> want_summa;
        }
        double rub_amount = want_summa * Cours_USD;
        cards[loggedInCard].balance_USD -= want_summa;
        cards[loggedInCard].balance_RUB += rub_amount;

        // Добавляем запись о операции конвертации
        addOperation(cards[loggedInCard], "convertacia", want_summa, "USD",
            "Конвертация в " + to_string(rub_amount) + " RUB");
        break;
    }
    }
}

// Главное меню системы
void bankSystemMenu() {
    while (true) { 
        center_print("=== БАНКОВСКАЯ СИСТЕМА ===");
        cout << "Коммисия сегодня: " << (1 - Comissiya) * 100 << "%\n"; 
        cout << "1. Стать клиентом банка (создать карту и счет)\n";
        cout << "2. Войти в систему\n"; 
        cout << "3. Выйти\n";
        cout << "Выберите действие: "; 
        cin.ignore(); // Очищаем буфер перед вводом строки                                           //ererewrer
        
        char nomer = '17';
        cin >> nomer;
        /*
        while (sizeof(nomer) > 1)
        {
            cin >> nomer;
            if (sizeof(nomer) > 1) {
                cout << "Ввести цифру!\n";
            }
        }
        */
        if (sizeof(nomer) > 1) {
            cout << "Ввести одну цифру!\n";
            break;
        }
        switch (nomer) { 
        case '1':
            generateNewKC();
            break;
        case '2':
            login();
            if (!loggedInCard.empty()) { // проверяем что авторизация прошла успешно
                while (true) { 
                    center_print("\n=== ЛИЧНЫЙ КАБИНЕТ ==="); 

                    
                    cout << "1. Просмотр баланса карты\n";
                    cout << "2. Пополнить счет\n"; 
                    cout << "3. Перевод в валюты\n"; 
                    cout << "4. Снять наличные\n"; 
                    cout << "5. Сменить PIN-код\n";
                    cout << "6. Аналитика операций\n"; 
                    cout << "7. Информация о клиенте\n";
                    cout << "8. Перевод со счета на карту\n"; 
                    cout << "9. Выйти\n"; 
                    cout << "Выберите действие: ";

                    char innerChoice; 
                    cin >> innerChoice; 

                    switch (innerChoice) { 
                    case '1': 
                        checkbalance(); 
                        break; 
                    case '2': 
                        suda(); // пополнение счета
                        break; 
                    case '3':
                        obmen(); // обмен валют
                        break; 
                    case '4': 
                        tuda(); // снятие наличных
                        break; 
                    case '5': 
                        changePIN(); // смена PIN-кода
                        break; 
                    case '6': 
                        analitika(); // аналитика
                        break; 
                    case '7':
                        clientInfo(); // информации о клиенте
                        break; 
                    case '8':
                        transferAccountToCard(); // перевод
                        break; 
                    case '9':
                        loggedInCard = "";
                        loggedInClient = "";
                        print("Выход из системы...");
                        break;
                    default: 
                        print("Неверный выбор!");
                        break; 
                    }

                    if (innerChoice == '9') break;
                }
            }
            break;
        case '3':
            cout << "До свидания!\n";
            return;
        default:
            cout << "Неверный выбор!\n";
            break;
        
        }
    }
}

int main() {
    setlocale(LC_ALL, "Russian");
    //cout << Ru_Alpha("йцу");
    srand(time(0)); // Генератор случайных чисел
    bankSystemMenu(); //Игра
    ofstream clearFile("rezervnoe_sohranenye.txt", ios::trunc);// Очистка файла
    return 0;
}