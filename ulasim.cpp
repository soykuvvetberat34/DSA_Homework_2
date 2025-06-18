#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <climits>


template <typename T>
class VecArr {
private:
    T* data;
    int cap;
    int len;

    void resize(int yeniCap) {
        T* newData = (T*) std::malloc(sizeof(T) * yeniCap);
        for (int i = 0; i < len; ++i) {
            newData[i] = data[i];
        }
        std::free(data);
        data = newData;
        cap = yeniCap;
    }

public:
    VecArr(): data(nullptr), cap(0), len(0) {}
    ~VecArr() { if (data) std::free(data); }

    void pushLast(const T& val) {
        if (len + 1 > cap) {
            int newCap = (cap == 0 ? 4 : cap * 2);
            resize(newCap);
        }
        data[len++] = val;
    }

    void popLast() {
        if (len > 0) --len;
    }

    T& operator[](int idx) {
        if (idx < 0 || idx >= len) {
            std::fprintf(stderr, "VecArr HATA: idx=%d, len=%d\n", idx, len);
            std::exit(1);
        }
        return data[idx];
    }

    int size() const { return len; }
};

template <typename T>
struct BagliNode {
    T val;
    BagliNode<T>* next;
    BagliNode(const T& v): val(v), next(nullptr) {}
};

template <typename T>
class BagliList {
private:
    BagliNode<T>* bas;
    BagliNode<T>* son;

public:
    BagliList(): bas(nullptr), son(nullptr) {}
    ~BagliList() {
        BagliNode<T>* cur = bas;
        while (cur) {
            BagliNode<T>* nx = cur->next;
            delete cur;
            cur = nx;
        }
    }

    void pushEnd(const T& v) {
        BagliNode<T>* node = new BagliNode<T>(v);
        if (!bas) {
            bas = son = node;
        } else {
            son->next = node;
            son = node;
        }
    }

    BagliNode<T>* head() const { return bas; }
};

struct HeapN {
    int vertex;
    int dist;
};

class MinHeap {
private:
    VecArr<HeapN> arr;
    int* pos;

    void siftUp(int i) {
        while (i > 0) {
            int p = (i - 1) / 2;
            if (arr[p].dist > arr[i].dist) {
                HeapN tmp = arr[p];
                arr[p] = arr[i];
                arr[i] = tmp;
                pos[arr[p].vertex] = p;
                pos[arr[i].vertex] = i;
                i = p;
            } else break;
        }
    }

    void siftDown(int i) {
        int n = arr.size();
        int l = 2*i + 1;
        int r = 2*i + 2;
        int smallest = i;
        if (l < n && arr[l].dist < arr[smallest].dist) smallest = l;
        if (r < n && arr[r].dist < arr[smallest].dist) smallest = r;
        if (smallest != i) {
            HeapN tmp = arr[smallest];
            arr[smallest] = arr[i];
            arr[i] = tmp;
            pos[arr[smallest].vertex] = smallest;
            pos[arr[i].vertex] = i;
            siftDown(smallest);
        }
    }

public:
    MinHeap(int vertexCount) {
        arr = VecArr<HeapN>();
        pos = (int*) std::malloc(sizeof(int) * vertexCount);
    }
    ~MinHeap() { std::free(pos); }

    bool isEmpty() const { return arr.size() == 0; }

    void insert(int v, int d) {
        HeapN hn; hn.vertex = v; hn.dist = d;
        arr.pushLast(hn);
        int i = arr.size() - 1;
        pos[v] = i;
        siftUp(i);
    }

    HeapN extractMin() {
        HeapN root = arr[0];
        HeapN last = arr[arr.size() - 1];
        arr[0] = last;
        pos[last.vertex] = 0;
        arr.popLast();
        siftDown(0);
        return root;
    }

    void decreaseKey(int v, int newDist) {
        int i = pos[v];
        if (i < 0 || i >= arr.size()) return;
        arr[i].dist = newDist;
        siftUp(i);
    }
};



struct Edge {
    int dest;
    int weight;
    BagliList<int> lineList;
    Edge(int d=0,int w=0): dest(d), weight(w) {}
};

class Graph {
private:
    int V;
    VecArr< BagliList<Edge*> > adj;

public:
    Graph(int v): V(v) {
        adj = VecArr< BagliList<Edge*> >();
        for (int i = 0; i < V; ++i) {
            BagliList<Edge*> l;
            adj.pushLast(l);
        }
    }
    ~Graph() {
        for (int u = 0; u < V; ++u) {
            BagliNode<Edge*>* node = adj[u].head();
            while (node) {
                delete node->val;
                node = node->next;
            }
        }
    }

    void addEdge(int u,int v,int w) {
        if (u < 0 || u >= V || v < 0 || v >= V) return;
        Edge* e1 = new Edge(v,w);
        adj[u].pushEnd(e1);
        Edge* e2 = new Edge(u,w);
        adj[v].pushEnd(e2);
    }

    void addLineToEdge(int u,int v,int lineNo) {
        if (u < 0 || u >= V || v < 0 || v >= V) return;
        BagliNode<Edge*>* node = adj[u].head();
        while (node) {
            if (node->val->dest == v) {
                node->val->lineList.pushEnd(lineNo);
                break;
            }
            node = node->next;
        }
        node = adj[v].head();
        while (node) {
            if (node->val->dest == u) {
                node->val->lineList.pushEnd(lineNo);
                break;
            }
            node = node->next;
        }
    }

    void dijkstra(int src,int* dist,int* parent) {
        if (src < 0 || src >= V) return;
        for (int i = 0; i < V; ++i) {
            dist[i] = INT_MAX;
            parent[i] = -1;
        }
        dist[src] = 0;
        MinHeap mh(V);
        for (int i = 0; i < V; ++i) {
            mh.insert(i, dist[i]);
        }
        while (!mh.isEmpty()) {
            HeapN hn = mh.extractMin();
            int u = hn.vertex;
            BagliNode<Edge*>* en = adj[u].head();
            while (en) {
                int v = en->val->dest;
                int w = en->val->weight;
                if (dist[u] != INT_MAX && dist[u] + w < dist[v]) {
                    dist[v] = dist[u] + w;
                    parent[v] = u;
                    mh.decreaseKey(v, dist[v]);
                }
                en = en->next;
            }
        }
    }

    BagliList<Edge*>& getAdj(int u) {
        static BagliList<Edge*> empty;
        if (u < 0 || u >= V) return empty;
        return adj[u];
    }
};

struct BSTNode {
    int durakID;
    BagliList<int> hatList;
    BSTNode* left;
    BSTNode* right;
    BSTNode(int id): durakID(id), left(nullptr), right(nullptr) {}
};

class DurakHatBST {
private:
    BSTNode* root;

    BSTNode* insertNode(BSTNode* node,int durakID,int hatNo) {
        if (!node) {
            node = new BSTNode(durakID);
            node->hatList.pushEnd(hatNo);
            return node;
        }
        if (durakID < node->durakID) {
            node->left = insertNode(node->left,durakID,hatNo);
        } else if (durakID > node->durakID) {
            node->right = insertNode(node->right,durakID,hatNo);
        } else {
            node->hatList.pushEnd(hatNo);
        }
        return node;
    }

    BSTNode* searchNode(BSTNode* node,int durakID) const {
        if (!node) return nullptr;
        if (durakID < node->durakID) return searchNode(node->left,durakID);
        else if (durakID > node->durakID) return searchNode(node->right,durakID);
        else return node;
    }

    void deleteTree(BSTNode* node) {
        if (!node) return;
        deleteTree(node->left);
        deleteTree(node->right);
        delete node;
    }

public:
    DurakHatBST(): root(nullptr) {}
    ~DurakHatBST() { deleteTree(root); }

    void insert(int durakID,int hatNo) { root = insertNode(root,durakID,hatNo); }

    BSTNode* search(int durakID) const { return searchNode(root,durakID); }
};

// Sabitler ve Global Degiskenler 
static const int MAX_DURAK = 114;
static const int MAX_HAT = 100;

static char durakNameArr[MAX_DURAK + 1][256];
static int hatNoArr[MAX_HAT];
static char hatNameArr[MAX_HAT][256];
static int hatSayisi = 0;
static VecArr< VecArr<int> > hatDuraklari;
static VecArr< VecArr<char*> > hatDurakAdlari;
static int hatUsageArr[MAX_HAT];

static DurakHatBST durakHatAgaci;

class Graph;
Graph globalGraph(MAX_DURAK);

// Yardimci: Durak ismine gore id bul
int findDurakIDbyName(const char* name) {
    for (int i = 1; i <= MAX_DURAK; ++i) {
        if (std::strcmp(durakNameArr[i], name) == 0) {
            return i;
        }
    }
    return -1;
}

// Fonksiyonlar 
void grafOlustur(const char* durakListF, const char* hatlarF, const char* mesafeF, Graph& g);
void durakAra(int durakID);
void yolcuPlanla(const char* yolcuFile, int arananYolcuID);
void tumHatlariListele();
void hatDuraklariGoster(int hatNo);

// GRAFI OLUSTUR 
void grafOlustur(const char* durakListF, const char* hatlarF, const char* mesafeF, Graph& g) {
    // durak_listesi.txt oku
    FILE* fDurak = std::fopen(durakListF, "r");
    if (!fDurak) {
        std::perror("durak_listesi.txt acilamadi");
        std::exit(1);
    }
    char line[16000];
    std::fgets(line, sizeof(line), fDurak);

    int durakSatirCount = 0;
    while (std::fgets(line, sizeof(line), fDurak)) {
        bool only_ws = true;
        for (int i = 0; line[i] != '\0'; ++i)
            if (line[i] != ' ' && line[i] != '\t' && line[i] != '\n' && line[i] != '\r') { only_ws = false; break; }
        if (only_ws) continue;

        char* token = std::strtok(line, ",");
        if (!token) continue;
        int id = std::atoi(token);
        if (id < 1 || id > MAX_DURAK) continue;

        token = std::strtok(nullptr, ",");
        if (!token) continue;
        while (*token == ' ' || *token == '\t' || *token == '\n' || *token == '\r') ++token;
        int len = std::strlen(token);
        while (len > 0 && (token[len - 1] == ' ' || token[len - 1] == '\t' || token[len - 1] == '\n' || token[len - 1] == '\r')) token[--len] = '\0';
        std::strncpy(durakNameArr[id], token, 256);
        durakNameArr[id][255] = '\0';
        durakSatirCount++;
    }
    std::fclose(fDurak);

    // hatlar.txt oku
    FILE* fHat = std::fopen(hatlarF, "r");
    if (!fHat) { std::perror("hatlar.txt acilamadi"); std::exit(1); }
    std::fgets(line, sizeof(line), fHat);

    int hatSatirCount = 0; hatSayisi = 0;
    while (std::fgets(line, sizeof(line), fHat)) {
        bool only_ws = true;
        for (int i = 0; line[i] != '\0'; ++i)
            if (line[i] != ' ' && line[i] != '\t' && line[i] != '\n' && line[i] != '\r') { only_ws = false; break; }
        if (only_ws) continue;

        char* token = std::strtok(line, ",");
        if (!token) continue;
        int hatNo = std::atoi(token);
        if (hatNo <= 0) continue;

        if (hatSayisi >= MAX_HAT) break;
        hatNoArr[hatSayisi] = hatNo;

        token = std::strtok(nullptr, ",");
        if (!token) { hatNameArr[hatSayisi][0] = '\0'; }
        else {
            while (*token == ' ' || *token == '\t' || *token == '\n' || *token == '\r') ++token;
            int len2 = std::strlen(token);
            while (len2 > 0 && (token[len2 - 1] == ' ' || token[len2 - 1] == '\t' || token[len2 - 1] == '\n' || token[len2 - 1] == '\r')) token[--len2] = '\0';
            std::strncpy(hatNameArr[hatSayisi], token, 256); hatNameArr[hatSayisi][255] = '\0';
        }

        VecArr<int> dlist;
        VecArr<char*> isimList;
        while ((token = std::strtok(nullptr, ",")) != nullptr) {
            while (*token == ' ' || *token == '\t' || *token == '\n' || *token == '\r') ++token;
            int len2 = std::strlen(token);
            while (len2 > 0 && (token[len2 - 1] == ' ' || token[len2 - 1] == '\t' || token[len2 - 1] == '\n' || token[len2 - 1] == '\r')) token[--len2] = '\0';
            if (len2 == 0) continue;

            char* copyName = (char*) std::malloc(len2 + 1); std::strcpy(copyName, token); isimList.pushLast(copyName);
            int durakID = findDurakIDbyName(token);
            if (durakID != -1) { dlist.pushLast(durakID); durakHatAgaci.insert(durakID, hatNo); }
        }
        hatDuraklari.pushLast(dlist); hatDurakAdlari.pushLast(isimList);
        hatSayisi++; hatSatirCount++;
    }
    std::fclose(fHat);

    // durak_mesafeleri.txt oku 
    FILE* fMesafe = std::fopen(mesafeF, "r");
    if (!fMesafe) { std::perror("durak_mesafeleri.txt acilamadi"); std::exit(1); }
    int row = 0;
    while (std::fgets(line, sizeof(line), fMesafe)) {
        bool only_ws = true;
        for (int i = 0; line[i] != '\0'; ++i)
            if (line[i] != ' ' && line[i] != '\t' && line[i] != '\n' && line[i] != '\r') { only_ws = false; break; }
        if (only_ws) { row++; continue; }
        int col = 0;
        char* token = std::strtok(line, ",");
        while (token) {
            while (*token == ' ' || *token == '\t' || *token == '\n' || *token == '\r') ++token;
            int len2 = std::strlen(token);
            while (len2 > 0 && (token[len2 - 1] == ' ' || token[len2 - 1] == '\t' || token[len2 - 1] == '\n' || token[len2 - 1] == '\r')) token[--len2] = '\0';
            if (*token && std::strcmp(token, "-") != 0) {
                int w = std::atoi(token); int u = row; int v = col;
                if (u >= 0 && u < MAX_DURAK && v >= 0 && v < MAX_DURAK) g.addEdge(u, v, w);
            }
            token = std::strtok(nullptr, ","); col++;
        }
        row++;
    }
    std::fclose(fMesafe);

    // Hat bazli durak dizisinden graf kenarina hatNo ekle
    for (int h = 0; h < hatSayisi; ++h) {
        int hatNo = hatNoArr[h];
        VecArr<int>& dlist = hatDuraklari[h];
        for (int i = 0; i + 1 < dlist.size(); ++i) {
            int u = dlist[i] - 1; int v = dlist[i + 1] - 1;
            g.addLineToEdge(u, v, hatNo);
        }
    }
}

// Durak Ara 
void durakAra(int durakID) {
    if (durakID < 1 || durakID > MAX_DURAK) {
        printf("HATA: Geçersiz durakID=%d\n", durakID); return;
    }
    BSTNode* node = durakHatAgaci.search(durakID);
    if (!node) {
        printf("Durak ID %d bulunamadi.\n", durakID); return;
    }
    printf("Durak: %s (%d)\n", durakNameArr[durakID], durakID);
    printf("  Bulundugu Hatlar:");
    BagliNode<int>* ln = node->hatList.head(); int hatSayCount = 0;
    while (ln) { printf(" %d", ln->val); hatSayCount++; ln = ln->next; }
    if (hatSayCount == 0) { printf(" (hat bulunamadi)"); }
    printf("\n");
}

//  Tüm Hatları Listele 
void tumHatlariListele() {
    printf("\n--- Tüm Hatlar ---\n");
    for (int i = 0; i < hatSayisi; ++i) {
        printf("Hat %d  (%s)\n", hatNoArr[i], hatNameArr[i]);
    }
    printf(">>> Toplam hat sayisi: %d\n", hatSayisi);
}

// Bir Hattın Duraklarını Göster 
void hatDuraklariGoster(int hatNo) {
    if (hatNo <= 0) { printf("Gecersiz hat numarasi.\n"); return; }
    int idx = -1;
    for (int i = 0; i < hatSayisi; ++i) if (hatNoArr[i] == hatNo) { idx = i; break; }
    if (idx < 0) { printf("HATA: Hat %d bulunamadi.\n", hatNo); return; }

    VecArr<char*>& isimList = hatDurakAdlari[idx];
    if (isimList.size() == 0) {
        printf("HatNo %d (%s) icin durak yok.\n", hatNoArr[idx], hatNameArr[idx]);
        return;
    }
    printf("\nHatNo %d (%s) Duraklar (%d):\n", hatNoArr[idx], hatNameArr[idx], isimList.size());
    for (int i = 0; i < isimList.size(); ++i) {
        if (isimList[i] == nullptr) { printf("  [GECERSİZ durak ismi]\n"); continue; }
        const char* name = isimList[i];
        int durakID = findDurakIDbyName(name);
        if (durakID < 1 || durakID > MAX_DURAK) { printf("  %s [Gecersiz Durak]\n", name); }
        else { printf("  %s (%d)\n", name, durakID); }
    }
    printf(">>> HatNo %d icin toplam %d durak bulundu.\n", hatNo, isimList.size());
}

// === Yolcu Planla Fonksiyonu ===
void yolcuPlanla(const char* yolcuFile, int arananYolcuID) {
    FILE* fp = std::fopen(yolcuFile, "r");
    if (!fp) { std::perror("yolcu_listesi.txt acilamadi"); return; }
    char line[16000];
    std::fgets(line, sizeof(line), fp);

    bool bulundu = false;
    while (std::fgets(line, sizeof(line), fp)) {
        char* token = std::strtok(line, ",");
        if (!token) continue;
        int yolcuID = std::atoi(token);
        if (yolcuID != arananYolcuID) continue;
        bulundu = true;

        token = std::strtok(nullptr, ",");
        if (!token) continue;
        char adSoyad[256];
        while (*token == ' ' || *token == '\t' || *token == '\n' || *token == '\r') ++token;
        int len = std::strlen(token);
        while (len > 0 && (token[len - 1] == ' ' || token[len - 1] == '\t' || token[len - 1] == '\n' || token[len - 1] == '\r')) token[--len] = '\0';
        std::strncpy(adSoyad, token, 256); adSoyad[255] = '\0';

        token = std::strtok(nullptr, ","); if (!token) continue;
        int basID = std::atoi(token);

        token = std::strtok(nullptr, ","); if (!token) continue;
        int bitID = std::atoi(token);

        const char* bas_adi = (basID >= 1 && basID <= MAX_DURAK) ? durakNameArr[basID] : "Bilinmiyor";
        const char* bit_adi = (bitID >= 1 && bitID <= MAX_DURAK) ? durakNameArr[bitID] : "Bilinmiyor";

        printf("\nYolcu: %s\nBaslangic: %s (Id: %d)\nBitis: %s (Id: %d)\n", adSoyad, bas_adi, basID, bit_adi, bitID);

        if (basID < 1 || basID > MAX_DURAK || bitID < 1 || bitID > MAX_DURAK) {
            printf("  Gecersiz durak ID: basID=%d, bitID=%d\n", basID, bitID); break;
        }

        int* dist = (int*) std::malloc(sizeof(int) * MAX_DURAK);
        int* parent = (int*) std::malloc(sizeof(int) * MAX_DURAK);

        globalGraph.dijkstra(basID-1, dist, parent);

        if (dist[bitID-1] == INT_MAX) {
            printf("  %d'den %d'ye yol bulunamadi.\n", basID, bitID);
            free(dist); free(parent); break;
        }

        int path[MAX_DURAK], pathLen = 0;
        int crawl = bitID-1; path[pathLen++] = crawl;
        while (parent[crawl] != -1) { crawl = parent[crawl]; path[pathLen++] = crawl; }
        for (int i = 0; i < pathLen/2; i++) { int temp = path[i]; path[i] = path[pathLen-1-i]; path[pathLen-1-i] = temp; }

        printf("\nRota :\n");
        int prevLine = -1;
        for (int i = 0; i < pathLen-1; i++) {
            int u = path[i];
            int v = path[i+1];
            BagliNode<Edge*>* en = globalGraph.getAdj(u).head();
            int foundLine = -1;
            while (en) {
                if (en->val->dest == v) {
                    BagliNode<int>* hn = en->val->lineList.head();
                    if (hn) {
                        if (prevLine != -1) {
                            BagliNode<int>* hn2 = en->val->lineList.head();
                            while (hn2) {
                                if (hn2->val == prevLine) { foundLine = prevLine; break; }
                                hn2 = hn2->next;
                            }
                        }
                        if (foundLine == -1) foundLine = hn->val;
                    }
                    break;
                }
                en = en->next;
            }
            if (foundLine == -1 && prevLine != -1) foundLine = prevLine;
            if (foundLine == -1) {
                for (int j = i+1; j < pathLen-1; j++) {
                    int uu = path[j]; int vv = path[j+1];
                    BagliNode<Edge*>* enn = globalGraph.getAdj(uu).head();
                    while (enn) {
                        if (enn->val->dest == vv) {
                            BagliNode<int>* hnn = enn->val->lineList.head();
                            if (hnn) { foundLine = hnn->val; break; }
                        }
                        enn = enn->next;
                    }
                    if (foundLine != -1) break;
                }
            }
            printf("%d. %s (%d) [Hat: ", i+1, durakNameArr[u+1], u+1);
            if (foundLine != -1) printf("%d", foundLine); else printf("Yok");
            printf("]");
            if (foundLine != prevLine && prevLine != -1) { printf(" <<< Aktarma!"); }
            printf("\n");
            prevLine = foundLine;
        }
        printf("%d. %s (%d) [Hat: ", pathLen, durakNameArr[path[pathLen-1]+1], path[pathLen-1]+1);
        if (prevLine != -1) printf("%d", prevLine); else printf("Yok");
        printf("]\n");

        free(dist); free(parent); break;
    }
    if (!bulundu) printf("Yolcu bulunamadi\n");
    fclose(fp);
}


// ANA MENU 
int main() {

    grafOlustur("durak_listesi.txt", "hatlar.txt", "durak_mesafeleri.txt", globalGraph);

    while (true) {
        printf("\n=== Ulasim Menu ===\n");
        printf("1. Tum Hatlari Listele\n");
        printf("2. Hatta Ait Duraklari Goruntule\n");
        printf("3. Durak Ara\n");
        printf("4. Yolculuk Plani Hazirla\n");
        printf("5. cikis\n");
        printf("Seciminiz (1-5): ");

        int secim;
        if (scanf("%d", &secim) != 1) { scanf("%*s"); continue; }

        if (secim == 1) { tumHatlariListele(); }
        else if (secim == 2) {
            printf("HatNo giriniz: ");
            int hatNo; if (scanf("%d", &hatNo) != 1) { int c; while ((c = getchar()) != '\n' && c != EOF); continue; }
            hatDuraklariGoster(hatNo);
        }
        else if (secim == 3) {
            printf("Durak ID giriniz: ");
            int durakID; scanf("%d", &durakID);
            if (durakID < 1 || durakID > MAX_DURAK) { printf("Gecersiz durak ID.\n"); }
            else { durakAra(durakID); }
        }
        else if (secim == 4) {
            int yolcuID;
            printf("Yolcu ID giriniz: ");
            scanf("%d", &yolcuID);
            yolcuPlanla("yolcu_listesi.txt", yolcuID);
        }
        else if (secim == 5) { printf("Cikis yapiliyor...\n"); break; }
        else { printf("Gecersiz secim. Tekrar deneyin.\n"); }
    }
    return 0;
}
