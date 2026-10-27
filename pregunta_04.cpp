#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <stack>
#include <stdexcept>
#include <vector>
using namespace std;

// ----------------------------- Node (privado al grafo) -----------------------------
class Node {
public:
    explicit Node(int id) : id_(id) {}
    int getId() const { return id_; }
private:
    friend class Graph;
    int id_;
    vector<int> neighbors_;
};

// ----------------------------- Graph -----------------------------
class Graph {
public:
    void addNode(int id) {
        if (!nodes_.count(id)) nodes_.emplace(id, make_unique<Node>(id));
    }

    void addEdge(int from, int to) {            // arista dirigida from -> to
        addNode(from);
        addNode(to);
        auto& adj = nodes_[from]->neighbors_;
        if (find(adj.begin(), adj.end(), to) == adj.end()) adj.push_back(to);
    }

    bool contains(int id) const { return nodes_.count(id) > 0; }

    // Devuelve una COPIA: la estructura interna nunca se expone.
    vector<int> neighbors(int id) const {
        auto it = nodes_.find(id);
        return it == nodes_.end() ? vector<int>{} : it->second->neighbors_;
    }

private:
    map<int, unique_ptr<Node>> nodes_;
};

// ----------------------------- Interfaz del iterador -----------------------------
class GraphIterator {
public:
    virtual ~GraphIterator() = default;
    virtual bool hasNext() = 0;
    virtual int next() = 0;
    virtual void reset(int start) = 0;
};

// ----------------------------- BFS -----------------------------
class BFSIterator : public GraphIterator {
public:
    BFSIterator(const Graph& g, int start) : graph_(g) { reset(start); }

    bool hasNext() override { return !queue_.empty(); }

    int next() override {
        if (queue_.empty()) throw out_of_range("BFSIterator: no hay mas nodos");
        int cur = queue_.front();
        queue_.pop();
        for (int n : graph_.neighbors(cur)) {
            if (visited_.insert(n).second) queue_.push(n);   // se marca al encolar
        }
        return cur;
    }

    void reset(int start) override {
        queue_ = {};
        visited_.clear();
        if (graph_.contains(start)) {
            visited_.insert(start);
            queue_.push(start);
        }
    }

private:
    const Graph& graph_;
    queue<int> queue_;
    set<int> visited_;
};

// ----------------------------- DFS -----------------------------
class DFSIterator : public GraphIterator {
public:
    DFSIterator(const Graph& g, int start) : graph_(g) { reset(start); }

    bool hasNext() override {
        while (!stack_.empty() && visited_.count(stack_.top())) stack_.pop();  // descarta ya visitados
        return !stack_.empty();
    }

    int next() override {
        if (!hasNext()) throw out_of_range("DFSIterator: no hay mas nodos");
        int cur = stack_.top();
        stack_.pop();
        visited_.insert(cur);
        auto adj = graph_.neighbors(cur);
        for (auto it = adj.rbegin(); it != adj.rend(); ++it)   // inverso: el primer vecino sale primero
            if (!visited_.count(*it)) stack_.push(*it);
        return cur;
    }

    void reset(int start) override {
        stack_ = {};
        visited_.clear();
        if (graph_.contains(start)) stack_.push(start);
    }

private:
    const Graph& graph_;
    stack<int> stack_;
    set<int> visited_;
};

int main() {
    Graph g;
    g.addEdge(1, 2);
    g.addEdge(1, 3);
    g.addEdge(2, 4);

    unique_ptr<GraphIterator> it = make_unique<BFSIterator>(g, 1);
    while (it->hasNext()) {
        cout << it->next() << " ";
    }
    cout << endl;
}