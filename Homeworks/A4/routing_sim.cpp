#include <iostream>
#include <vector>
#include <limits>
#include <queue>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <set>
using namespace std;

const int INF = 9999;

void printDVRTable(int node, const vector<vector<int>>& table, const vector<vector<int>>& nextHop) {
    cout << "Node " << node << " Routing Table:\n";
    cout << "Dest\tCost\tNext Hop\n";
    for (int i = 0; i < table.size(); ++i) {
        cout << i << "\t" << table[node][i] << "\t";
        if (nextHop[node][i] == -1) cout << "-";
        else cout << nextHop[node][i];
        cout << endl;
    }
    cout << endl;
}

void simulateDVR(const vector<vector<int>>& graph) {
    int n = graph.size();
    vector<vector<int>> dist = graph;
    vector<vector<int>> nextHop(n, vector<int>(n));

    
    // Initialize the nextHop table with -1, indicating no direct path initially
    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++) nextHop[i][j] = -1;
    }

    // Set the nextHop for directly connected nodes
    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            if(i==j) continue; // Skip the same node
            if(graph[i][j] == INF || graph[i][j] == 0) continue; // Skip if no direct connection
            nextHop[i][j] = j; // Directly connected nodes have the next hop as the destination itself 
        }
    }

    // Floyd-Warshall algorithm to compute shortest paths and update nextHop table
    for(int k=0;k<n;k++){ // Intermediate node
        for(int i=0;i<n;i++){ // Source node
            for(int j=0;j<n;j++){ // Destination node
                // If a shorter path is found via the intermediate node k
                if(dist[i][j] > dist[i][k] + dist[k][j]){
                    dist[i][j] = dist[i][k] + dist[k][j]; // Update the shortest distance
                    nextHop[i][j] = nextHop[i][k]; // Update the next hop to go through k
                }
            }
        }
    }
    cout << "--- DVR Final Tables ---\n";
    for (int i = 0; i < n; ++i) printDVRTable(i, dist, nextHop);
}

void printLSRTable(int src, const vector<int>& dist, const vector<int>& prev) {
    cout << "Node " << src << " Routing Table:\n";
    cout << "Dest\tCost\tNext Hop\n";
    for (int i = 0; i < dist.size(); ++i) {
        if (i == src) continue;
        cout << i << "\t" << dist[i] << "\t";
        int hop = i;
        while (prev[hop] != src && prev[hop] != -1)
            hop = prev[hop];
        cout << (prev[hop] == -1 ? -1 : hop) << endl;
    }
    cout << endl;
}

void simulateLSR(const vector<vector<int>>& graph) {
    int n = graph.size();
    for (int src = 0; src < n; ++src) {
        vector<int> dist(n, INF);
        vector<int> prev(n, -1);
        vector<bool> visited(n, false);
        // Initialize the distance to the source node as 0
        dist[src] = 0;

        // Use a set to act as a priority queue for Dijkstra's algorithm
        set<pair<int,int>> pq;

        // Pair to store the current node and its distance
        pair<int,int> currentPair;

        // Insert the source node with distance 0 into the priority queue
        pq.insert({0,src});

        // While the priority queue is not empty
        while(!pq.empty()){
            // Get the node with the smallest distance
            currentPair = *pq.begin();

            // Mark the current node as visited
            visited[currentPair.second] = true;

            // Remove the current node from the priority queue
            pq.erase(pq.begin());

            // Iterate through all neighbors of the current node
            for(int i=0;i<n;i++){
                // If the neighbor is not visited and there is a direct edge
                if(!visited[i] && graph[currentPair.second][i] != INF && graph[currentPair.second][i] != 0){
                    // If a shorter path to the neighbor is found
                    if(dist[i] > dist[currentPair.second] + graph[currentPair.second][i]){
                        // Remove the old distance of the neighbor from the priority queue
                        if(dist[i] != INF)
                            pq.erase({dist[i],i});

                        // Update the distance to the neighbor
                        dist[i] = dist[currentPair.second] + graph[currentPair.second][i];

                        // Update the previous node for the neighbor
                        prev[i] = currentPair.second;

                        // Insert the updated distance of the neighbor into the priority queue
                        pq.insert({dist[i],i});
                    }
                }
            }
        }
        

        
        printLSRTable(src, dist, prev);
    }
}

vector<vector<int>> readGraphFromFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        exit(1);
    }
    
    int n;
    file >> n;
    vector<vector<int>> graph(n, vector<int>(n));

    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            file >> graph[i][j];

    file.close();
    return graph;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <input_file>\n";
        return 1;
    }

    string filename = argv[1];
    vector<vector<int>> graph = readGraphFromFile(filename);

    cout << "\n--- Distance Vector Routing Simulation ---\n";
    simulateDVR(graph);

    cout << "\n--- Link State Routing Simulation ---\n";
    simulateLSR(graph);

    return 0;
}

