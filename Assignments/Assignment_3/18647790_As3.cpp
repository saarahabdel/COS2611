/*
----------------------------------------------------------------------------------
Name: Saarah Abdelmaged
Student Number: 18647790
----------------------------------------------------------------------------------
*/

#include <bits/stdc++.h>
using namespace std;

// --------------------------- Utility Pretty Printers ----------------------------

// Small helper to left-pad or right-pad text in tables.
static string padRight(const string &s, size_t width) {
    if (s.size() >= width) return s.substr(0, width);
    return s + string(width - s.size(), ' ');
}

// ------------------------------- Graph Class -----------------------------------

class Graph {
public:
    /*
    We store:
    - names:    index -> city name (e.g., 0 -> "Cape Town")
    - code:     index -> short code for display (e.g., "CT")
    - indexOf:  city name -> index, for quick lookup
    - adj:      adjacency list: adj[u] = vector of (v, distanceKm)
    */
    vector<string> names;
    vector<string> codes;
    unordered_map<string, int> indexOf;
    vector<vector<pair<int,int>>> adj; // pair: (neighborIndex, distanceKm)

    // Constructor creates N empty lists for N cities
    Graph(int n = 0) {
        names.reserve(n);
        codes.reserve(n);
        adj.assign(n, {});
    }

    // Add a new city; returns its index
    int addCity(const string &name, const string &code) {
        int idx = (int)names.size();
        names.push_back(name);
        codes.push_back(code);
        indexOf[name] = idx;
        adj.push_back({}); // ensure adjacency list exists
        return idx;
    }

    // Add an undirected edge (road) with a distance in km
    void addUndirectedRoad(const string &fromCity, const string &toCity, int km) {
        int u = indexOf.at(fromCity);
        int v = indexOf.at(toCity);
        // Push both directions because roads are two-way for this model
        adj[u].push_back({v, km});
        adj[v].push_back({u, km});
    }

    // Print a human-friendly summary of the graph
    void printSummary() const {
        cout << "Southern Africa Transport Graph\n";
        cout << "------------------------------------------------------\n";
        cout << "Cities (" << names.size() << "): ";
        for (size_t i = 0; i < names.size(); ++i) {
            cout << names[i] << " (" << codes[i] << ")";
            if (i + 1 != names.size()) cout << ", ";
        }
        cout << "\n\n";
        cout << "Connections (direct roads with distances):\n";
        for (size_t u = 0; u < names.size(); ++u) {
            cout << "  - " << padRight(names[u] + " (" + codes[u] + ")", 24) << " -> ";
            if (adj[u].empty()) {
                cout << "(no direct roads)";
            } else {
                // List all neighbours of u
                for (size_t k = 0; k < adj[u].size(); ++k) {
                    int v = adj[u][k].first;
                    int km = adj[u][k].second;
                    cout << names[v] << " (" << codes[v] << ", " << km << " km)";
                    if (k + 1 != adj[u].size()) cout << "; ";
                }
            }
            cout << "\n";
        }
        cout << "\n";
    }

    // Display the adjacency matrix of distances (0 = no direct road; diagonal = 0)
    void printAdjacencyMatrix() const {
        const int n = (int)names.size();
        // Build matrix initialised with 0s
        vector<vector<int>> mat(n, vector<int>(n, 0));
        // Fill from adjacency list
        for (int u = 0; u < n; ++u) {
            for (auto [v, km] : adj[u]) {
                mat[u][v] = km;
            }
        }

        // Header row: codes
        const int cellW = 9;
        cout << "Adjacency Matrix (Distances in km):\n\n";
        cout << padRight("", cellW);
        for (int j = 0; j < n; ++j) {
            cout << padRight(codes[j], cellW);
        }
        cout << "\n";

        // Rows
        for (int i = 0; i < n; ++i) {
            cout << padRight(codes[i], cellW);
            for (int j = 0; j < n; ++j) {
                // Print 0 if no direct road (or i==j), else the distance
                string cell = to_string(mat[i][j]);
                cout << padRight(cell, cellW);
            }
            cout << "\n";
        }
        cout << "\n";
    }

    // ---------------------------- BFS Traversal --------------------------------
    /*
    BFS (Breadth-First Search) explanation (XAI):
    - We start at a chosen city 'start' and explore all its direct neighbours
      first (distance 1 hop), then the neighbours of those (distance 2 hops),
      and so on.
    - The queue ensures we visit in "rings" around the start city.
    - The 'visited' array prevents revisiting the same city.
    - Output: The order we discover cities, demonstrating reachability.
    */

    vector<int> bfsOrder(const string &startCity) const {
        vector<int> order;                         // store the visiting order
        auto it = indexOf.find(startCity);
        if (it == indexOf.end()) return order;    // return empty if city unknown
        int s = it->second;

        vector<int> visited(names.size(), 0);
        queue<int> q;
        visited[s] = 1;
        q.push(s);

        while (!q.empty()) {
            int u = q.front(); q.pop();
            order.push_back(u);
            for (auto [v, km] : adj[u]) {
                (void)km; // distance not needed for BFS ordering
                if (!visited[v]) {
                    visited[v] = 1;
                    q.push(v);
                }
            }
        }
        return order;
    }

    // ------------------------- Dijkstra's Algorithm ----------------------------
    /*
    Dijkstra (XAI):
    - We want minimal total distance from source to every other city.
    - 'dist[i]' stores the best distance found so far to city i.
    - We use a min-heap priority queue always picking the next city with the
      smallest temporary distance.
    - When we find a shorter path to a neighbour, we update dist[] and record
      the predecessor to reconstruct the route.
    */

    pair<int, vector<int>> dijkstraPath(const string &srcCity, const string &dstCity) const {
        auto itS = indexOf.find(srcCity);
        auto itT = indexOf.find(dstCity);
        if (itS == indexOf.end() || itT == indexOf.end()) {
            return {INT_MAX, {}}; // invalid cities
        }
        int s = itS->second, t = itT->second;
        const int n = (int)names.size();
        const int INF = 1e9;

        vector<int> dist(n, INF);
        vector<int> parent(n, -1);
        // min-heap of (distanceSoFar, node)
        priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;

        dist[s] = 0;
        pq.push({0, s});

        while (!pq.empty()) {
            auto [d, u] = pq.top(); pq.pop();
            if (d != dist[u]) continue; // skip stale entry
            if (u == t) break;          // we can stop early if we reached dest

            for (auto [v, w] : adj[u]) {
                if (dist[v] > dist[u] + w) {
                    dist[v] = dist[u] + w;
                    parent[v] = u;
                    pq.push({dist[v], v});
                }
            }
        }

        if (dist[t] >= INF) {
            return {INT_MAX, {}}; // no path exists
        }

        // Reconstruct path from t back to s using parent[]
        vector<int> route;
        for (int cur = t; cur != -1; cur = parent[cur]) {
            route.push_back(cur);
        }
        reverse(route.begin(), route.end());
        return {dist[t], route};
    }
};

Graph buildSampleGraph() {
    Graph G;
    // Add at least 5 cities (we add 8 for richness)
    int ct  = G.addCity("Cape Town",      "CT");
    int jhb = G.addCity("Johannesburg",   "JHB");
    int dbn = G.addCity("Durban",         "DBN");
    int gbe = G.addCity("Gaborone",       "GBE");
    int wdh = G.addCity("Windhoek",       "WDH");
    int hre = G.addCity("Harare",         "HRE");
    int mpt = G.addCity("Maputo",         "MPT");
    int bfn = G.addCity("Bloemfontein",   "BFN");

    (void)ct; (void)jhb; (void)dbn; (void)gbe; (void)wdh; (void)hre; (void)mpt; (void)bfn;

    // Undirected roads with approximate distances (km)
    G.addUndirectedRoad("Cape Town",    "Bloemfontein", 1000);
    G.addUndirectedRoad("Cape Town",    "Windhoek",     1490);
    G.addUndirectedRoad("Bloemfontein", "Johannesburg", 400);
    G.addUndirectedRoad("Bloemfontein", "Durban",       640);
    G.addUndirectedRoad("Johannesburg", "Durban",       570);
    G.addUndirectedRoad("Johannesburg", "Gaborone",     360);
    G.addUndirectedRoad("Johannesburg", "Harare",       1100);
    G.addUndirectedRoad("Johannesburg", "Maputo",       500);
    G.addUndirectedRoad("Gaborone",     "Windhoek",     950);
    G.addUndirectedRoad("Gaborone",     "Harare",       960);
    G.addUndirectedRoad("Harare",       "Maputo",       530);

    return G;
}

// ------------------------------- Menu Helpers ----------------------------------

// Get a valid city name from user input; we accept either code or full name.
string normalizeCityInput(const Graph &G, const string &raw) {
    string s = raw;
    // Trim whitespace
    auto trim = [](string &x){
        while(!x.empty() && isspace((unsigned char)x.front())) x.erase(x.begin());
        while(!x.empty() && isspace((unsigned char)x.back()))  x.pop_back();
    };
    trim(s);

    // Case-insensitive compare helper
    auto eqCaseInsensitive = [](const string &a, const string &b){
        if (a.size() != b.size()) return false;
        for (size_t i = 0; i < a.size(); ++i) {
            if (tolower((unsigned char)a[i]) != tolower((unsigned char)b[i])) return false;
        }
        return true;
    };

    // Try match by code first
    for (size_t i = 0; i < G.codes.size(); ++i) {
        if (eqCaseInsensitive(s, G.codes[i])) return G.names[i];
    }
    // Then by full name
    for (size_t i = 0; i < G.names.size(); ++i) {
        if (eqCaseInsensitive(s, G.names[i])) return G.names[i];
    }
    return ""; // not found
}

// Print a route sequence nicely: CityA -> CityB -> CityC
void printRoute(const Graph &G, const vector<int> &route) {
    for (size_t i = 0; i < route.size(); ++i) {
        cout << G.names[route[i]] << " (" << G.codes[route[i]] << ")";
        if (i + 1 != route.size()) cout << " -> ";
    }
    cout << "\n";
}

// ------------------------------ Program Entry ----------------------------------

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Graph G = buildSampleGraph();

    cout << "==================== Transport Connectivity Tool ====================\n";
    cout << "This tool models cities as vertices and roads as weighted edges.\n";
    cout << "You can view the graph, run BFS (reachability), and run Dijkstra\n";
    cout << "(shortest paths) between two cities.\n";
    cout << "----------------------------------------------------------------------\n\n";

    // Always show a summary and adjacency matrix first (visualisation requirement)
    G.printSummary();
    G.printAdjacencyMatrix();

    // Interactive mini-menu (simple and clear)
    while (true) {
        cout << "Choose an option:\n";
        cout << "  1) BFS traversal from a city (reachability order)\n";
        cout << "  2) Dijkstra shortest path between two cities\n";
        cout << "  3) Show graph summary & adjacency matrix again\n";
        cout << "  0) Exit\n";
        cout << "Enter choice: ";
        int choice;
        if (!(cin >> choice)) return 0;
        cout << "\n";

        if (choice == 0) {
            cout << "Goodbye!\n";
            break;
        } else if (choice == 1) {
            cout << "Enter starting city (code or full name, e.g., JHB or Johannesburg): ";
            string raw; cin.ignore(numeric_limits<streamsize>::max(), '\n');
            getline(cin, raw);
            string start = normalizeCityInput(G, raw);
            if (start.empty()) {
                cout << "City not recognised. Please try again.\n\n";
                continue;
            }
            auto order = G.bfsOrder(start);
            if (order.empty()) {
                cout << "No traversal produced (unexpected). Try another city.\n\n";
                continue;
            }
            cout << "BFS Traversal starting from " << start << ":\n";
            for (size_t i = 0; i < order.size(); ++i) {
                cout << G.names[order[i]] << " (" << G.codes[order[i]] << ")";
                if (i + 1 != order.size()) cout << " -> ";
            }
            cout << "\n\n";
        } else if (choice == 2) {
            cout << "Enter source city  (code or full name): ";
            string rawS; cin.ignore(numeric_limits<streamsize>::max(), '\n');
            getline(cin, rawS);
            string src = normalizeCityInput(G, rawS);

            cout << "Enter target city  (code or full name): ";
            string rawT; getline(cin, rawT);
            string dst = normalizeCityInput(G, rawT);

            if (src.empty() || dst.empty()) {
                cout << "One or both cities not recognised. Please try again.\n\n";
                continue;
            }

            auto [bestKm, route] = G.dijkstraPath(src, dst);
            if (bestKm == INT_MAX || route.empty()) {
                cout << "No path found between " << src << " and " << dst << ".\n\n";
                continue;
            }
            cout << "Dijkstra’s Shortest Path from " << src << " to " << dst << ":\n";
            cout << "Path: ";
            printRoute(G, route);
            cout << "Total distance: " << bestKm << " km\n\n";
        } else if (choice == 3) {
            G.printSummary();
            G.printAdjacencyMatrix();
        } else {
            cout << "Invalid choice. Please try again.\n\n";
        }
    }

    return 0;
}

/** 
 * PROMPTS USED
 * 
 * “Summarise the COS2611 Assessment 3 brief and list every rubric requirement.”
 * Propose a minimal scope that still gets full marks (cities, edges, BFS, Dijkstra, adjacency matrix).
 * Draft a C++17 Graph class API (addCity, addRoad, bfsOrder, dijkstraPath, printAdjacencyMatrix).
 * Write BFS (queue) and Dijkstra (min-heap) methods with O(V+E) and O(E log V) explained in 2 lines each.
 * Pick 8 Southern African cities with reasonable distances; keep the graph connected.
 * Add a simple text menu: 1) BFS 2) Dijkstra 3) Show graph 0) Exit.
 * Format outputs neatly (summary, adjacency matrix, and route like A -> B -> C).
 * Add XAI-style top comments explaining data structures and algorithm choices (5–8 lines).
 * Give me 4 test pairs with expected shortest distances to verify correctness.
 * Do a quick rubric check on my final .cpp and list anything missing.”
 * Give 5 viva questions (BFS, Dijkstra, adjacency list vs matrix) with one-sentence answers.”
 * 
 */

/** 
----------------------------------------------------------------------------------
SHORT REFLECTION (5–10 lines)
- Helpful: Using an adjacency list made the representation compact and easy to
  explain; converting to an adjacency matrix afterwards satisfied the rubric and
  improved readability for non-programmers.
- BFS and Dijkstra were straightforward with STL queue and priority_queue; the
  biggest challenge was balancing clear XAI comments with concise code.
- I adjusted city distances to reasonable approximations for educational use,
  not exact mapping data, to avoid external data dependencies.
- I added a small interactive menu so the marker can test BFS and Dijkstra
  on multiple city pairs during one run.
==================================================================================
*/