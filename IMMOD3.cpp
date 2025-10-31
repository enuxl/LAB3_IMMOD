#include <iostream>
#include <iomanip>
#include <locale>
#include <cmath>
#include <cstdlib>
#include <ctime>

using namespace std;

// === КОНСТАНТЫ ===
const double INIT_ACCOUNT = 10000.0;     // Начальный счёт
const double BASIC_PRICE = 30.0;         // Базовая цена спроса
const double OFFER_BASE_PRICE = 35.0;    // Цена мелкооптовой партии
const double MAX_DEMAND = 100.0;         // Макс. спрос
const double MEAN_D_PRICE = 100.0;       // Средняя цена спроса
const double DAILY_SPENDING = 700.0;     // Ежедневные расходы
const double RENT_RATE = 200.0;          // Аренда
const double WAGES_AND_TAXES = 500.0;    // Зарплаты + налоги (в день)
const double VAT_RATE = 0.2;             // НДС
const double OFFER_VOLUME_BASE = 40.0;   // Объём партии
const double BASIC_STORE_INIT = 360.0;   // Склад база
const double SHOP_STORE_INIT = 80.0;     // Склад магазин
const int SIMULATION_DAYS = 100;         // Дней симуляции

// === КРЕДИТ ===
const double CREDIT_LIMIT = 50000.0;     // Лимит кредита
const double CREDIT_RATE = 0.15;         // 15% годовых (в день: 0.15 / 365)
const double CREDIT_DAILY_RATE = 0.15 / 365.0;

// === СОСТОЯНИЕ ===
struct ModelState {
    double account;          // Счёт
    double basicStore;       // Склад база
    double shopStore;        // Склад магазин
    double income;           // Доход за день
    double demand;           // Спрос
    double retPrice;         // Цена продажи
    double offerVolume;      // Объём партии
    double offerPrice;       // Цена партии
    double offerAccept;      // Принять партию?
    double transferVol;      // Перевозка
    double sold;             // Продано
    double lost;             // Потеряно
    double creditTaken;      // Взятый кредит
    double creditDebt;       // Долг по кредиту
    int day;                 // Текущий день
};

// === ВЫВОД СОСТОЯНИЯ ===
void printState(const ModelState& state) {
    cout << "\n";
    cout << "========================================\n";
    cout << "           ДЕНЬ " << state.day << " из " << SIMULATION_DAYS << "\n";
    cout << "========================================\n";
    cout << fixed << setprecision(2);
    cout << "Счёт:                    " << state.account << " руб.\n";
    cout << "Склад (база):            " << state.basicStore << " ед.\n";
    cout << "Склад (магазин):         " << state.shopStore << " ед.\n";
    cout << "Спрос:                   " << state.demand << " ед.\n";
    cout << "Цена продажи:            " << state.retPrice << " руб./ед.\n";
    cout << "Доход:                   " << state.income << " руб.\n";
    cout << "Продано:                 " << state.sold << " ед.\n";
    cout << "Потеряно:                " << state.lost << " ед.\n";

    cout << "\n--- Мелкооптовая партия ---\n";
    cout << "Объём:                   " << state.offerVolume << " ед.\n";
    cout << "Цена за ед.:             " << state.offerPrice << " руб.\n";
    cout << "Общая стоимость:         " << (state.offerVolume * state.offerPrice) << " руб.\n";

    cout << "\n--- КРЕДИТ ---\n";
    cout << "Лимит:                   " << CREDIT_LIMIT << " руб.\n";
    cout << "Взято кредита:           " << state.creditTaken << " руб.\n";
    cout << "Долг по кредиту:         " << state.creditDebt << " руб.\n";
    cout << "----------------------------------------\n";
}

// === ВВОД ДАННЫХ ===
void inputStep(ModelState& state) {
    cout << "\nВВОД НА ДЕНЬ " << state.day << ":\n";
    cout << "----------------------------------------\n";

    // 1. Перевозка
    double vol;
    cout << "Перевезти с базы в магазин (0 = нет): ";
    cin >> vol;
    state.transferVol = max(0.0, min(vol, state.basicStore));

    // 2. Принять партию?
    int accept;
    cout << "Принять партию? (1 = да, 0 = нет): ";
    cin >> accept;
    state.offerAccept = (accept == 1) ? 1.0 : 0.0;

    // 3. Взять кредит?
    if (state.creditTaken < CREDIT_LIMIT) {
        double want;
        cout << "Взять кредит? (0 = нет, сумма до " << (CREDIT_LIMIT - state.creditTaken) << "): ";
        cin >> want;
        if (want > 0 && want <= (CREDIT_LIMIT - state.creditTaken)) {
            state.creditTaken += want;
            state.creditDebt += want;
            state.account += want;
            cout << "Кредит выдан: " << want << " руб.\n";
        }
    }

    // 4. Цена продажи
    cout << "Цена продажи (10-50): ";
    cin >> state.retPrice;
    state.retPrice = max(10.0, min(state.retPrice, 50.0));
}

int main() {
    setlocale(LC_ALL, "Russian");
    srand(time(0));

    cout << "=======================================\n";
    cout << "   МАГАЗИН С КРЕДИТОВАНИЕМ\n";
    cout << "=======================================\n\n";

    ModelState state = { INIT_ACCOUNT, BASIC_STORE_INIT, SHOP_STORE_INIT, 0, 0, 15.0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    for (state.day = 1; state.day <= SIMULATION_DAYS; ++state.day) {

        // Генерация партии каждые 10 дней
        if (state.day % 10 == 1) {
            state.offerVolume = OFFER_VOLUME_BASE + (rand() % 20 - 10);
            state.offerVolume = max(30.0, min(state.offerVolume, 50.0));
            state.offerPrice = OFFER_BASE_PRICE + (rand() % 10 - 5);
            state.offerPrice = max(30.0, min(state.offerPrice, 40.0));
        }

        printState(state);
        if (state.day < SIMULATION_DAYS) inputStep(state);

        // Перевозка
        if (state.transferVol > 0) {
            state.basicStore -= state.transferVol;
            state.shopStore += state.transferVol;
        }

        // Покупка партии
        if (state.offerAccept == 1.0) {
            double cost = state.offerVolume * state.offerPrice;
            if (state.account >= cost) {
                state.account -= cost;
                state.basicStore += state.offerVolume;
                cout << "Партия куплена за " << cost << " руб.\n";
            }
            else {
                cout << "Не хватает денег! Партия отклонена.\n";
                state.offerAccept = 0;
            }
        }

        // Спрос
        state.demand = MAX_DEMAND * (MEAN_D_PRICE / state.retPrice);
        state.demand = max(0.0, state.demand + (rand() % 40 - 20));

        // Продажа
        double canSell = min(state.shopStore, state.demand);
        state.sold = canSell;
        state.shopStore -= canSell;
        state.lost = state.demand - canSell;
        state.income = canSell * state.retPrice;
        state.account += state.income;

        // Расходы
        state.account -= DAILY_SPENDING + RENT_RATE + WAGES_AND_TAXES;

        // Проценты по кредиту
        if (state.creditDebt > 0) {
            double interest = state.creditDebt * CREDIT_DAILY_RATE;
            state.account -= interest;
            state.creditDebt += interest;
            cout << "Начислены проценты: " << interest << " руб.\n";
        }

        if (state.account < 0) cout << "Счёт в минусе!\n";

        if (state.day < SIMULATION_DAYS) {
            cout << "\nНажмите Enter...\n";
            cin.ignore(); cin.get();
        }
    }

    cout << "\n\n=======================================\n";
    cout << "ИТОГ: Счёт = " << state.account << " руб.\n";
    cout << "Долг по кредиту: " << state.creditDebt << " руб.\n";
    cout << "=======================================\n";

    return 0;
}
