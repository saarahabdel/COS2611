/* 
   18647790_FP.cpp
   COS2611-25-Y: Final Practical Project — Smart City Route Management System
*/

#include <bits/stdc++.h>
using namespace std;

/* ===========================
   Data Structures Overview
   ===========================
   - Graph stored as adjacency list: unordered_map<string, vector<edge>>
   - Edge list also kept in a set (sorted by (from,to)) for quick existence checks
   - History stacks (undo/redo) implemented with std::stack
   - Vectors + custom functors for sorting routes by distance/time
   - Priority queue for Dijkstra
*/

struct Edge {
    string to;
    double distanceKm;   // base distance
    double baseMinutes;  // base travel time (no congestion)
    Edge(string t, double d, double m) : to(t), distanceKm(d), baseMinutes(m) {}
};

struct RouteKey {
    string from, to;
    bool operator<(const RouteKey& other) const {
        return (from < other.from) || (from == other.from && to < other.to);
    }
};

// For sorting views
struct ByDistance {
    bool operator()(const Edge& a, const Edge& b) const {
        return a.distanceKm < b.distanceKm;
    }
};
struct ByTime {
    bool operator()(const Edge& a, const Edge& b) const {
        return a.baseMinutes < b.baseMinutes;
    }
};

class Graph {
private:
    unordered_map<string, vector<Edge>> adj;         // adjacency list
    set<pair<string,string>> edgeIndex;              // quick membership check
    set<string> nodes;                               // all unique nodes

    // History for undo/redo
    enum class OpType { ADD, REMOVE, UPDATE };
    struct Op {
        OpType type;
        string from, to;
        double distanceKm_before, baseMinutes_before;
        double distanceKm_after,  baseMinutes_after;
    };
    stack<Op> undoStack, redoStack;

    // Simple "AI-like" congestion multiplier based on hour of day (0..23)
    // WHY: I make traffic higher in peak hours; document the rule clearly for transparency.
    double congestionMultiplier(int hour) const {
        // 07-09 and 16-18 are peaks; 1.35x time. Night is fastest at 0.85x; otherwise neutral 1.0x
        if ((hour >=7 && hour <=9) || (hour >=16 && hour <=18)) return 1.35;
        if (hour >= 22 || hour <= 5) return 0.85;
        return 1.0;
    }

public:
    bool addRoute(const string& from, const string& to, double dist, double mins) {
        if (dist <= 0 || mins <= 0) return false;
        if (edgeIndex.count({from,to})) return false; // already exists
        adj[from].push_back(Edge(to, dist, mins));
        edgeIndex.insert({from,to});
        nodes.insert(from); nodes.insert(to);

        // Record history
        undoStack.push({OpType::ADD, from, to, 0,0, dist, mins});
        // Clear redo after new action
        while(!redoStack.empty()) redoStack.pop();
        return true;
    }

    bool removeRoute(const string& from, const string& to) {
        if (!edgeIndex.count({from,to})) return false;
        auto &vec = adj[from];
        for (size_t i=0;i<vec.size();++i) {
            if (vec[i].to == to) {
                // Save for undo
                undoStack.push({OpType::REMOVE, from, to, vec[i].distanceKm, vec[i].baseMinutes, 0,0});
                // Clear redo
                while(!redoStack.empty()) redoStack.pop();

                vec.erase(vec.begin()+i);
                edgeIndex.erase({from,to});
                return true;
            }
        }
        return false;
    }

    bool updateRoute(const string& from, const string& to, double newDist, double newMins) {
        if (!edgeIndex.count({from,to}) || newDist<=0 || newMins<=0) return false;
        auto &vec = adj[from];
        for (auto& e : vec) {
            if (e.to == to) {
                // Save old for undo
                undoStack.push({OpType::UPDATE, from, to, e.distanceKm, e.baseMinutes, newDist, newMins});
                // Clear redo
                while(!redoStack.empty()) redoStack.pop();

                e.distanceKm = newDist;
                e.baseMinutes = newMins;
                return true;
            }
        }
        return false;
    }

    bool undo() {
        if (undoStack.empty()) return false;
        Op op = undoStack.top(); undoStack.pop();
        // Reverse effect
        if (op.type == OpType::ADD) {
            // Undo add -> remove
            removeInternal(op.from, op.to);
            // For redo
            redoStack.push(op);
        } else if (op.type == OpType::REMOVE) {
            // Undo remove -> add back with old values
            addInternal(op.from, op.to, op.distanceKm_before, op.baseMinutes_before);
            redoStack.push(op);
        } else { // UPDATE
            // Revert to "before"
            updateInternal(op.from, op.to, op.distanceKm_before, op.baseMinutes_before);
            redoStack.push(op);
        }
        return true;
    }

    bool redo() {
        if (redoStack.empty()) return false;
        Op op = redoStack.top(); redoStack.pop();
        // Re-apply original effect
        if (op.type == OpType::ADD) {
            addInternal(op.from, op.to, op.distanceKm_after, op.baseMinutes_after);
            undoStack.push(op);
        } else if (op.type == OpType::REMOVE) {
            removeInternal(op.from, op.to);
            undoStack.push(op);
        } else {
            updateInternal(op.from, op.to, op.distanceKm_after, op.baseMinutes_after);
            undoStack.push(op);
        }
        return true;
    }

    // Helper (no history mutation)
    void addInternal(const string& from, const string& to, double d, double m) {
        if (!edgeIndex.count({from,to})) {
            adj[from].push_back(Edge(to,d,m));
            edgeIndex.insert({from,to});
            nodes.insert(from); nodes.insert(to);
        }
    }
    void removeInternal(const string& from, const string& to) {
        if (!edgeIndex.count({from,to})) return;
        auto &vec = adj[from];
        for (size_t i=0;i<vec.size();++i) if (vec[i].to==to) { vec.erase(vec.begin()+i); break; }
        edgeIndex.erase({from,to});
    }
    void updateInternal(const string& from, const string& to, double d, double m) {
        auto &vec = adj[from];
        for (auto& e : vec) if (e.to==to){ e.distanceKm=d; e.baseMinutes=m; return; }
    }

    void listAllRoutesSortedBy(const string& from, bool byTime) const {
        auto it = adj.find(from);
        if (it == adj.end() || it->second.empty()) {
            cout << "No outgoing routes from " << from << ".\n";
            return;
        }
        vector<Edge> v = it->second;
        if (byTime) {
            // Sort by base travel time because the official asked to prioritise time.
            sort(v.begin(), v.end(), ByTime());
        } else {
            // Sort by distance for a distance-focused view.
            sort(v.begin(), v.end(), ByDistance());
        }
        cout << "Routes from " << from << " (" << (byTime? "sorted by time" : "sorted by distance") << "):\n";
        for (auto &e : v) {
            cout << "  -> " << e.to << "  [distance=" << e.distanceKm << " km, base time=" << e.baseMinutes << " min]\n";
        }
    }

    void viewAll() const {
        if (adj.empty()) { cout << "No routes in the network yet.\n"; return; }
        cout << "=== Current Route Network ===\n";
        for (auto &p : adj) {
            cout << p.first << ":\n";
            for (auto &e : p.second) {
                cout << "  -> " << e.to << "  [distance=" << e.distanceKm 
                     << " km, base time=" << e.baseMinutes << " min]\n";
            }
        }
    }

    // Dijkstra shortest path by *time*, optionally applying congestion by hour.
    // If useCongestion==true, edge time = baseMinutes * congestionMultiplier(hour)
    // WHY: I choose Dijkstra because all edge costs are non-negative times; 
    //      the algorithm guarantees optimality for such graphs.
    vector<string> shortestPath(const string& src, const string& dst, bool useCongestion, int hour,
                                double& outTotalMinutes, double& outTotalDistance,
                                vector<string>& xaiTrace) const
    {
        xaiTrace.clear();
        const double INF = 1e18;
        unordered_map<string, double> dist;    // minutes cost
        unordered_map<string, double> distKm;  // track distance for explanation
        unordered_map<string, string> parent;
        struct Node { double d; string v; };
        struct Cmp { bool operator()(const Node& a, const Node& b) const { return a.d > b.d; } };
        priority_queue<Node, vector<Node>, Cmp> pq;

        // Init
        for (auto &n : nodes) { dist[n]=INF; distKm[n]=0; }
        if (!nodes.count(src) || !nodes.count(dst)) {
            xaiTrace.push_back("Either source or destination does not exist in the graph.");
            outTotalMinutes = outTotalDistance = INF;
            return {};
        }
        dist[src]=0; distKm[src]=0; parent[src]="";
        pq.push({0, src});
        xaiTrace.push_back("Start at " + src + " with initial cost 0.");

        double mult = useCongestion ? congestionMultiplier(hour) : 1.0;
        if (useCongestion) {
            // Applying congestion multiplier to base times to reflect time-of-day traffic.
            xaiTrace.push_back("Congestion multiplier at hour " + to_string(hour) + " is " + to_string(mult) + ".");
        } else {
            xaiTrace.push_back("No congestion applied: using base travel times.");
        }

        // Dijkstra
        while (!pq.empty()) {
            auto [cd, u] = pq.top(); pq.pop();
            if (cd != dist[u]) continue; // skip stale entry

            // Node selection rationale
            xaiTrace.push_back("Selecting node " + u + " next because it currently has the smallest known travel time (" + to_string(cd) + " min).");

            if (u == dst) break; // early exit possible

            auto it = adj.find(u);
            if (it == adj.end()) continue;
            for (auto &e : it->second) {
                double w = e.baseMinutes * mult; // effective time
                double nd = dist[u] + w;
                if (nd < dist[e.to]) {
                    dist[e.to] = nd;
                    distKm[e.to] = distKm[u] + e.distanceKm;
                    parent[e.to] = u;
                    pq.push({nd, e.to});
                    // Relaxation explanation
                    xaiTrace.push_back("Updated best time to " + e.to + " via " + u + " to " + to_string(nd) + " min (distance so far " + to_string(distKm[e.to]) + " km).");
                }
            }
        }

        if (dist[dst] >= INF/2) {
            xaiTrace.push_back("No path found from " + src + " to " + dst + ".");
            outTotalMinutes = outTotalDistance = INF;
            return {};
        }

        // Reconstruct path
        vector<string> path;
        for (string v = dst; !v.empty(); v = parent[v]) path.push_back(v);
        reverse(path.begin(), path.end());

        outTotalMinutes = dist[dst];
        outTotalDistance = distKm[dst];

        // Final justification
        xaiTrace.push_back("Shortest path found using Dijkstra. Nodes visited are those selected with smallest known times.");
        xaiTrace.push_back("Total cost: " + to_string(outTotalMinutes) + " minutes; Total distance: " + to_string(outTotalDistance) + " km.");
        return path;
    }

    bool routeExists(const string& from, const string& to) const {
        return edgeIndex.count({from,to});
    }
};

void seedDemoData(Graph& g) {
    // A small demo city network
    g.addRoute("CBD","Station", 2.0, 6.0);
    g.addRoute("CBD","Harbour", 3.5, 10.0);
    g.addRoute("Station","Harbour", 1.2, 4.0);
    g.addRoute("Station","Airport", 12.0, 25.0);
    g.addRoute("Harbour","Airport", 10.0, 18.0);
    g.addRoute("Harbour","University", 5.0, 12.0);
    g.addRoute("CBD","University", 7.0, 20.0);
    g.addRoute("University","Airport", 8.0, 16.0);
}

void printMenu() {
    cout << "\n===== SMART CITY ROUTE MANAGEMENT =====\n";
    cout << "1. Add a route\n";
    cout << "2. Remove a route\n";
    cout << "3. Update a route\n";
    cout << "4. View all routes\n";
    cout << "5. View routes from a node (sorted by distance)\n";
    cout << "6. View routes from a node (sorted by time)\n";
    cout << "7. Find the shortest path (base time)\n";
    cout << "8. Find the shortest path (with congestion)\n";
    cout << "9. Undo\n";
    cout << "10. Redo\n";
    cout << "0. Exit\n";
    cout << "Select: ";
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Graph g;
    seedDemoData(g);

    // Start with a small seeded network to let officials test immediately.
    cout << "Seeded a demo network with CBD/Station/Harbour/University/Airport.\n";

    while (true) {
        printMenu();
        int choice; 
        if (!(cin >> choice)) break;

        if (choice == 0) {
            cout << "Goodbye!\n";
            break;
        }

        if (choice == 1) {
            string a,b; double d,m;
            cout << "From node: "; cin >> a;
            cout << "To node: "; cin >> b;
            cout << "Distance (km): "; cin >> d;
            cout << "Base time (min): "; cin >> m;
            if (g.addRoute(a,b,d,m)) {
                cout << "Route added.\n";
                
                cout << "Added a new edge because it did not exist and inputs were valid (positive distance/time).\n";
            } else cout << "Failed to add route (maybe exists or invalid values).\n";
        }
        else if (choice == 2) {
            string a,b;
            cout << "From node: "; cin >> a;
            cout << "To node: "; cin >> b;
            if (g.removeRoute(a,b)) {
                cout << "Route removed.\n";
                cout << "Removed the edge to update the network topology as requested.\n";
            } else cout << "Route not found.\n";
        }
        else if (choice == 3) {
            string a,b; double d,m;
            cout << "From node: "; cin >> a;
            cout << "To node: "; cin >> b;
            cout << "New distance (km): "; cin >> d;
            cout << "New base time (min): "; cin >> m;
            if (g.updateRoute(a,b,d,m)) {
                cout << "Route updated.\n";
                cout << "Updated the edge to reflect changed measurements; consistency maintained.\n";
            } else cout << "Update failed (route missing or invalid values).\n";
        }
        else if (choice == 4) {
            g.viewAll();
            cout << "Viewing the raw network helps verify data prior to optimisation.\n";
        }
        else if (choice == 5) {
            string a; cout << "From node: "; cin >> a;
            g.listAllRoutesSortedBy(a,false);
            cout << "Sorted by distance to emphasise shorter physical routes.\n";
        }
        else if (choice == 6) {
            string a; cout << "From node: "; cin >> a;
            g.listAllRoutesSortedBy(a,true);
            cout << "Sorted by time to emphasise faster routes.\n";
        }
        else if (choice == 7 || choice == 8) {
            string s,t; 
            cout << "Source: "; cin >> s;
            cout << "Destination: "; cin >> t;
            bool useCong = (choice==8);
            int hour = 12;
            if (useCong) { cout << "Hour of day (0..23): "; cin >> hour; }
            double totalMin, totalKm;
            vector<string> xai;
            auto path = g.shortestPath(s,t,useCong,hour,totalMin,totalKm,xai);
            if (path.empty()) {
                cout << "No path found.\n";
            } else {
                cout << "Shortest path: ";
                for (size_t i=0;i<path.size();++i) {
                    cout << path[i] << (i+1==path.size() ? "" : " -> ");
                }
                cout << "\nTotal time: " << fixed << setprecision(2) << totalMin << " min";
                cout << " | Total distance: " << fixed << setprecision(2) << totalKm << " km\n";
                cout << "\n--- XAI TRACE ---\n";
                for (auto& line : xai) cout << line << "\n";
                cout << "-----------------\n";
            }
        }
        else if (choice == 9) {
            if (g.undo()) cout << "Undo successful.\n";
            else cout << "Nothing to undo.\n";
        }
        else if (choice == 10) {
            if (g.redo()) cout << "Redo successful.\n";
            else cout << "Nothing to redo.\n";
        }
        else {
            cout << "Invalid choice.\n";
        }
    }
    return 0;
}

/* ===============
   Documentation 
   ===============

1) Problem analysis & approach
   I model intersections as nodes and roads as directed edges with two base attributes:
   - distanceKm (physical length)
   - baseMinutes (uncongested travel time)
   The graph is an adjacency list (unordered_map<string, vector<Edge>>), which is memory efficient
   for sparse urban networks and fast for typical operations.

2) Data structures & algorithms used (and WHY)
   - unordered_map<string, vector<Edge>> (Graph/Adjacency List): O(1) average access by node; fits well for varying degrees.
   - set<pair<string,string>>: Keeps a canonical index of existing routes, enabling quick existence checks and preventing duplicates.
   - vector<Edge> + custom functors (ByDistance, ByTime): Supports sorting by different criteria (distance vs time).
   - priority_queue for Dijkstra: Efficiently selects next node with smallest known cost.
   - stack<Op> for undo/redo: Provides simple, LIFO history of edits (add/remove/update).

   Graph Algorithm:
   - Dijkstra (by time): Non-negative travel times satisfy Dijkstra’s optimality conditions.
     We also track cumulative distance for richer explanations.

3) Principles applied (WHERE)
     * Why a node is chosen by Dijkstra (smallest known time).
     * Why edges are relaxed (new shorter time found).
     * Why sorting is by time or distance for different views.
     * Why congestion is (optionally) applied.
     * Why edits (add/remove/update) succeed/fail.
   - The program prints a structured "XAI TRACE" when computing shortest paths, showing:
     * Start conditions
     * Congestion multiplier decisions
     * Node selections and relaxations
     * Final justification with total time and distance

4) AI integration
   - A lightweight, rule-based "congestionMultiplier(hour)" acts as an interpretable AI-like component:
     Peak hours (07–09, 16–18) inflate times by 1.35x, nights (22–05) deflate to 0.85x; else 1.0x.
     This is simple, transparent, and fully explained in the XAI output.

5) Menu-driven interface
   - Required actions supported:
     * Add, Remove, Update routes
     * View all routes
     * Sort views (by distance or time)
     * Find shortest path (with/without congestion)
     * Undo/Redo changes

6) Possible extensions
   - Visualisation layer for nodes/edges
   - Export/import to CSV
   - Compare Dijkstra vs BFS on hop-count (with explanation)
*/