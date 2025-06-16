#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <limits>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <stack>
#include <ctime>
#include <stdexcept>
#include <list>

using namespace std;

struct Pelanggan
{
    int id;
    string nama;
    string noTelepon;
    list<int> riwayatTransaksi;
};

struct Sepatu
{
    string id, nama, merk;
    float ukuran;
    int harga, stok;
};

struct CartItem
{
    string shoeID;
    int quantity;
    double unitPrice;
};

struct Transaction
{
    string transactionID;
    string dateTime;
    string customerName;
    vector<CartItem> items;
    double total;
    Transaction *next;

    Transaction(const string &id, const string &dt,
                const string &cust, const vector<CartItem> &its, double tot)
        : transactionID(id), dateTime(dt), customerName(cust), items(its), total(tot), next(nullptr) {}
};

struct Node
{
    Sepatu data;
    Node *left;
    Node *right;
    int height;
    Node(Sepatu d) : data(d), left(nullptr), right(nullptr), height(1) {}
};

unordered_map<string, Pelanggan> tabelPelanggan;

int idPelangganBerikutnya = 1001;
Node *root = nullptr;
unordered_map<string, Sepatu> hashTable;
Transaction *head = nullptr;

int getHeight(Node *n)
{
    return n ? n->height : 0;
}

int getBalance(Node *n)
{
    return n ? getHeight(n->left) - getHeight(n->right) : 0;
}

int max(int a, int b)
{
    return (a > b) ? a : b;
}

Node *rightRotate(Node *y)
{
    Node *x = y->left;
    Node *T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = max(getHeight(y->left), getHeight(y->right)) + 1;
    x->height = max(getHeight(x->left), getHeight(x->right)) + 1;
    return x;
}

Node *leftRotate(Node *x)
{
    Node *y = x->right;
    Node *T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = max(getHeight(x->left), getHeight(x->right)) + 1;
    y->height = max(getHeight(y->left), getHeight(y->right)) + 1;
    return y;
}

Node *insert(Node *node, Sepatu data)
{
    if (!node)
        return new Node(data);
    if (data.id < node->data.id)
        node->left = insert(node->left, data);
    else if (data.id > node->data.id)
        node->right = insert(node->right, data);
    else
        return node;

    node->height = 1 + max(getHeight(node->left), getHeight(node->right));
    int balance = getBalance(node);

    if (balance > 1 && data.id < node->left->data.id)
        return rightRotate(node);
    if (balance < -1 && data.id > node->right->data.id)
        return leftRotate(node);
    if (balance > 1 && data.id > node->left->data.id)
    {
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }
    if (balance < -1 && data.id < node->right->data.id)
    {
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }

    return node;
}

Node *minValueNode(Node *node)
{
    Node *current = node;
    while (current && current->left)
        current = current->left;
    return current;
}

Node *deleteNode(Node *root, string id)
{
    if (!root)
        return root;
    if (id < root->data.id)
        root->left = deleteNode(root->left, id);
    else if (id > root->data.id)
        root->right = deleteNode(root->right, id);
    else
    {
        if (!root->left || !root->right)
        {
            Node *temp = root->left ? root->left : root->right;
            if (!temp)
            {
                temp = root;
                root = nullptr;
            }
            else
                *root = *temp;
            delete temp;
        }
        else
        {
            Node *temp = minValueNode(root->right);
            root->data = temp->data;
            root->right = deleteNode(root->right, temp->data.id);
        }
    }
    if (!root)
        return root;

    root->height = 1 + max(getHeight(root->left), getHeight(root->right));
    int balance = getBalance(root);
    if (balance > 1 && getBalance(root->left) >= 0)
        return rightRotate(root);
    if (balance > 1 && getBalance(root->left) < 0)
    {
        root->left = leftRotate(root->left);
        return rightRotate(root);
    }
    if (balance < -1 && getBalance(root->right) <= 0)
        return leftRotate(root);
    if (balance < -1 && getBalance(root->right) > 0)
    {
        root->right = rightRotate(root->right);
        return leftRotate(root);
    }
    return root;
}

void inorder(Node *root, vector<Sepatu> &result)
{
    if (!root)
        return;
    inorder(root->left, result);
    result.push_back(root->data);
    inorder(root->right, result);
}

void loadFromFile(Node *&root, unordered_map<string, Sepatu> &hashTable)
{
    ifstream file("sepatu.csv");
    if (!file.is_open())
    {
        cerr << "Gagal membuka file sepatu.csv. File tidak ditemukan.\n";
        return;
    }

    string line;
    while (getline(file, line))
    {
        try
        {
            stringstream ss(line);
            string id, nama, merk, ukuranStr, hargaStr, stokStr;
            getline(ss, id, ',');
            getline(ss, nama, ',');
            getline(ss, merk, ',');
            getline(ss, ukuranStr, ',');
            getline(ss, hargaStr, ',');
            getline(ss, stokStr, ',');

            Sepatu s;
            s.id = id;
            s.nama = nama;
            s.merk = merk;
            s.ukuran = stof(ukuranStr);
            s.harga = stoi(hargaStr);
            s.stok = stoi(stokStr);

            root = insert(root, s);
            hashTable[s.id] = s;
        }
        catch (exception &e)
        {
            cerr << "Format baris salah: " << e.what() << endl;
        }
    }
    file.close();
}

void saveToFile(Node *root)
{
    try
    {
        ofstream file("sepatu.csv");
        if (!file.is_open())
            throw runtime_error("Gagal membuka file untuk menyimpan.");

        vector<Sepatu> list;
        inorder(root, list);
        for (auto &s : list)
        {
            file << s.id << "," << s.nama << "," << s.merk << "," << s.ukuran << "," << s.harga << "," << s.stok << endl;
        }
        file.close();
    }
    catch (exception &e)
    {
        cerr << "Error saat menyimpan data: " << e.what() << endl;
    }
}

void tampilkanDetailPelanggan(const Pelanggan &p)
{
    cout << "------------------------------------\n";
    cout << "ID Pelanggan    : " << p.id << "\n";
    cout << "Nama            : " << p.nama << "\n";
    cout << "No. Telepon     : " << p.noTelepon << "\n";
    cout << "Riwayat Transaksi (ID): ";
    if (p.riwayatTransaksi.empty())
    {
        cout << "Belum ada transaksi.\n";
    }
    else
    {
        for (int idTrans : p.riwayatTransaksi)
        {
            cout << idTrans << " ";
        }
        cout << "\n";
    }
    cout << "------------------------------------\n";
}

void tambahkanTransaksiKePelanggan(const string &noHp, int idTransaksi)
{
    auto it = tabelPelanggan.find(noHp);
    if (it != tabelPelanggan.end())
    {
        it->second.riwayatTransaksi.push_back(idTransaksi);
    }
}

void loadPelangganFromFile()
{
    ifstream file("pelanggan.csv");
    if (!file.is_open())
    {
        cerr << "Gagal membuka file pelanggan.csv. File tidak ditemukan.\n";
        return;
    }

    string line;
    // Skip header
    getline(file, line);

    while (getline(file, line))
    {
        try
        {
            stringstream ss(line);
            string idStr, nama, noTelepon;
            getline(ss, idStr, ',');
            getline(ss, nama, ',');
            getline(ss, noTelepon, ',');

            // Extract numeric ID from CUST-XXX format
            string numId = idStr.substr(5); // Skip "CUST-"
            int id = stoi(numId);

            Pelanggan p;
            p.id = id;
            p.nama = nama;
            p.noTelepon = noTelepon;

            tabelPelanggan[noTelepon] = p;
            idPelangganBerikutnya = max(idPelangganBerikutnya, id + 1);
        }
        catch (exception &e)
        {
            cerr << "Format baris salah: " << e.what() << endl;
        }
    }
    file.close();
}

void savePelangganToFile()
{
    try
    {
        ofstream file("pelanggan.csv");
        if (!file.is_open())
            throw runtime_error("Gagal membuka file untuk menyimpan.");

        // Write header
        file << "ID,Nama,Telepon\n";

        // Write data
        for (const auto &pair : tabelPelanggan)
        {
            const Pelanggan &p = pair.second;
            file << "CUST-" << setfill('0') << setw(3) << p.id << ","
                 << p.nama << ","
                 << p.noTelepon << "\n";
        }
        file.close();
    }
    catch (exception &e)
    {
        cerr << "Error saat menyimpan data pelanggan: " << e.what() << endl;
    }
}

void tambahPelanggan()
{
    string nama, noTelepon;

    cout << "\n--- Menambahkan Pelanggan Baru ---\n";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "Masukkan Nomor Telepon (akan jadi ID utama): ";
    getline(cin, noTelepon);

    if (tabelPelanggan.count(noTelepon))
    {
        cout << "\n>> Gagal! Nomor telepon '" << noTelepon << "' sudah terdaftar. <<\n";
        return;
    }

    cout << "Masukkan Nama Pelanggan: ";
    getline(cin, nama);

    Pelanggan pelangganBaru;
    pelangganBaru.id = idPelangganBerikutnya;
    pelangganBaru.nama = nama;
    pelangganBaru.noTelepon = noTelepon;

    tabelPelanggan[noTelepon] = pelangganBaru;
    savePelangganToFile(); // Save after adding new customer

    cout << "\n>> Sukses! Pelanggan '" << nama << "' berhasil ditambahkan. <<\n";
    cout << "ID Internal: " << idPelangganBerikutnya << ", No HP (Key): " << noTelepon << "\n";

    idPelangganBerikutnya++;
}

void lihatDaftarPelanggan()
{
    cout << "\n--- Daftar Semua Pelanggan ---\n";
    if (tabelPelanggan.empty())
    {
        cout << "Belum ada data pelanggan yang tersimpan.\n";
        return;
    }

    vector<Pelanggan> daftar;
    for (const auto &pair : tabelPelanggan)
    {
        daftar.push_back(pair.second);
    }

    sort(daftar.begin(), daftar.end(), [](const Pelanggan &a, const Pelanggan &b)
         { return a.nama < b.nama; });

    cout << "\nDiurutkan berdasarkan nama (A-Z):\n";
    for (const auto &p : daftar)
    {
        tampilkanDetailPelanggan(p);
    }
}

void editDataPelanggan()
{
    cout << "\n--- Mengedit Data Pelanggan ---\n";
    if (tabelPelanggan.empty())
    {
        cout << "Belum ada data pelanggan untuk diedit.\n";
        return;
    }

    string noHpCari;
    cout << "Masukkan No. Telepon pelanggan yang akan diedit: ";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, noHpCari);

    auto it = tabelPelanggan.find(noHpCari);

    if (it != tabelPelanggan.end())
    {
        Pelanggan &pelanggan = it->second;
        cout << "\nData Pelanggan Saat Ini:\n";
        tampilkanDetailPelanggan(pelanggan);

        string namaBaru, noHpBaru;
        cout << "Masukkan Nama Baru (kosongkan jika tidak ingin diubah): ";
        getline(cin, namaBaru);
        cout << "Masukkan No. Telepon Baru (kosongkan jika tidak ingin diubah): ";
        getline(cin, noHpBaru);

        bool adaPerubahan = false;

        if (!namaBaru.empty())
        {
            pelanggan.nama = namaBaru;
            adaPerubahan = true;
            cout << "Nama berhasil diubah.\n";
        }

        if (!noHpBaru.empty() && noHpBaru != noHpCari)
        {
            if (tabelPelanggan.count(noHpBaru))
            {
                cout << "Gagal mengubah No. Telepon, nomor '" << noHpBaru << "' sudah digunakan oleh pelanggan lain.\n";
            }
            else
            {
                Pelanggan dataPindahan = pelanggan;
                dataPindahan.noTelepon = noHpBaru;

                tabelPelanggan.erase(it);
                tabelPelanggan[noHpBaru] = dataPindahan;
                adaPerubahan = true;
                cout << "No. Telepon berhasil diubah ke " << noHpBaru << ".\n";
            }
        }

        if (adaPerubahan)
        {
            savePelangganToFile(); // Save after making changes
        }
        cout << "\n>> Update selesai. <<\n";
    }
    else
    {
        cout << "\n>> Pelanggan dengan No. Telepon '" << noHpCari << "' tidak ditemukan. <<\n";
    }
}

void hapusDataPelanggan()
{
    cout << "\n--- Menghapus Data Pelanggan ---\n";
    if (tabelPelanggan.empty())
    {
        cout << "Belum ada data pelanggan untuk dihapus.\n";
        return;
    }

    string noHpCari;
    cout << "Masukkan No. Telepon pelanggan yang akan dihapus: ";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, noHpCari);

    auto it = tabelPelanggan.find(noHpCari);

    if (it != tabelPelanggan.end())
    {
        cout << "\nAnda akan menghapus data berikut:\n";
        tampilkanDetailPelanggan(it->second);

        char konfirmasi;
        cout << "Apakah Anda yakin ingin menghapus? (y/n): ";
        cin >> konfirmasi;

        if (konfirmasi == 'y' || konfirmasi == 'Y')
        {
            tabelPelanggan.erase(it);
            savePelangganToFile(); // Save after deleting customer
            cout << "\n>> Pelanggan berhasil dihapus. <<\n";
        }
        else
        {
            cout << "\n>> Penghapusan dibatalkan. <<\n";
        }
    }
    else
    {
        cout << "\n>> Pelanggan dengan No. Telepon '" << noHpCari << "' tidak ditemukan. <<\n";
    }
}

void cariPelanggan()
{
    cout << "\n--- Mencari Pelanggan ---\n";
    if (tabelPelanggan.empty())
    {
        cout << "Belum ada data pelanggan untuk dicari.\n";
        return;
    }

    int pilihan;
    cout << "Cari berdasarkan:\n";
    cout << "1. Nomor Telepon (Pencarian Cepat O(1))\n";
    cout << "2. Nama Pelanggan (Pencarian Lambat O(N))\n";
    cout << "Pilih opsi: ";
    cin >> pilihan;

    if (pilihan == 1)
    {
        string noHpCari;
        cout << "Masukkan No. Telepon: ";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        getline(cin, noHpCari);

        auto it = tabelPelanggan.find(noHpCari);
        if (it != tabelPelanggan.end())
        {
            cout << "\nPelanggan ditemukan:\n";
            tampilkanDetailPelanggan(it->second);
        }
        else
        {
            cout << "\n>> Pelanggan dengan No. Telepon '" << noHpCari << "' tidak ditemukan. <<\n";
        }
    }
    else if (pilihan == 2)
    {
        string nama;
        cout << "Masukkan Nama Pelanggan: ";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        getline(cin, nama);

        bool ditemukan = false;
        cout << "\nHasil Pencarian:\n";

        for (const auto &pair : tabelPelanggan)
        {
            if (pair.second.nama.find(nama) != string::npos)
            {
                tampilkanDetailPelanggan(pair.second);
                ditemukan = true;
            }
        }

        if (!ditemukan)
        {
            cout << ">> Pelanggan dengan nama yang mengandung '" << nama << "' tidak ditemukan. <<\n";
        }
    }
    else
    {
        cout << "Pilihan tidak valid.\n";
    }
}

void tampilkanTabel(const vector<Sepatu> &data)
{
    cout << setfill('=') << setw(84) << "=" << endl;
    cout << setfill(' ') << left
         << "| " << setw(8) << "ID"
         << "| " << setw(20) << "Nama"
         << "| " << setw(15) << "Merk"
         << "| " << setw(8) << "Ukuran"
         << "| " << setw(12) << "Harga"
         << "| " << setw(8) << "Stok"
         << "|" << endl;
    cout << setfill('=') << setw(84) << "=" << endl;
    cout << setfill(' ');

    for (const auto &s : data)
    {
        cout << "| " << setw(8) << s.id
             << "| " << setw(20) << s.nama
             << "| " << setw(15) << s.merk
             << "| " << setw(8) << s.ukuran
             << "| " << setw(12) << s.harga
             << "| " << setw(8) << s.stok
             << "|" << endl;
    }

    cout << setfill('=') << setw(84) << "=" << endl;
}

void menuManajemenSepatu()
{
    int pilihan;
    do
    {
        cout << "\n=== MENU MANAJEMEN DATA SEPATU ===\n";
        cout << "1. Tambah Data Sepatu\n";
        cout << "2. Tampilkan Semua Data\n";
        cout << "3. Edit Data Sepatu\n";
        cout << "4. Hapus Data Sepatu\n";
        cout << "5. Cari Berdasarkan ID\n";
        cout << "0. Kembali ke Menu Utama\n";
        cout << "Pilihan: ";
        cin >> pilihan;

        if (cin.fail())
        {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Input tidak valid. Harap masukkan angka.\n";
            continue;
        }

        if (pilihan == 1)
        {
            Sepatu s;
            cout << "ID: ";
            cin >> s.id;
            cout << "Nama: ";
            cin.ignore();
            getline(cin, s.nama);
            cout << "Merk: ";
            getline(cin, s.merk);
            try
            {
                cout << "Ukuran: ";
                cin >> s.ukuran;
                if (cin.fail())
                    throw runtime_error("Ukuran harus angka.");
                cout << "Harga: ";
                cin >> s.harga;
                if (cin.fail())
                    throw runtime_error("Harga harus angka.");
                cout << "Stok: ";
                cin >> s.stok;
                if (cin.fail())
                    throw runtime_error("Stok harus angka.");
                cin.ignore();
            }
            catch (exception &e)
            {
                cin.clear();
                cin.ignore(10000, '\n');
                cout << "Input error: " << e.what() << endl;
                continue;
            }
            root = insert(root, s);
            hashTable[s.id] = s;
            cout << "Data berhasil ditambahkan.\n";
        }
        else if (pilihan == 2)
        {
            vector<Sepatu> list;
            inorder(root, list);
            sort(list.begin(), list.end(), [](Sepatu a, Sepatu b)
                 { return a.nama < b.nama; });
            tampilkanTabel(list);
        }
        else if (pilihan == 3)
        {
            string id;
            cout << "Masukkan ID sepatu yang ingin diedit: ";
            cin >> id;
            auto it = hashTable.find(id);
            if (it == hashTable.end())
            {
                cout << "Data tidak ditemukan.\n";
            }
            else
            {
                root = deleteNode(root, id);
                Sepatu &s = it->second;
                cout << "Data lama: " << s.nama << ", " << s.merk << ", Ukuran: " << s.ukuran << ", Harga: " << s.harga << ", Stok: " << s.stok << endl;
                cout << "Nama Baru: ";
                cin.ignore();
                getline(cin, s.nama);
                cout << "Merk Baru: ";
                getline(cin, s.merk);
                try
                {
                    cout << "Ukuran Baru: ";
                    cin >> s.ukuran;
                    cout << "Harga Baru: ";
                    cin >> s.harga;
                    cout << "Stok Baru: ";
                    cin >> s.stok;
                    if (cin.fail())
                        throw runtime_error("Data harus berupa angka.");
                }
                catch (exception &e)
                {
                    cin.clear();
                    cin.ignore(10000, '\n');
                    cout << "Error: " << e.what() << endl;
                    continue;
                }
                root = insert(root, s);
                cout << "Data berhasil diperbarui.\n";
            }
        }
        else if (pilihan == 4)
        {
            string id;
            cout << "Masukkan ID sepatu yang ingin dihapus: ";
            cin >> id;
            if (hashTable.count(id))
            {
                root = deleteNode(root, id);
                hashTable.erase(id);
                cout << "Data berhasil dihapus.\n";
            }
            else
            {
                cout << "Data tidak ditemukan.\n";
            }
        }
        else if (pilihan == 5)
        {
            string id;
            cout << "Masukkan ID yang dicari: ";
            cin >> id;
            if (hashTable.count(id))
            {
                Sepatu s = hashTable[id];
                cout << "Data ditemukan:\n";
                tampilkanTabel({s});
            }
            else
            {
                cout << "Data tidak ditemukan.\n";
            }
        }
        else if (pilihan != 0)
        {
            cout << "Pilihan tidak valid.\n";
        }

    } while (pilihan != 0);
}

string generateTransactionID()
{
    time_t t = time(nullptr);
    ostringstream ss;
    ss << "TRX" << t;
    return ss.str();
}

string getCurrentDateTime()
{
    time_t t = time(nullptr);
    char buf[100];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t));
    return buf;
}

bool getProductInfo(const string &shoeID, double &price, int &stock)
{
    ifstream file("sepatu.csv");
    if (!file.is_open())
        return false;
    string line;
    while (getline(file, line))
    {
        stringstream ss(line);
        string id, name, brand;
        int size, st;
        double pr;
        char comma;
        getline(ss, id, ',');
        getline(ss, name, ',');
        getline(ss, brand, ',');
        ss >> size >> comma >> pr >> comma >> st;
        if (id == shoeID)
        {
            price = pr;
            stock = st;
            return true;
        }
    }
    return false;
}

void updateProductStock(const string &shoeID, int boughtQty)
{
    ifstream inFile("sepatu.csv");
    ofstream outFile("sepatu_tmp.csv");
    string line;
    while (getline(inFile, line))
    {
        stringstream ss(line);
        string id, name, brand;
        int size, stock;
        double price;
        char comma;
        getline(ss, id, ',');
        getline(ss, name, ',');
        getline(ss, brand, ',');
        ss >> size >> comma >> price >> comma >> stock;
        if (id == shoeID)
            stock -= boughtQty;
        outFile << id << "," << name << "," << brand << "," << size
                << "," << price << "," << stock << "\n";
    }
    inFile.close();
    outFile.close();
    remove("sepatu.csv");
    rename("sepatu_tmp.csv", "sepatu.csv");
}

void saveTransactionCSV(const Transaction *trx)
{
    ofstream file("transaksi.csv", ios::app);
    for (const auto &item : trx->items)
    {
        double subtotal = item.quantity * item.unitPrice;
        file << trx->transactionID << ","
             << trx->dateTime << ","
             << trx->customerName << ","
             << item.shoeID << ","
             << item.quantity << ","
             << item.unitPrice << ","
             << subtotal << "\n";
    }
}

void appendTransaction(Transaction *newTrx)
{
    if (!head)
        head = newTrx;
    else
    {
        Transaction *cur = head;
        while (cur->next)
            cur = cur->next;
        cur->next = newTrx;
    }
}

void processTransaction()
{
    stack<CartItem> keranjangStack;
    vector<CartItem> keranjangItems;
    string namaPelanggan;

    cout << "Masukkan nama pelanggan: ";
    getline(cin, namaPelanggan);

    while (true)
    {
        cout << "\n-- Menu Keranjang --\n";
        cout << "1. Tambah Item" << endl;
        cout << "2. Batalkan Item Terakhir" << endl;
        cout << "3. Selesai Input Item" << endl;
        cout << "Pilih opsi: ";
        int pilihan;
        cin >> pilihan;
        cin.ignore();

        if (pilihan == 1)
        {
            string id;
            int qty;
            double harga;
            int stok;
            cout << "Masukkan ID sepatu: ";
            getline(cin, id);
            cout << "Masukkan jumlah: ";
            cin >> qty;
            cin.ignore();
            if (!getProductInfo(id, harga, stok))
            {
                cout << "ID sepatu tidak ditemukan.\n";
                continue;
            }
            if (qty <= 0)
            {
                cout << "Jumlah harus lebih dari 0.\n";
                continue;
            }
            if (qty > stok)
            {
                cout << "Stok tidak cukup. Tersisa: " << stok << "\n";
                continue;
            }
            CartItem item{id, qty, harga};
            keranjangStack.push(item);
            keranjangItems.push_back(item);
            cout << "Item berhasil ditambahkan.\n";
        }
        else if (pilihan == 2)
        {
            if (keranjangStack.empty())
            {
                cout << "Tidak ada item untuk dibatalkan.\n";
            }
            else
            {
                CartItem terakhir = keranjangStack.top();
                keranjangStack.pop();
                keranjangItems.pop_back();
                cout << "Item " << terakhir.shoeID << " dibatalkan.\n";
            }
        }
        else if (pilihan == 3)
        {
            double total = 0;
            cout << "\n-- Ringkasan Transaksi --\n";
            for (const auto &it : keranjangItems)
            {
                double subtotal = it.quantity * it.unitPrice;
                cout << "ID: " << it.shoeID
                     << " | Jumlah: " << it.quantity
                     << " | Harga/unit: " << it.unitPrice
                     << " | Subtotal: " << subtotal << endl;
                total += subtotal;
            }
            cout << "Total keseluruhan: " << total << endl;
            cout << "Konfirmasi transaksi? (y/n): ";
            char konfirmasi;
            cin >> konfirmasi;
            cin.ignore();
            if (konfirmasi == 'y' || konfirmasi == 'Y')
            {
                string trxID = generateTransactionID();
                string dt = getCurrentDateTime();
                Transaction *trx = new Transaction(trxID, dt, namaPelanggan, keranjangItems, total);
                appendTransaction(trx);
                saveTransactionCSV(trx);
                for (const auto &it : keranjangItems)
                    updateProductStock(it.shoeID, it.quantity);
                cout << "Transaksi berhasil. ID: " << trxID << endl;
            }
            else
            {
                cout << "Transaksi dibatalkan.\n";
            }
            break;
        }
        else
        {
            cout << "Opsi tidak valid. Silakan coba lagi.\n";
        }
    }
}

void menuProsesPenjualan()
{
    while (true)
    {
        cout << "\n== Proses Penjualan Sepatu ==" << endl;
        cout << "1. Buat Transaksi Baru" << endl;
        cout << "0. Kembali ke Menu Utama" << endl;
        cout << "Pilih opsi: ";
        int pilihan;
        cin >> pilihan;
        cin.ignore();
        if (pilihan == 1)
            processTransaction();
        else if (pilihan == 0)
            break;
        else
            cout << "Opsi tidak valid.\n";
    }
}

void tampilkanTransaksi(Transaction *t)
{
    cout << "\nID Transaksi     : " << t->transactionID << endl;
    cout << "Nama Pelanggan   : " << t->customerName << endl;
    cout << "Tanggal          : " << t->dateTime << endl;
    cout << "Daftar Produk    :\n";
    for (auto &item : t->items)
    {
        cout << "- " << item.shoeID << " x" << item.quantity
             << " @ " << item.unitPrice << endl;
    }
    cout << "Total Harga      : " << t->total << endl;
}

void lihatSemua(bool terbaru)
{
    vector<Transaction *> transaksiList;
    Transaction *temp = head;
    while (temp)
    {
        transaksiList.push_back(temp);
        temp = temp->next;
    }

    sort(transaksiList.begin(), transaksiList.end(), [=](Transaction *a, Transaction *b)
         { return terbaru ? a->dateTime > b->dateTime : a->dateTime < b->dateTime; });

    if (transaksiList.empty())
    {
        cout << "Belum ada transaksi.\n";
        return;
    }

    for (auto t : transaksiList)
        tampilkanTransaksi(t);
}

Transaction *cariTransaksi(const string &key, const string &tipe)
{
    Transaction *temp = head;
    while (temp)
    {
        if ((tipe == "id" && temp->transactionID == key) ||
            (tipe == "nama" && temp->customerName == key) ||
            (tipe == "tanggal" && temp->dateTime == key))
        {
            return temp;
        }
        temp = temp->next;
    }
    return nullptr;
}

void hapusTransaksi(const string &id)
{
    Transaction *temp = head, *prev = nullptr;
    while (temp && temp->transactionID != id)
    {
        prev = temp;
        temp = temp->next;
    }

    if (!temp)
    {
        cout << "Transaksi tidak ditemukan.\n";
        return;
    }

    tampilkanTransaksi(temp);
    char konfirmasi;
    cout << "Yakin ingin menghapus transaksi ini? (y/n): ";
    cin >> konfirmasi;
    if (konfirmasi == 'y' || konfirmasi == 'Y')
    {
        if (!prev)
            head = temp->next;
        else
            prev->next = temp->next;
        delete temp;
        cout << "Transaksi berhasil dihapus.\n";
    }
}

void filterTanggal(const string &awal, const string &akhir)
{
    Transaction *temp = head;
    bool ditemukan = false;
    while (temp)
    {
        if (temp->dateTime >= awal && temp->dateTime <= akhir)
        {
            tampilkanTransaksi(temp);
            ditemukan = true;
        }
        temp = temp->next;
    }
    if (!ditemukan)
        cout << "Tidak ada transaksi dalam rentang tanggal tersebut.\n";
}

void menuRiwayatTransaksi()
{
    int pilihan;
    do
    {
        cout << "\n===== MENU RIWAYAT TRANSAKSI =====\n";
        cout << "1. Lihat Semua Riwayat\n";
        cout << "2. Cari Riwayat Transaksi\n";
        cout << "3. Hapus Riwayat Transaksi\n";
        cout << "4. Filter Berdasarkan Tanggal\n";
        cout << "0. Kembali ke Menu Utama\n";
        cout << "Pilih: ";
        cin >> pilihan;
        cin.ignore();

        if (pilihan == 1)
        {
            int urut;
            cout << "Urutkan berdasarkan:\n1. Terlama\n2. Terbaru\nPilihan: ";
            cin >> urut;
            lihatSemua(urut == 2);
        }
        else if (pilihan == 2)
        {
            int tipe;
            cout << "Cari berdasarkan:\n1. ID\n2. Nama Pelanggan\n3. Tanggal\nPilihan: ";
            cin >> tipe;
            cin.ignore();
            string key;
            cout << "Masukkan kata kunci: ";
            getline(cin, key);
            string jenis = (tipe == 1) ? "id" : (tipe == 2) ? "nama"
                                                            : "tanggal";
            Transaction *hasil = cariTransaksi(key, jenis);
            if (hasil)
                tampilkanTransaksi(hasil);
            else
                cout << "Transaksi tidak ditemukan.\n";
        }
        else if (pilihan == 3)
        {
            string id;
            cout << "Masukkan ID transaksi yang ingin dihapus: ";
            cin.ignore();
            getline(cin, id);
            hapusTransaksi(id);
        }
        else if (pilihan == 4)
        {
            string awal, akhir;
            cout << "Tanggal awal (YYYY-MM-DD): ";
            cin >> awal;
            cout << "Tanggal akhir (YYYY-MM-DD): ";
            cin >> akhir;
            filterTanggal(awal, akhir);
        }
        else if (pilihan != 0)
        {
            cout << "Pilihan tidak valid.\n";
        }
    } while (pilihan != 0);
}

void menuManajemenPelanggan()
{
    int pilihan;
    do
    {
        cout << "\n====================================\n";
        cout << "    SUB-MENU MANAJEMEN PELANGGAN\n";
        cout << "====================================\n";
        cout << "1. Tambah Pelanggan\n";
        cout << "2. Lihat Daftar Pelanggan\n";
        cout << "3. Edit Data Pelanggan\n";
        cout << "4. Hapus Data Pelanggan\n";
        cout << "5. Cari Pelanggan\n";
        cout << "6. Kembali ke Menu Utama\n";
        cout << "------------------------------------\n";
        cout << "Pilih opsi: ";
        cin >> pilihan;

        if (cin.fail())
        {
            cout << "Input tidak valid. Harap masukkan angka.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        switch (pilihan)
        {
        case 1:
            tambahPelanggan();
            break;
        case 2:
            lihatDaftarPelanggan();
            break;
        case 3:
            editDataPelanggan();
            break;
        case 4:
            hapusDataPelanggan();
            break;
        case 5:
            cariPelanggan();
            break;
        case 6:
            cout << "Kembali ke Menu Utama...\n";
            break;
        default:
            cout << "Pilihan tidak valid. Silakan coba lagi.\n";
        }
    } while (pilihan != 6);
}

int main()
{
    int pilihan;

    cout << "Startup Program & Data Loading...\n";
    loadPelangganFromFile(); // Load customer data from CSV
    loadFromFile(root, hashTable);
    cout << "Data berhasil dimuat.\n";

    do
    {
        cout << "\n****************************************\n";
        cout << "* SISTEM MANAJEMEN TOKO SEPATU   *\n";
        cout << "****************************************\n";
        cout << "1. Manajemen Sepatu\n";
        cout << "2. Manajemen Pelanggan\n";
        cout << "3. Proses Penjualan\n";
        cout << "4. Riwayat Transaksi\n";
        cout << "5. Keluar Program\n";
        cout << "----------------------------------------\n";
        cout << "Pilih opsi: ";
        cin >> pilihan;

        if (cin.fail())
        {
            cout << "Input tidak valid. Harap masukkan angka.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            pilihan = 0;
        }

        switch (pilihan)
        {
        case 1:
            menuManajemenSepatu();
            break;
        case 2:
            menuManajemenPelanggan();
            break;
        case 3:
            menuProsesPenjualan();
            break;
        case 4:
            menuRiwayatTransaksi();
            break;
        case 5:
            cout << "\nMenyimpan data sebelum keluar...\n";
            saveToFile(root);
            savePelangganToFile(); // Save customer data before exit
            cout << "Terima kasih telah menggunakan sistem!\n";
            break;
        default:
            if (pilihan != 0)
            {
                cout << "Pilihan tidak valid. Silakan coba lagi.\n";
            }
        }

    } while (pilihan != 5);

    return 0;
}