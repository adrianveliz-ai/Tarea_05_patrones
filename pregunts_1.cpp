#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
using namespace std;

class Account {
public:
    Account(string id, double initialBalance) {
        id_ = move(id);
        balance_ = initialBalance;
    }

    const string& getId() const {
        return id_;
    }

    double getBalance() const {
        lock_guard<mutex> lock(mutex_);
        return balance_;
    }

private:
    friend class TransactionManager;

    void credit(double amount) {
        lock_guard<mutex> lock(mutex_);
        balance_ += amount;
    }

    // Verificar fondos y descontar ocurre bajo el mismo lock (atomico).
    bool debit(double amount) {
        lock_guard<mutex> lock(mutex_);
        if (balance_ < amount) return false;
        balance_ -= amount;
        return true;
    }

    string id_;
    double balance_;
    mutable mutex mutex_;
};

class Transaction {
public:
    enum class Type { Deposit, Withdraw };

    Transaction(unsigned long id, Type type, string accountId,
                double amount, bool success) {
        id_ = id;
        type_ = type;
        accountId_ = accountId;
        amount_ = amount;
        success_ = success;

    }

    unsigned long getId() const {
        return id_;
    }
    Type getType() const {
        return type_;
    }
    const string& getAccountId() const {
        return accountId_;
    }
    double getAmount() const {
        return amount_;
    }
    bool wasSuccessful() const {
        return success_;
    }

private:
    unsigned long id_;
    Type type_;
    string accountId_;
    double amount_;
    bool success_;
};


class TransactionManager {
public:
    static TransactionManager& getInstance() {
        static TransactionManager instance;
        return instance;
    }

    TransactionManager(const TransactionManager&) = delete;
    TransactionManager& operator=(const TransactionManager&) = delete;
    TransactionManager(TransactionManager&&) = delete;
    TransactionManager& operator=(TransactionManager&&) = delete;

    bool deposit(Account* account, double amount) {
        if (!account || amount <= 0) return false;
        lock_guard<mutex> lock(mutex_);   // toda la operacion es atomica
        account->credit(amount);
        history_.emplace_back(++nextId_, Transaction::Type::Deposit,
                              account->getId(), amount, true);
        return true;
    }

    bool withdraw(Account* account, double amount) {
        if (!account || amount <= 0) return false;
        lock_guard<mutex> lock(mutex_);
        bool ok = account->debit(amount);
        history_.emplace_back(++nextId_, Transaction::Type::Withdraw,
                              account->getId(), amount, ok);
        return ok;
    }

    size_t transactionCount() const {
        lock_guard<mutex> lock(mutex_);
        return history_.size();
    }

    vector<Transaction> getHistory() const {   // copia: no se expone el estado interno
        lock_guard<mutex> lock(mutex_);
        return history_;
    }

private:
    TransactionManager() = default;

    mutable mutex mutex_;
    vector<Transaction> history_;
    unsigned long nextId_ = 0;
};

int main() {
    auto& manager = TransactionManager::getInstance();
    Account account("A1", 1000);

    vector<thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&]() {
            manager.deposit(&account, 100);
            manager.withdraw(&account, 50);
        });
    }
    for (auto& t : threads) t.join();

    cout << "Balance final: " << account.getBalance() << endl;
    cout << "Direccion manager: " << &TransactionManager::getInstance() << endl;
    cout << "Direccion manager (referencia original): " << &manager << endl;
    cout << "Transacciones registradas: " << manager.transactionCount() << endl;
}