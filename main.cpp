#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <queue>
#include <ctime>
#include <cstdlib>
#include <string>
#include <chrono>

using namespace std;

const int INF = 1000000000;

// Struktury danych grafu
struct Krawedz {
    int poczatek, koniec, waga;
};

struct Sasiad {
    int numer, waga;
};

struct GrafLista { // Lista sasiedztwa - kazdy wierzcholek ma swoich sasiadow
    int V, E; // V - wierzcholki, E - krawedzie
    vector<vector<Sasiad>> lista;
};

struct GrafMacierz { // Graf zapisany jak macierz sasiedztwa VxV
    int V, E;
    vector<vector<int>> macierz;
};

// Sprawdzamy czy bedzie cykl
struct ZbioryRozlaczne {
    vector<int> rodzic, wysokosc;

    ZbioryRozlaczne(int n) {
        rodzic.resize(n);
        wysokosc.resize(n, 0);
        for (int i = 0; i < n; i++) rodzic[i] = i;
    }

    int znajdz(int x) {
        if (rodzic[x] != x) rodzic[x] = znajdz(rodzic[x]);
        return rodzic[x];
    }

    void polacz(int a, int b) {
        int pa = znajdz(a), pb = znajdz(b);
        if (pa == pb) return;
        if (wysokosc[pa] < wysokosc[pb]) rodzic[pa] = pb;
        else if (wysokosc[pa] > wysokosc[pb]) rodzic[pb] = pa;
        else { rodzic[pb] = pa; wysokosc[pa]++; }
    }
};

// Globalne zmienne grafów
GrafLista grafLista;
GrafMacierz grafMacierz;
int startowy = 0;

void utworzGraf(int V) {
    grafLista.V = V; grafLista.E = 0; grafLista.lista.clear(); grafLista.lista.resize(V);
    grafMacierz.V = V; grafMacierz.E = 0; grafMacierz.macierz.clear();
    grafMacierz.macierz.resize(V, vector<int>(V, -1));
}

void dodajKrawedzNieskierowana(int a, int b, int waga) {
    if (a < 0 || b < 0 || a >= grafLista.V || b >= grafLista.V || a == b || grafMacierz.macierz[a][b] != -1) return;
    grafLista.lista[a].push_back({b, waga});
    grafLista.lista[b].push_back({a, waga});
    grafLista.E++;
    grafMacierz.macierz[a][b] = waga;
    grafMacierz.macierz[b][a] = waga;
    grafMacierz.E++;
}

void dodajKrawedzSkierowana(int a, int b, int waga) {
    if (a < 0 || b < 0 || a >= grafLista.V || b >= grafLista.V || a == b || grafMacierz.macierz[a][b] != -1) return;
    grafLista.lista[a].push_back({b, waga});
    grafLista.E++;
    grafMacierz.macierz[a][b] = waga;
    grafMacierz.E++;
}

void pokazGraf() {
    if (grafLista.V == 0) { cout << "Brak grafu w pamieci.\n"; return; }

    cout << "\n--- Lista sasiedztwa ---\n";
    for (int i = 0; i < grafLista.V; i++) {
        cout << "V[" << i << "]: ";
        for (auto &sasiedzi : grafLista.lista[i]) {
            cout << "->" << sasiedzi.numer << "(" << sasiedzi.waga << ") ";
        }
        cout << "\n";
    }

    cout << "\n--- Macierz sasiedztwa ---\n";
    for (int i = 0; i < grafMacierz.V; i++) {
        for (int j = 0; j < grafMacierz.V; j++) {
            if (grafMacierz.macierz[i][j] == -1) cout << "X\t";
            else cout << grafMacierz.macierz[i][j] << "\t";
        }
        cout << "\n";
    }
}

bool porownajKrawedzie(Krawedz a, Krawedz b) { return a.waga < b.waga; }

void pokazWynikMST(vector<Krawedz> mst, string nazwa) {
    int suma = 0;
    cout << "\nWynik algorytmu " << nazwa << ":\nKrawedzie MST:\n";
    for (auto &k : mst) {
        cout << k.poczatek << " - " << k.koniec << " : " << k.waga << "\n";
        suma += k.waga;
    }
    cout << "Suma wag: " << suma << "\n";
}

void pokazSciezke(int v, const vector<int>& rodzic) {
    if (v == -1) return;
    if (rodzic[v] == -1) { cout << v; return; }
    pokazSciezke(rodzic[v], rodzic);
    cout << " -> " << v;
}

void pokazWynikSciezki(const vector<int>& koszt, const vector<int>& rodzic, string nazwa) {
    cout << "\nWynik algorytmu " << nazwa << ":\nWierzcholek startowy: " << startowy << "\n";
    for (int i = 0; i < (int)koszt.size(); i++) {
        cout << "Do wierzcholka " << i << ": ";
        if (koszt[i] >= INF) cout << "brak drogi\n";
        else { cout << "koszt = " << koszt[i] << ", droga: "; pokazSciezke(i, rodzic); cout << "\n"; }
    }
}

void wczytajZPliku(string nazwaPliku, bool czySkierowany) {
    ifstream plik(nazwaPliku.c_str());
    if (!plik.is_open()) { cout << "Nie udalo sie otworzyc pliku.\n"; return; }
    int L_krawedzi, L_wierzcholkow;
    if (czySkierowany) plik >> L_krawedzi >> L_wierzcholkow >> startowy;
    else plik >> L_krawedzi >> L_wierzcholkow;

    utworzGraf(L_wierzcholkow);
    if (startowy < 0 || startowy >= L_wierzcholkow) startowy = 0;

    for (int i = 0; i < L_krawedzi; i++) {
        int a, b, waga;
        plik >> a >> b >> waga;
        if (czySkierowany) dodajKrawedzSkierowana(a, b, waga);
        else dodajKrawedzNieskierowana(a, b, waga);
    }
    plik.close();
    cout << "Wczytano graf z pliku.\n";
    pokazGraf();
}

void losujGrafMST(int V, double gestosc, bool wyswietl) {
    if (V <= 0) return;
    utworzGraf(V);
    int maksKrawedzi = V * (V - 1) / 2;
    int ileKrawedzi = (int)(maksKrawedzi * gestosc / 100.0);
    if (ileKrawedzi < V - 1) ileKrawedzi = V - 1;

    vector<int> odwiedzone, nieodwiedzone;
    odwiedzone.push_back(0);
    for (int i = 1; i < V; i++) nieodwiedzone.push_back(i);

    while (!nieodwiedzone.empty()) {
        int idxA = rand() % odwiedzone.size(), idxB = rand() % nieodwiedzone.size();
        int a = odwiedzone[idxA], b = nieodwiedzone[idxB];
        dodajKrawedzNieskierowana(a, b, rand() % 100 + 1);
        odwiedzone.push_back(b);
        nieodwiedzone.erase(nieodwiedzone.begin() + idxB);
    }
    while (grafLista.E < ileKrawedzi) {
        int a = rand() % V, b = rand() % V;
        if (a != b) dodajKrawedzNieskierowana(a, b, rand() % 100 + 1);
    }
    if (wyswietl) pokazGraf();
}

void losujGrafSkierowany(int V, double gestosc, bool wyswietl) {
    if (V <= 0) return;
    utworzGraf(V); startowy = 0;
    int maksKrawedzi = V * (V - 1);
    int ileKrawedzi = (int)(maksKrawedzi * gestosc / 100.0);
    if (V > 1 && ileKrawedzi < V) ileKrawedzi = V;

    if (V > 1) {
        for (int i = 0; i < V - 1; i++) dodajKrawedzSkierowana(i, i + 1, rand() % 100 + 1);
        dodajKrawedzSkierowana(V - 1, 0, rand() % 100 + 1);
    }
    while (grafLista.E < ileKrawedzi) {
        int a = rand() % V, b = rand() % V;
        if (a != b) dodajKrawedzSkierowana(a, b, rand() % 100 + 1);
    }
    if (wyswietl) pokazGraf();
}

// Implementacja algorytmow
vector<Krawedz> kruskalListaWynik() {
    vector<Krawedz> wszystkie, mst;
    for (int i = 0; i < grafLista.V; i++) {
        for (auto &k : grafLista.lista[i]) {
            if (i < k.numer) wszystkie.push_back({i, k.numer, k.waga});
        }
    }
    sort(wszystkie.begin(), wszystkie.end(), porownajKrawedzie);
    ZbioryRozlaczne zbiory(grafLista.V);
    for (auto &k : wszystkie) {
        if (zbiory.znajdz(k.poczatek) != zbiory.znajdz(k.koniec)) {
            mst.push_back(k);
            zbiory.polacz(k.poczatek, k.koniec);
        }
        if ((int)mst.size() == grafLista.V - 1) break;
    }
    return mst;
}

vector<Krawedz> kruskalMacierzWynik() {
    vector<Krawedz> wszystkie, mst;
    for (int i = 0; i < grafMacierz.V; i++) {
        for (int j = i + 1; j < grafMacierz.V; j++) {
            if (grafMacierz.macierz[i][j] != -1) wszystkie.push_back({i, j, grafMacierz.macierz[i][j]});
        }
    }
    sort(wszystkie.begin(), wszystkie.end(), porownajKrawedzie);
    ZbioryRozlaczne zbiory(grafMacierz.V);
    for (auto &k : wszystkie) {
        if (zbiory.znajdz(k.poczatek) != zbiory.znajdz(k.koniec)) {
            mst.push_back(k);
            zbiory.polacz(k.poczatek, k.koniec);
        }
        if ((int)mst.size() == grafMacierz.V - 1) break;
    }
    return mst;
}

vector<Krawedz> primListaWynik() {
    vector<int> koszt(grafLista.V, INF), rodzic(grafLista.V, -1);
    vector<bool> gotowy(grafLista.V, false);
    vector<Krawedz> mst;
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> kolejka;

    koszt[0] = 0; kolejka.push({0, 0});
    while (!kolejka.empty()) {
        int u = kolejka.top().second; kolejka.pop();
        if (gotowy[u]) continue;
        gotowy[u] = true;
        if (rodzic[u] != -1) mst.push_back({rodzic[u], u, koszt[u]});
        for (auto &sasiedzi : grafLista.lista[u]) {
            int v = sasiedzi.numer, w = sasiedzi.waga;
            if (!gotowy[v] && w < koszt[v]) {
                koszt[v] = w; rodzic[v] = u; kolejka.push({koszt[v], v});
            }
        }
    }
    return mst;
}

vector<Krawedz> primMacierzWynik() {
    vector<int> koszt(grafMacierz.V, INF), rodzic(grafMacierz.V, -1);
    vector<bool> gotowy(grafMacierz.V, false);
    vector<Krawedz> mst;
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> kolejka;

    koszt[0] = 0; kolejka.push({0, 0});
    while (!kolejka.empty()) {
        int u = kolejka.top().second; kolejka.pop();
        if (gotowy[u]) continue;
        gotowy[u] = true;
        if (rodzic[u] != -1) mst.push_back({rodzic[u], u, koszt[u]});
        for (int v = 0; v < grafMacierz.V; v++) {
            int w = grafMacierz.macierz[u][v];
            if (w != -1 && !gotowy[v] && w < koszt[v]) {
                koszt[v] = w; rodzic[v] = u; kolejka.push({koszt[v], v});
            }
        }
    }
    return mst;
}

void dijkstraListaWynik(vector<int> &koszt, vector<int> &rodzic) {
    koszt.assign(grafLista.V, INF); rodzic.assign(grafLista.V, -1);
    vector<bool> gotowy(grafLista.V, false);
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> kolejka;
    koszt[startowy] = 0; kolejka.push({0, startowy});

    while (!kolejka.empty()) {
        int u = kolejka.top().second; kolejka.pop();
        if (gotowy[u]) continue;
        gotowy[u] = true;
        for (auto &sasiedzi : grafLista.lista[u]) {
            int v = sasiedzi.numer, w = sasiedzi.waga;
            if (koszt[u] + w < koszt[v]) {
                koszt[v] = koszt[u] + w; rodzic[v] = u; kolejka.push({koszt[v], v});
            }
        }
    }
}

void dijkstraMacierzWynik(vector<int> &koszt, vector<int> &rodzic) {
    koszt.assign(grafMacierz.V, INF); rodzic.assign(grafMacierz.V, -1);
    vector<bool> gotowy(grafMacierz.V, false);
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> kolejka;
    koszt[startowy] = 0; kolejka.push({0, startowy});

    while (!kolejka.empty()) {
        int u = kolejka.top().second; kolejka.pop();
        if (gotowy[u]) continue;
        gotowy[u] = true;
        for (int v = 0; v < grafMacierz.V; v++) {
            int w = grafMacierz.macierz[u][v];
            if (w != -1 && koszt[u] + w < koszt[v]) {
                koszt[v] = koszt[u] + w; rodzic[v] = u; kolejka.push({koszt[v], v});
            }
        }
    }
}

void bellmanFordListaWynik(vector<int> &koszt, vector<int> &rodzic) {
    koszt.assign(grafLista.V, INF); rodzic.assign(grafLista.V, -1); koszt[startowy] = 0;
    for (int k = 0; k < grafLista.V - 1; k++) {
        bool zmiana = false;
        for (int u = 0; u < grafLista.V; u++) {
            if (koszt[u] >= INF) continue;
            for (auto &sasiedzi : grafLista.lista[u]) {
                int v = sasiedzi.numer, w = sasiedzi.waga;
                if (koszt[u] + w < koszt[v]) { koszt[v] = koszt[u] + w; rodzic[v] = u; zmiana = true; }
            }
        }
        if (!zmiana) break;
    }
}

void bellmanFordMacierzWynik(vector<int> &koszt, vector<int> &rodzic) {
    koszt.assign(grafMacierz.V, INF); rodzic.assign(grafMacierz.V, -1); koszt[startowy] = 0;
    for (int k = 0; k < grafMacierz.V - 1; k++) {
        bool zmiana = false;
        for (int u = 0; u < grafMacierz.V; u++) {
            if (koszt[u] >= INF) continue;
            for (int v = 0; v < grafMacierz.V; v++) {
                int w = grafMacierz.macierz[u][v];
                if (w != -1 && koszt[u] + w < koszt[v]) { koszt[v] = koszt[u] + w; rodzic[v] = u; zmiana = true; }
            }
        }
        if (!zmiana) break;
    }
}

// Funkcje pomiarowe
void pomiaryMST() {
    int rozmiary[7] = {20, 40, 60, 80, 100, 150, 200};
    int gestosci[3] = {20, 60, 99};
    int powtorzenia = 50;
    ofstream plik("wyniki_mst.csv");
    plik << "problem;V;gestosc;algorytm;reprezentacja;sredni_czas_us\n";
    cout << "\nTrwaja pomiary MST...\n";

    for (int i = 0; i < 7; i++) {
        for (int g = 0; g < 3; g++) {
            long long tPL = 0, tPM = 0, tKL = 0, tKM = 0;
            for (int p = 0; p < powtorzenia; p++) {
                losujGrafMST(rozmiary[i], gestosci[g], false);

                auto s = chrono::high_resolution_clock::now(); primListaWynik();
                tPL += chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - s).count();

                s = chrono::high_resolution_clock::now(); primMacierzWynik();
                tPM += chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - s).count();

                s = chrono::high_resolution_clock::now(); kruskalListaWynik();
                tKL += chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - s).count();

                s = chrono::high_resolution_clock::now(); kruskalMacierzWynik();
                tKM += chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - s).count();
            }
            plik << "MST;" << rozmiary[i] << ";" << gestosci[g] << ";Prim;lista;" << tPL / powtorzenia << "\n";
            plik << "MST;" << rozmiary[i] << ";" << gestosci[g] << ";Prim;macierz;" << tPM / powtorzenia << "\n";
            plik << "MST;" << rozmiary[i] << ";" << gestosci[g] << ";Kruskal;lista;" << tKL / powtorzenia << "\n";
            plik << "MST;" << rozmiary[i] << ";" << gestosci[g] << ";Kruskal;macierz;" << tKM / powtorzenia << "\n";
            cout << "Zrobiono MST: V=" << rozmiary[i] << ", g=" << gestosci[g] << "%\n";
        }
    }
    plik.close();
}

void pomiarySciezki() {
    int rozmiary[7] = {20, 40, 60, 80, 100, 150, 200};
    int gestosci[3] = {20, 60, 99};
    int powtorzenia = 50;
    ofstream plik("wyniki_sciezki.csv");
    plik << "problem;V;gestosc;algorytm;reprezentacja;sredni_czas_us\n";
    cout << "\nTrwaja pomiary sciezek...\n";

    for (int i = 0; i < 7; i++) {
        for (int g = 0; g < 3; g++) {
            long long tDL = 0, tDM = 0, tBL = 0, tBM = 0;
            vector<int> kosz, rodz;
            for (int p = 0; p < powtorzenia; p++) {
                losujGrafSkierowany(rozmiary[i], gestosci[g], false);

                auto s = chrono::high_resolution_clock::now(); dijkstraListaWynik(kosz, rodz);
                tDL += chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - s).count();

                s = chrono::high_resolution_clock::now(); dijkstraMacierzWynik(kosz, rodz);
                tDM += chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - s).count();

                s = chrono::high_resolution_clock::now(); bellmanFordListaWynik(kosz, rodz);
                tBL += chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - s).count();

                s = chrono::high_resolution_clock::now(); bellmanFordMacierzWynik(kosz, rodz);
                tBM += chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - s).count();
            }
            plik << "Sciezki;" << rozmiary[i] << ";" << gestosci[g] << ";Dijkstra;lista;" << tDL / powtorzenia << "\n";
            plik << "Sciezki;" << rozmiary[i] << ";" << gestosci[g] << ";Dijkstra;macierz;" << tDM / powtorzenia << "\n";
            plik << "Sciezki;" << rozmiary[i] << ";" << gestosci[g] << ";Bellman;lista;" << tBL / powtorzenia << "\n";
            plik << "Sciezki;" << rozmiary[i] << ";" << gestosci[g] << ";Bellman;macierz;" << tBM / powtorzenia << "\n";
            cout << "Zrobiono sciezki: V=" << rozmiary[i] << ", g=" << gestosci[g] << "%\n";
        }
    }
    plik.close();
}

void menuMST() {
    int wybor;
    while (true) {
        cout << "\n=== MENU MST ===\n1. Wczytaj z pliku\n2. Wygeneruj losowo\n3. Wyswietl graf\n4. Prim (macierz i lista)\n5. Kruskal (macierz i lista)\n6. Pomiary czasu\n0. Powrot\nWybor: ";
        cin >> wybor;
        if (wybor == 1) { string n; cout << "Nazwa pliku: "; cin >> n; wczytajZPliku(n, false); }
        else if (wybor == 2) { int v; double g; cout << "V: "; cin >> v; cout << "G%%: "; cin >> g; losujGrafMST(v, g, true); }
        else if (wybor == 3) pokazGraf();
        else if (wybor == 4 && grafLista.V > 0) { pokazWynikMST(primMacierzWynik(), "Prima - Macierz"); pokazWynikMST(primListaWynik(), "Prima - Lista"); }
        else if (wybor == 5 && grafLista.V > 0) { pokazWynikMST(kruskalMacierzWynik(), "Kruskala - Macierz"); pokazWynikMST(kruskalListaWynik(), "Kruskala - Lista"); }
        else if (wybor == 6) pomiaryMST();
        else if (wybor == 0) return;
        else cout << "Blad / Brak grafu.\n";
    }
}

void menuSciezki() {
    int wybor;
    while (true) {
        cout << "\n=== MENU SCIEZKI ===\n1. Wczytaj z pliku\n2. Wygeneruj losowo\n3. Wyswietl graf\n4. Dijkstra (macierz i lista)\n5. Bellman-Ford (macierz i lista)\n6. Pomiary czasu\n0. Powrot\nWybor: ";
        cin >> wybor;
        vector<int> kosz, rodz;
        if (wybor == 1) { string n; cout << "Nazwa pliku: "; cin >> n; wczytajZPliku(n, true); }
        else if (wybor == 2) { int v; double g; cout << "V: "; cin >> v; cout << "G%%: "; cin >> g; losujGrafSkierowany(v, g, true); }
        else if (wybor == 3) pokazGraf();
        else if (wybor == 4 && grafLista.V > 0) { dijkstraMacierzWynik(kosz, rodz); pokazWynikSciezki(kosz, rodz, "Dijkstra Macierz"); dijkstraListaWynik(kosz, rodz); pokazWynikSciezki(kosz, rodz, "Dijkstra Lista"); }
        else if (wybor == 5 && grafLista.V > 0) { bellmanFordMacierzWynik(kosz, rodz); pokazWynikSciezki(kosz, rodz, "Bellman-Ford Macierz"); bellmanFordListaWynik(kosz, rodz); pokazWynikSciezki(kosz, rodz, "Bellman-Ford Lista"); }
        else if (wybor == 6) pomiarySciezki();
        else if (wybor == 0) return;
        else cout << "Blad / Brak grafu.\n";
    }
}

int main() {
    srand(time(NULL));
    int wybor;
    while (true) {
        cout << "\nWybierz problem:\n1. MST (Zadanie 2 - Etap 1)\n2. Najkrotsza sciezka (Zadanie 2 - Etap 2)\n0. Koniec\nWybor: ";
        cin >> wybor;
        if (wybor == 1) menuMST();
        else if (wybor == 2) menuSciezki();
        else if (wybor == 0) break;
    }
    return 0;
}