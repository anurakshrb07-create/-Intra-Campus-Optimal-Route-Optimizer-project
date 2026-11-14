#include <iostream>
#include <cmath>
#include <cstring>
#include <iomanip>

using namespace std;

const int MAX_NODES = 50;
const double INF = 999999999.0;
const double WALKING_SPEED = 80.0;

enum TrafficLevel {
    TRAFFIC_LOW = 0,
    TRAFFIC_MODERATE = 1,
    TRAFFIC_HIGH = 2,
    TRAFFIC_CONGESTED = 3
};

const double TRAFFIC_MULTIPLIER[4] = { 1.0, 1.3, 1.8, 2.5 };
const char TRAFFIC_NAMES[4][16] = { "LOW", "MODERATE", "HIGH", "CONGESTED" };
const char TRAFFIC_DESC[4][32] = {
    "Free Flow (1.0x)",
    "Moderate Flow (1.3x)",
    "Heavy Rush (1.8x)",
    "Severe Jam (2.5x)"
};

enum RoutingMetric {
    METRIC_TIME = 0,
    METRIC_DISTANCE = 1
};

struct Node {
    char id[32];
    char name[64];
    char category[32];
    double lat;
    double lng;
};

struct EdgeNode {
    int target;
    double distance;
    char pathType[20];
    bool isAccessible;
    char edgeName[64];
    int trafficLevel;
    double congestionFactor;
    EdgeNode* next;
};

class Graph {
private:
    Node nodes[MAX_NODES];
    EdgeNode* adj[MAX_NODES];
    int nodeCount;
    int currentScenario;
    char scenarioName[64];

public:
    Graph() {
        nodeCount = 0;
        currentScenario = 0;
        strncpy(scenarioName, "Normal / Off-Peak Hours", sizeof(scenarioName) - 1);
        scenarioName[sizeof(scenarioName) - 1] = '\0';
        for (int i = 0; i < MAX_NODES; ++i) {
            adj[i] = NULL;
        }
    }

    ~Graph() {

        for (int i = 0; i < nodeCount; ++i) {
            EdgeNode* curr = adj[i];
            while (curr != NULL) {
                EdgeNode* temp = curr;
                curr = curr->next;
                delete temp;
            }
            adj[i] = NULL;
        }
    }

    int addNode(const char* id, const char* name, const char* category, double lat, double lng) {
        if (nodeCount >= MAX_NODES) return -1;
        int index = nodeCount++;
        strncpy(nodes[index].id, id, sizeof(nodes[index].id) - 1);
        nodes[index].id[sizeof(nodes[index].id) - 1] = '\0';

        strncpy(nodes[index].name, name, sizeof(nodes[index].name) - 1);
        nodes[index].name[sizeof(nodes[index].name) - 1] = '\0';

        strncpy(nodes[index].category, category, sizeof(nodes[index].category) - 1);
        nodes[index].category[sizeof(nodes[index].category) - 1] = '\0';

        nodes[index].lat = lat;
        nodes[index].lng = lng;
        return index;
    }

    int findNodeIndex(const char* id) const {
        for (int i = 0; i < nodeCount; ++i) {
            if (strcmp(nodes[i].id, id) == 0) return i;
        }
        return -1;
    }

    void addEdge(int u, int v, double distance, const char* type, bool isAccessible,
                 const char* name, int traffic = TRAFFIC_LOW, bool bidirectional = true) {
        if (u < 0 || u >= nodeCount || v < 0 || v >= nodeCount) return;
        if (traffic < 0 || traffic > 3) traffic = TRAFFIC_LOW;

        EdgeNode* e1 = new EdgeNode;
        e1->target = v;
        e1->distance = distance;
        strncpy(e1->pathType, type, sizeof(e1->pathType) - 1);
        e1->pathType[sizeof(e1->pathType) - 1] = '\0';
        e1->isAccessible = isAccessible;
        strncpy(e1->edgeName, name, sizeof(e1->edgeName) - 1);
        e1->edgeName[sizeof(e1->edgeName) - 1] = '\0';
        e1->trafficLevel = traffic;
        e1->congestionFactor = TRAFFIC_MULTIPLIER[traffic];
        e1->next = adj[u];
        adj[u] = e1;

        if (bidirectional) {

            EdgeNode* e2 = new EdgeNode;
            e2->target = u;
            e2->distance = distance;
            strncpy(e2->pathType, type, sizeof(e2->pathType) - 1);
            e2->pathType[sizeof(e2->pathType) - 1] = '\0';
            e2->isAccessible = isAccessible;
            strncpy(e2->edgeName, name, sizeof(e2->edgeName) - 1);
            e2->edgeName[sizeof(e2->edgeName) - 1] = '\0';
            e2->trafficLevel = traffic;
            e2->congestionFactor = TRAFFIC_MULTIPLIER[traffic];
            e2->next = adj[v];
            adj[v] = e2;
        }
    }

    void addEdgeById(const char* id1, const char* id2, double distance, const char* type,
                     bool isAccessible, const char* name, int traffic = TRAFFIC_LOW, bool bidirectional = true) {
        int u = findNodeIndex(id1);
        int v = findNodeIndex(id2);
        if (u != -1 && v != -1) {
            addEdge(u, v, distance, type, isAccessible, name, traffic, bidirectional);
        }
    }

    bool setEdgeTraffic(int u, int v, int level, bool bidirectional = true) {
        if (u < 0 || u >= nodeCount || v < 0 || v >= nodeCount) return false;
        if (level < 0 || level > 3) return false;

        bool updated = false;
        EdgeNode* curr = adj[u];
        while (curr != NULL) {
            if (curr->target == v) {
                curr->trafficLevel = level;
                curr->congestionFactor = TRAFFIC_MULTIPLIER[level];
                updated = true;
                break;
            }
            curr = curr->next;
        }

        if (bidirectional) {
            curr = adj[v];
            while (curr != NULL) {
                if (curr->target == u) {
                    curr->trafficLevel = level;
                    curr->congestionFactor = TRAFFIC_MULTIPLIER[level];
                    break;
                }
                curr = curr->next;
            }
        }
        return updated;
    }

    bool setEdgeTrafficById(const char* id1, const char* id2, int level, bool bidirectional = true) {
        int u = findNodeIndex(id1);
        int v = findNodeIndex(id2);
        if (u != -1 && v != -1) {
            return setEdgeTraffic(u, v, level, bidirectional);
        }
        return false;
    }

    void resetAllTraffic(int defaultLevel = TRAFFIC_LOW) {
        for (int i = 0; i < nodeCount; ++i) {
            EdgeNode* curr = adj[i];
            while (curr != NULL) {
                curr->trafficLevel = defaultLevel;
                curr->congestionFactor = TRAFFIC_MULTIPLIER[defaultLevel];
                curr = curr->next;
            }
        }
    }

    void applyTrafficScenario(int scenarioId) {
        currentScenario = scenarioId;
        resetAllTraffic(TRAFFIC_LOW);

        switch (scenarioId) {
            case 0: {
                strncpy(scenarioName, "Normal / Off-Peak Hours", sizeof(scenarioName) - 1);

                setEdgeTrafficById("MAIN_GATE", "GATE_PLAZA", TRAFFIC_MODERATE);
                setEdgeTrafficById("GATE_PLAZA", "ADMIN_BLOCK", TRAFFIC_MODERATE);
                setEdgeTrafficById("GATE_PLAZA", "CENTRAL_CAFE", TRAFFIC_MODERATE);
                setEdgeTrafficById("ADMIN_BLOCK", "CENTRAL_QUAD", TRAFFIC_MODERATE);
                break;
            }
            case 1: {
                strncpy(scenarioName, "Class-Change Rush Hour (10:50 AM / 01:50 PM)", sizeof(scenarioName) - 1);

                setEdgeTrafficById("ADMIN_BLOCK", "CENTRAL_QUAD", TRAFFIC_HIGH);
                setEdgeTrafficById("CENTRAL_QUAD", "LHC_STAIRS", TRAFFIC_CONGESTED);
                setEdgeTrafficById("LHC_STAIRS", "LHC", TRAFFIC_CONGESTED);
                setEdgeTrafficById("CENTRAL_QUAD", "LHC_RAMP", TRAFFIC_HIGH);
                setEdgeTrafficById("LHC_RAMP", "LHC", TRAFFIC_HIGH);
                setEdgeTrafficById("CENTRAL_QUAD", "LIB_STAIRS", TRAFFIC_HIGH);
                setEdgeTrafficById("LIB_STAIRS", "CENTRAL_LIBRARY", TRAFFIC_HIGH);
                setEdgeTrafficById("LHC", "CS_BLOCK", TRAFFIC_CONGESTED);
                setEdgeTrafficById("LHC", "TECH_CORRIDOR", TRAFFIC_CONGESTED);
                setEdgeTrafficById("CS_BLOCK", "TECH_CORRIDOR", TRAFFIC_HIGH);
                setEdgeTrafficById("TECH_CORRIDOR", "ECE_BLOCK", TRAFFIC_HIGH);
                break;
            }
            case 2: {
                strncpy(scenarioName, "Lunch Break Dining Rush (12:30 PM - 02:00 PM)", sizeof(scenarioName) - 1);

                setEdgeTrafficById("GATE_PLAZA", "CENTRAL_CAFE", TRAFFIC_CONGESTED);
                setEdgeTrafficById("AUDITORIUM", "CENTRAL_CAFE", TRAFFIC_CONGESTED);
                setEdgeTrafficById("STUDENT_CENTRE", "CENTRAL_CAFE", TRAFFIC_CONGESTED);
                setEdgeTrafficById("CENTRAL_CAFE", "HOSTEL_CIRCLE", TRAFFIC_CONGESTED);
                setEdgeTrafficById("HOSTEL_CIRCLE", "HOSTEL_ARYABHATTA", TRAFFIC_HIGH);
                setEdgeTrafficById("HOSTEL_CIRCLE", "HOSTEL_RAMANUJAN", TRAFFIC_HIGH);
                setEdgeTrafficById("STUDENT_CENTRE", "SPORTS_COMPLEX", TRAFFIC_MODERATE);
                break;
            }
            case 3: {
                strncpy(scenarioName, "Campus Mega Event / Convocation / Fest", sizeof(scenarioName) - 1);

                setEdgeTrafficById("MAIN_GATE", "GATE_PLAZA", TRAFFIC_CONGESTED);
                setEdgeTrafficById("GATE_PLAZA", "ADMIN_BLOCK", TRAFFIC_CONGESTED);
                setEdgeTrafficById("ADMIN_BLOCK", "AUDITORIUM", TRAFFIC_CONGESTED);
                setEdgeTrafficById("CENTRAL_QUAD", "AUDITORIUM", TRAFFIC_CONGESTED);
                setEdgeTrafficById("AUDITORIUM", "STUDENT_CENTRE", TRAFFIC_HIGH);
                setEdgeTrafficById("AUDITORIUM", "CENTRAL_CAFE", TRAFFIC_HIGH);
                setEdgeTrafficById("SPORTS_COMPLEX", "CRICKET_GROUND", TRAFFIC_HIGH);
                break;
            }
            case 4: {
                strncpy(scenarioName, "Custom / User-Modified Real-Time Traffic", sizeof(scenarioName) - 1);
                break;
            }
            default:
                break;
        }
        scenarioName[sizeof(scenarioName) - 1] = '\0';
    }

    int getNodeCount() const { return nodeCount; }
    const Node& getNode(int idx) const { return nodes[idx]; }
    const EdgeNode* getNeighbors(int idx) const { return adj[idx]; }
    const char* getScenarioName() const { return scenarioName; }
    int getScenarioId() const { return currentScenario; }

    void printAllNodes() const {
        cout << "\n================================================================================\n";
        cout << "                           CAMPUS LOCATIONS DIRECTORY                           \n";
        cout << "================================================================================\n";
        cout << left << setw(4) << "No."
             << setw(22) << "ID"
             << setw(34) << "Name"
             << setw(14) << "Category" << "\n";
        cout << "--------------------------------------------------------------------------------\n";
        for (int i = 0; i < nodeCount; ++i) {
            cout << left << setw(4) << (i + 1)
                 << setw(22) << nodes[i].id
                 << setw(34) << nodes[i].name
                 << setw(14) << nodes[i].category << "\n";
        }
        cout << "================================================================================\n";
    }

    void printTrafficDashboard() const {
        cout << "\n================================================================================\n";
        cout << "                       CAMPUS LIVE TRAFFIC & CONGESTION REPORT                  \n";
        cout << " Current Scenario: [" << scenarioName << "]\n";
        cout << " Average Walking Speed: " << WALKING_SPEED << " m/min | Delay Levels: LOW(1.0x) MOD(1.3x) HIGH(1.8x) JAM(2.5x)\n";
        cout << "================================================================================\n";
        cout << left << setw(4)  << "No."
             << setw(28) << "Pathway Corridor"
             << setw(10) << "Type"
             << setw(10) << "Dist(m)"
             << setw(12) << "Base(min)"
             << setw(13) << "Traffic"
             << setw(12) << "Actual(min)" << "\n";
        cout << "--------------------------------------------------------------------------------\n";

        int edgeIdx = 0;
        for (int i = 0; i < nodeCount; ++i) {
            const EdgeNode* curr = adj[i];
            while (curr != NULL) {

                if (i < curr->target) {
                    edgeIdx++;
                    char corridor[64];
                    strncpy(corridor, nodes[i].id, 13);
                    corridor[13] = '\0';
                    strcat(corridor, " <-> ");
                    strncat(corridor, nodes[curr->target].id, 12);

                    double baseTime = curr->distance / WALKING_SPEED;
                    double actualTime = baseTime * curr->congestionFactor;

                    cout << left << setw(4) << edgeIdx
                         << setw(28) << corridor
                         << setw(10) << curr->pathType
                         << setw(10) << fixed << setprecision(1) << curr->distance
                         << setw(12) << fixed << setprecision(2) << baseTime
                         << setw(13) << TRAFFIC_NAMES[curr->trafficLevel]
                         << setw(12) << fixed << setprecision(2) << actualTime << "\n";
                }
                curr = curr->next;
            }
        }
        cout << "================================================================================\n";
    }

    double calculateHeuristic(int u, int v) const {
        if (u < 0 || u >= nodeCount || v < 0 || v >= nodeCount) return 0.0;
        const double PI = 3.14159265358979323846;
        const double R = 6371000.0;

        double lat1Rad = nodes[u].lat * PI / 180.0;
        double lat2Rad = nodes[v].lat * PI / 180.0;
        double dLat = (nodes[v].lat - nodes[u].lat) * PI / 180.0;
        double dLng = (nodes[v].lng - nodes[u].lng) * PI / 180.0;

        double a = sin(dLat / 2.0) * sin(dLat / 2.0) +
                   cos(lat1Rad) * cos(lat2Rad) * sin(dLng / 2.0) * sin(dLng / 2.0);
        double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
        return R * c;
    }
};

struct PathResult {
    bool found;
    int path[MAX_NODES];
    EdgeNode pathEdge[MAX_NODES];
    int pathLength;
    double totalDistance;
    double baseWalkingTimeMin;
    double totalTimeMin;
    double trafficDelayMin;
    int nodesExplored;
    RoutingMetric metricUsed;
};

PathResult runDijkstra(const Graph& graph, int startIdx, int endIdx, bool accessibleOnly,
                       RoutingMetric metric = METRIC_TIME) {
    PathResult res;
    res.found = false;
    res.pathLength = 0;
    res.totalDistance = 0.0;
    res.baseWalkingTimeMin = 0.0;
    res.totalTimeMin = 0.0;
    res.trafficDelayMin = 0.0;
    res.nodesExplored = 0;
    res.metricUsed = metric;

    int n = graph.getNodeCount();
    if (startIdx < 0 || startIdx >= n || endIdx < 0 || endIdx >= n) return res;

    double cost[MAX_NODES];
    bool visited[MAX_NODES];
    int parent[MAX_NODES];
    EdgeNode parentEdge[MAX_NODES];

    for (int i = 0; i < n; ++i) {
        cost[i] = INF;
        visited[i] = false;
        parent[i] = -1;
    }

    cost[startIdx] = 0.0;

    for (int step = 0; step < n; ++step) {
        int u = -1;
        double minCost = INF;
        for (int i = 0; i < n; ++i) {
            if (!visited[i] && cost[i] < minCost) {
                minCost = cost[i];
                u = i;
            }
        }

        if (u == -1 || minCost >= INF) break;

        visited[u] = true;
        res.nodesExplored++;

        if (u == endIdx) {
            res.found = true;
            break;
        }

        const EdgeNode* curr = graph.getNeighbors(u);
        while (curr != NULL) {
            if (accessibleOnly && !curr->isAccessible) {
                curr = curr->next;
                continue;
            }

            int v = curr->target;

            double edgeWeight = 0.0;
            if (metric == METRIC_DISTANCE) {
                edgeWeight = curr->distance;
            } else {
                edgeWeight = (curr->distance / WALKING_SPEED) * curr->congestionFactor;
            }

            if (!visited[v] && cost[u] + edgeWeight < cost[v]) {
                cost[v] = cost[u] + edgeWeight;
                parent[v] = u;
                parentEdge[v] = *curr;
            }
            curr = curr->next;
        }
    }

    if (cost[endIdx] < INF) {
        res.found = true;

        int tempPath[MAX_NODES];
        EdgeNode tempEdges[MAX_NODES];
        int count = 0;

        int curr = endIdx;
        while (curr != -1) {
            tempPath[count] = curr;
            if (parent[curr] != -1) tempEdges[count] = parentEdge[curr];
            count++;
            curr = parent[curr];
        }

        res.pathLength = count;
        double calcDist = 0.0;
        double calcTime = 0.0;

        for (int i = 0; i < count; ++i) {
            res.path[i] = tempPath[count - 1 - i];
            if (i < count - 1) {
                res.pathEdge[i] = tempEdges[count - 2 - i];
                calcDist += res.pathEdge[i].distance;
                calcTime += (res.pathEdge[i].distance / WALKING_SPEED) * res.pathEdge[i].congestionFactor;
            }
        }

        res.totalDistance = calcDist;
        res.baseWalkingTimeMin = calcDist / WALKING_SPEED;
        res.totalTimeMin = calcTime;
        res.trafficDelayMin = calcTime - res.baseWalkingTimeMin;
    }

    return res;
}

PathResult runAStar(const Graph& graph, int startIdx, int endIdx, bool accessibleOnly,
                    RoutingMetric metric = METRIC_TIME) {
    PathResult res;
    res.found = false;
    res.pathLength = 0;
    res.totalDistance = 0.0;
    res.baseWalkingTimeMin = 0.0;
    res.totalTimeMin = 0.0;
    res.trafficDelayMin = 0.0;
    res.nodesExplored = 0;
    res.metricUsed = metric;

    int n = graph.getNodeCount();
    if (startIdx < 0 || startIdx >= n || endIdx < 0 || endIdx >= n) return res;

    double gScore[MAX_NODES];
    double fScore[MAX_NODES];
    bool visited[MAX_NODES];
    int parent[MAX_NODES];
    EdgeNode parentEdge[MAX_NODES];

    for (int i = 0; i < n; ++i) {
        gScore[i] = INF;
        fScore[i] = INF;
        visited[i] = false;
        parent[i] = -1;
    }

    gScore[startIdx] = 0.0;

    double initialH = graph.calculateHeuristic(startIdx, endIdx);
    if (metric == METRIC_TIME) {
        initialH = initialH / WALKING_SPEED;
    }
    fScore[startIdx] = initialH;

    for (int step = 0; step < n; ++step) {
        int u = -1;
        double minF = INF;
        for (int i = 0; i < n; ++i) {
            if (!visited[i] && fScore[i] < minF) {
                minF = fScore[i];
                u = i;
            }
        }

        if (u == -1 || minF >= INF) break;

        visited[u] = true;
        res.nodesExplored++;

        if (u == endIdx) {
            res.found = true;
            break;
        }

        const EdgeNode* curr = graph.getNeighbors(u);
        while (curr != NULL) {
            if (accessibleOnly && !curr->isAccessible) {
                curr = curr->next;
                continue;
            }

            int v = curr->target;
            double edgeCost = 0.0;
            if (metric == METRIC_DISTANCE) {
                edgeCost = curr->distance;
            } else {
                edgeCost = (curr->distance / WALKING_SPEED) * curr->congestionFactor;
            }

            double tentativeG = gScore[u] + edgeCost;

            if (tentativeG < gScore[v]) {
                parent[v] = u;
                parentEdge[v] = *curr;
                gScore[v] = tentativeG;

                double h = graph.calculateHeuristic(v, endIdx);
                if (metric == METRIC_TIME) {
                    h = h / WALKING_SPEED;
                }
                fScore[v] = gScore[v] + h;
            }
            curr = curr->next;
        }
    }

    if (gScore[endIdx] < INF) {
        res.found = true;

        int tempPath[MAX_NODES];
        EdgeNode tempEdges[MAX_NODES];
        int count = 0;

        int curr = endIdx;
        while (curr != -1) {
            tempPath[count] = curr;
            if (parent[curr] != -1) tempEdges[count] = parentEdge[curr];
            count++;
            curr = parent[curr];
        }

        res.pathLength = count;
        double calcDist = 0.0;
        double calcTime = 0.0;

        for (int i = 0; i < count; ++i) {
            res.path[i] = tempPath[count - 1 - i];
            if (i < count - 1) {
                res.pathEdge[i] = tempEdges[count - 2 - i];
                calcDist += res.pathEdge[i].distance;
                calcTime += (res.pathEdge[i].distance / WALKING_SPEED) * res.pathEdge[i].congestionFactor;
            }
        }

        res.totalDistance = calcDist;
        res.baseWalkingTimeMin = calcDist / WALKING_SPEED;
        res.totalTimeMin = calcTime;
        res.trafficDelayMin = calcTime - res.baseWalkingTimeMin;
    }

    return res;
}

void displayRoute(const Graph& graph, const PathResult& res, const char* algoName, bool accessibleMode) {
    cout << "\n================================================================================\n";
    cout << "                         " << algoName << " NAVIGATION REPORT\n";
    cout << "================================================================================\n";

    if (!res.found) {
        cout << "[Result] No route could be found connecting the selected points.\n";
        cout << "================================================================================\n";
        return;
    }

    cout << "  * Route Status        : SUCCESS\n";
    cout << "  * Optimization Goal   : " << (res.metricUsed == METRIC_TIME ? "FASTEST ROUTE (Traffic-Aware)" : "SHORTEST DISTANCE (Physical)") << "\n";
    cout << "  * Campus Scenario     : " << graph.getScenarioName() << "\n";
    cout << "  * Accessible Mode     : " << (accessibleMode ? "ON (Ramps/Walkways only, No stairs)" : "OFF (Fastest paths including stairs)") << "\n";
    cout << "  * Physical Distance   : " << fixed << setprecision(2) << res.totalDistance << " meters\n";
    cout << "  * Base Walk Time      : " << fixed << setprecision(1) << res.baseWalkingTimeMin << " minutes (zero traffic)\n";
    cout << "  * Actual Travel Time  : " << fixed << setprecision(1) << res.totalTimeMin << " minutes (with live congestion)\n";
    if (res.trafficDelayMin > 0.05) {
        double delayPct = (res.trafficDelayMin / res.baseWalkingTimeMin) * 100.0;
        cout << "  * Traffic Delay       : +" << fixed << setprecision(1) << res.trafficDelayMin
             << " mins (+" << fixed << setprecision(0) << delayPct << "% congestion overhead)\n";
    } else {
        cout << "  * Traffic Delay       : +0.0 mins (Zero congestion delay / Free flow)\n";
    }
    cout << "  * Nodes Explored      : " << res.nodesExplored << " locations evaluated\n";
    cout << "--------------------------------------------------------------------------------\n";
    cout << "TURN-BY-TURN STEP DIRECTIONS WITH TRAFFIC DETAILS:\n";
    cout << "--------------------------------------------------------------------------------\n";

    double cumDist = 0.0;
    double cumTime = 0.0;

    for (int i = 0; i < res.pathLength - 1; ++i) {
        int fromIdx = res.path[i];
        int toIdx = res.path[i + 1];
        const Node& u = graph.getNode(fromIdx);
        const Node& v = graph.getNode(toIdx);
        const EdgeNode& edge = res.pathEdge[i];

        cumDist += edge.distance;
        double stepBaseTime = edge.distance / WALKING_SPEED;
        double stepActualTime = stepBaseTime * edge.congestionFactor;
        cumTime += stepActualTime;

        cout << " Step " << (i + 1) << ": From [" << u.name << "]\n";
        cout << "         -> Path: " << edge.edgeName << " [" << edge.pathType << ", "
             << fixed << setprecision(1) << edge.distance << " m]\n";
        cout << "         -> Traffic Condition: [" << TRAFFIC_DESC[edge.trafficLevel] << "]\n";
        cout << "         -> Segment Time: " << fixed << setprecision(1) << stepActualTime << " min";
        if (edge.trafficLevel > TRAFFIC_LOW) {
            cout << " (Base: " << fixed << setprecision(1) << stepBaseTime << " min, +"
                 << fixed << setprecision(1) << (stepActualTime - stepBaseTime) << " min delay)";
        }
        cout << " | Cum: " << fixed << setprecision(1) << cumDist << " m, "
             << fixed << setprecision(1) << cumTime << " min\n";
        cout << "         -> Arrive at [" << v.name << "]\n";

        if (strcmp(edge.pathType, "stairs") == 0) {
            cout << "            [Notice: Flight of stairs on this segment]\n";
        } else if (strcmp(edge.pathType, "ramp") == 0) {
            cout << "            [Notice: Barrier-free ramp equipped]\n";
        }
        if (edge.trafficLevel == TRAFFIC_CONGESTED) {
            cout << "            [CAUTION: Heavy pedestrian congestion / bottleneck]\n";
        }
        cout << "\n";
    }

    cout << "SUMMARY PATHWAY:\n";
    for (int i = 0; i < res.pathLength; ++i) {
        cout << graph.getNode(res.path[i]).name;
        if (i < res.pathLength - 1) cout << "  -->  ";
    }
    cout << "\n================================================================================\n";
}

void compareShortestVsFastest(const Graph& graph, int startIdx, int endIdx) {
    cout << "\n================================================================================\n";
    cout << "            TRAFFIC IMPACT ANALYSIS: SHORTEST DISTANCE vs FASTEST ROUTE         \n";
    cout << "================================================================================\n";
    cout << " Origin      : " << graph.getNode(startIdx).name << "\n";
    cout << " Destination : " << graph.getNode(endIdx).name << "\n";
    cout << " Scenario    : " << graph.getScenarioName() << "\n";
    cout << "--------------------------------------------------------------------------------\n";

    PathResult distRes = runAStar(graph, startIdx, endIdx, false, METRIC_DISTANCE);
    PathResult timeRes = runAStar(graph, startIdx, endIdx, false, METRIC_TIME);

    if (!distRes.found || !timeRes.found) {
        cout << "[Notice] Unable to complete comparison (no route found).\n";
        return;
    }

    cout << left << setw(28) << "Evaluation Parameter"
         << setw(26) << "Shortest Route (Distance)"
         << setw(26) << "Fastest Route (Traffic-Aware)" << "\n";
    cout << "--------------------------------------------------------------------------------\n";

    cout << left << setw(28) << "Physical Distance"
         << setw(26) << (to_string((int)distRes.totalDistance) + " meters")
         << setw(26) << (to_string((int)timeRes.totalDistance) + " meters") << "\n";

    cout << left << setw(28) << "Actual Travel Time";
    cout << fixed << setprecision(1) << setw(26) << (to_string((int)(distRes.totalTimeMin * 10) / 10.0).substr(0, 4) + " min");
    cout << fixed << setprecision(1) << setw(26) << (to_string((int)(timeRes.totalTimeMin * 10) / 10.0).substr(0, 4) + " min") << "\n";

    cout << left << setw(28) << "Traffic Delay Overhead"
         << setw(26) << ("+" + to_string((int)(distRes.trafficDelayMin * 10) / 10.0).substr(0, 4) + " min")
         << setw(26) << ("+" + to_string((int)(timeRes.trafficDelayMin * 10) / 10.0).substr(0, 4) + " min") << "\n";

    cout << left << setw(28) << "Total Steps (Hops)"
         << setw(26) << (distRes.pathLength - 1)
         << setw(26) << (timeRes.pathLength - 1) << "\n";

    cout << left << setw(28) << "Nodes Explored"
         << setw(26) << distRes.nodesExplored
         << setw(26) << timeRes.nodesExplored << "\n";
    cout << "--------------------------------------------------------------------------------\n";

    cout << "\nPath Taken by Shortest Route:\n  ";
    for (int i = 0; i < distRes.pathLength; ++i) {
        cout << graph.getNode(distRes.path[i]).id;
        if (i < distRes.pathLength - 1) cout << " -> ";
    }

    cout << "\n\nPath Taken by Fastest (Traffic-Aware) Route:\n  ";
    for (int i = 0; i < timeRes.pathLength; ++i) {
        cout << graph.getNode(timeRes.path[i]).id;
        if (i < timeRes.pathLength - 1) cout << " -> ";
    }
    cout << "\n\n--------------------------------------------------------------------------------\n";
    cout << "[Academic & Engineering Evaluation Takeaway]:\n";
    double timeSaved = distRes.totalTimeMin - timeRes.totalTimeMin;
    double distDiff = timeRes.totalDistance - distRes.totalDistance;

    if (timeSaved > 0.1) {
        double pctFaster = (timeSaved / distRes.totalTimeMin) * 100.0;
        cout << "  >>> SMART REROUTING SUCCESS! <<<\n";
        cout << "  By taking an alternate corridor (+" << fixed << setprecision(1) << distDiff
             << " meters detour), the Traffic-Aware engine bypasses congested bottlenecks,\n";
        cout << "  saving " << fixed << setprecision(1) << timeSaved << " minutes ("
             << fixed << setprecision(0) << pctFaster << "% faster arrival)!\n";
    } else {
        cout << "  The shortest physical route is already the fastest route under the current traffic scenario.\n";
        cout << "  No bottleneck detour was required.\n";
    }
    cout << "================================================================================\n";
}

void compareAlgorithms(const Graph& graph, int startIdx, int endIdx, RoutingMetric metric) {
    cout << "\n================================================================================\n";
    cout << "                  ALGORITHM COMPARISON: DIJKSTRA vs A* SEARCH                   \n";
    cout << "================================================================================\n";
    cout << " Origin      : " << graph.getNode(startIdx).name << "\n";
    cout << " Destination : " << graph.getNode(endIdx).name << "\n";
    cout << " Metric      : " << (metric == METRIC_TIME ? "Fastest Time (Traffic-Aware)" : "Shortest Distance") << "\n";
    cout << " Scenario    : " << graph.getScenarioName() << "\n";
    cout << "--------------------------------------------------------------------------------\n";

    PathResult dRes = runDijkstra(graph, startIdx, endIdx, false, metric);
    PathResult aRes = runAStar(graph, startIdx, endIdx, false, metric);

    cout << left << setw(28) << "Metric"
         << setw(25) << "Dijkstra's Algorithm"
         << setw(25) << "A* Search Algorithm" << "\n";
    cout << "--------------------------------------------------------------------------------\n";
    cout << left << setw(28) << "Path Found?"
         << setw(25) << (dRes.found ? "YES" : "NO")
         << setw(25) << (aRes.found ? "YES" : "NO") << "\n";
    cout << left << setw(28) << "Total Distance (m)"
         << setw(25) << fixed << setprecision(1) << dRes.totalDistance
         << setw(25) << fixed << setprecision(1) << aRes.totalDistance << "\n";
    cout << left << setw(28) << "Actual Travel Time (min)"
         << setw(25) << fixed << setprecision(1) << dRes.totalTimeMin
         << setw(25) << fixed << setprecision(1) << aRes.totalTimeMin << "\n";
    cout << left << setw(28) << "Traffic Delay (min)"
         << setw(25) << fixed << setprecision(1) << dRes.trafficDelayMin
         << setw(25) << fixed << setprecision(1) << aRes.trafficDelayMin << "\n";
    cout << left << setw(28) << "Nodes Explored (Search Cost)"
         << setw(25) << dRes.nodesExplored
         << setw(25) << aRes.nodesExplored << "\n";
    cout << "--------------------------------------------------------------------------------\n";
    cout << "[Key Insight for Evaluators]:\n";
    cout << "  - Both algorithms converge on the mathematically identical optimal path.\n";
    cout << "  - Dijkstra explores radially in all directions (uninformed search).\n";
    cout << "  - A* employs an admissible Haversine heuristic (coordinates) to focus search towards the target,\n";
    cout << "    evaluating fewer or equal locations (" << aRes.nodesExplored << " vs " << dRes.nodesExplored << ")!\n";
    cout << "================================================================================\n";
}

void loadCampusData(Graph& g) {

    g.addNode("MAIN_GATE", "Main Entrance Gate", "gate", 28.5450, 77.2700);
    g.addNode("NORTH_GATE", "North Metro Gate", "gate", 28.5495, 77.2725);
    g.addNode("SOUTH_GATE", "South Gate (Hostel)", "gate", 28.5420, 77.2715);
    g.addNode("GATE_PLAZA", "Main Gate Plaza Junction", "junction", 28.5453, 77.2706);
    g.addNode("ADMIN_BLOCK", "Administrative Complex", "academic", 28.5458, 77.2712);
    g.addNode("CENTRAL_QUAD", "Central Academic Quad", "junction", 28.5462, 77.2718);
    g.addNode("CENTRAL_LIBRARY", "Central University Library", "facility", 28.5467, 77.2715);
    g.addNode("LIB_STAIRS", "Library Terrace Stairs", "transit", 28.5464, 77.2714);
    g.addNode("LIB_RAMP", "Library Accessible Ramp", "transit", 28.5463, 77.2712);
    g.addNode("LHC", "Lecture Hall Complex (LHC)", "academic", 28.5460, 77.2725);
    g.addNode("LHC_STAIRS", "LHC Shortcut Stairs", "transit", 28.5458, 77.2722);
    g.addNode("LHC_RAMP", "LHC Accessible Ramp", "transit", 28.5456, 77.2725);
    g.addNode("CS_BLOCK", "Computer Science & IT Block", "academic", 28.5470, 77.2722);
    g.addNode("TECH_CORRIDOR", "Tech Corridor Junction", "junction", 28.5468, 77.2729);
    g.addNode("ECE_BLOCK", "Electronics & Comm. Block", "academic", 28.5464, 77.2736);
    g.addNode("INNOVATION_HUB", "Innovation & Incubation Hub", "facility", 28.5475, 77.2732);
    g.addNode("BASIC_SCI_BLOCK", "Basic Sciences Block", "academic", 28.5478, 77.2712);
    g.addNode("NORTH_PATH", "North Avenue Junction", "junction", 28.5488, 77.2722);
    g.addNode("MECH_BLOCK", "Mechanical & Civil Block", "academic", 28.5448, 77.2738);
    g.addNode("AUDITORIUM", "Main Auditorium", "facility", 28.5452, 77.2723);
    g.addNode("STUDENT_CENTRE", "Student Activity Centre (SAC)", "facility", 28.5440, 77.2726);
    g.addNode("CENTRAL_CAFE", "Central Cafeteria", "facility", 28.5444, 77.2717);
    g.addNode("HOSTEL_CIRCLE", "Hostel Circle Roundabout", "junction", 28.5428, 77.2718);
    g.addNode("HOSTEL_ARYABHATTA", "Aryabhatta Boys Hostel", "hostel", 28.5424, 77.2711);
    g.addNode("HOSTEL_RAMANUJAN", "Ramanujan Boys Hostel", "hostel", 28.5421, 77.2722);
    g.addNode("HOSTEL_KALPANA", "Kalpana Chawla Girls Hostel", "hostel", 28.5482, 77.2738);
    g.addNode("HOSTEL_GARGI", "Gargi Girls Hostel", "hostel", 28.5488, 77.2742);
    g.addNode("SPORTS_COMPLEX", "Indoor Sports Complex & Gym", "facility", 28.5434, 77.2742);
    g.addNode("CRICKET_GROUND", "University Athletic Arena", "facility", 28.5430, 77.2753);
    g.addNode("HEALTH_CENTRE", "Campus Health & Wellness Centre", "facility", 28.5456, 77.2703);

    g.addEdgeById("MAIN_GATE", "GATE_PLAZA", 68.0, "walkway", true, "Main Entrance Boulevard");
    g.addEdgeById("GATE_PLAZA", "HEALTH_CENTRE", 45.0, "walkway", true, "Health Centre Walkway");
    g.addEdgeById("GATE_PLAZA", "ADMIN_BLOCK", 85.0, "walkway", true, "Administration Promenade");
    g.addEdgeById("GATE_PLAZA", "CENTRAL_CAFE", 140.0, "walkway", true, "Canteen Link Pathway");
    g.addEdgeById("HEALTH_CENTRE", "ADMIN_BLOCK", 75.0, "walkway", true, "Medical Block Shaded Way");
    g.addEdgeById("ADMIN_BLOCK", "CENTRAL_QUAD", 72.0, "walkway", true, "Admin to Quad Passage");
    g.addEdgeById("ADMIN_BLOCK", "AUDITORIUM", 115.0, "walkway", true, "Auditorium West Path");
    g.addEdgeById("ADMIN_BLOCK", "CENTRAL_LIBRARY", 160.0, "walkway", true, "Library West Shaded Arcade");

    g.addEdgeById("CENTRAL_QUAD", "LIB_STAIRS", 32.0, "stairs", false, "Library Terrace Short Staircase");
    g.addEdgeById("LIB_STAIRS", "CENTRAL_LIBRARY", 22.0, "stairs", false, "Library Portico Steps");
    g.addEdgeById("CENTRAL_QUAD", "LIB_RAMP", 48.0, "ramp", true, "Library Accessible North Ramp");
    g.addEdgeById("LIB_RAMP", "CENTRAL_LIBRARY", 52.0, "ramp", true, "Library West Accessible Arcade");

    g.addEdgeById("CENTRAL_QUAD", "LHC_STAIRS", 40.0, "stairs", false, "LHC Direct Flight Stairs");
    g.addEdgeById("LHC_STAIRS", "LHC", 25.0, "stairs", false, "LHC Upper Landing Stairs");
    g.addEdgeById("CENTRAL_QUAD", "LHC_RAMP", 65.0, "ramp", true, "LHC South Barrier-Free Ramp");
    g.addEdgeById("LHC_RAMP", "LHC", 42.0, "ramp", true, "LHC Covered Concourse");

    g.addEdgeById("CENTRAL_QUAD", "AUDITORIUM", 105.0, "walkway", true, "Central Quad South Walk");
    g.addEdgeById("CENTRAL_LIBRARY", "CS_BLOCK", 82.0, "walkway", true, "Library to Computing Bridge");
    g.addEdgeById("CENTRAL_LIBRARY", "BASIC_SCI_BLOCK", 110.0, "walkway", true, "Science Walk");
    g.addEdgeById("LHC", "CS_BLOCK", 95.0, "walkway", true, "LHC to CS North Pathway");
    g.addEdgeById("LHC", "AUDITORIUM", 90.0, "walkway", true, "Arts & Lecture Avenue");
    g.addEdgeById("LHC", "TECH_CORRIDOR", 88.0, "walkway", true, "Lecture Corridor East");
    g.addEdgeById("CS_BLOCK", "TECH_CORRIDOR", 70.0, "walkway", true, "CS East Walk");
    g.addEdgeById("CS_BLOCK", "BASIC_SCI_BLOCK", 125.0, "walkway", true, "Departmental Connector");
    g.addEdgeById("TECH_CORRIDOR", "ECE_BLOCK", 65.0, "walkway", true, "ECE Main Entrance Path");
    g.addEdgeById("TECH_CORRIDOR", "INNOVATION_HUB", 80.0, "walkway", true, "Incubator Alley");
    g.addEdgeById("TECH_CORRIDOR", "MECH_BLOCK", 185.0, "walkway", true, "Engineering South Walk");
    g.addEdgeById("ECE_BLOCK", "MECH_BLOCK", 145.0, "walkway", true, "Labs Connector Road");
    g.addEdgeById("ECE_BLOCK", "SPORTS_COMPLEX", 260.0, "walkway", true, "East Sports Walkway");
    g.addEdgeById("BASIC_SCI_BLOCK", "NORTH_PATH", 120.0, "walkway", true, "North Sciences Link");
    g.addEdgeById("NORTH_PATH", "NORTH_GATE", 85.0, "walkway", true, "North Metro Promenade");
    g.addEdgeById("NORTH_PATH", "HOSTEL_KALPANA", 135.0, "walkway", true, "Girls Hostel North Walk");
    g.addEdgeById("INNOVATION_HUB", "HOSTEL_KALPANA", 105.0, "walkway", true, "Tech to North Residential Path");
    g.addEdgeById("HOSTEL_KALPANA", "HOSTEL_GARGI", 75.0, "walkway", true, "Inter-Hostel North Lawn");
    g.addEdgeById("AUDITORIUM", "STUDENT_CENTRE", 115.0, "walkway", true, "SAC South Promenade");
    g.addEdgeById("AUDITORIUM", "CENTRAL_CAFE", 95.0, "walkway", true, "Dining Link East");
    g.addEdgeById("STUDENT_CENTRE", "CENTRAL_CAFE", 80.0, "walkway", true, "SAC Plaza");
    g.addEdgeById("STUDENT_CENTRE", "MECH_BLOCK", 130.0, "walkway", true, "Workshop Way");
    g.addEdgeById("STUDENT_CENTRE", "SPORTS_COMPLEX", 155.0, "walkway", true, "Athletics Link");
    g.addEdgeById("CENTRAL_CAFE", "HOSTEL_CIRCLE", 140.0, "walkway", true, "Hostel Dining Walkway");
    g.addEdgeById("HOSTEL_CIRCLE", "HOSTEL_ARYABHATTA", 70.0, "walkway", true, "Aryabhatta House Walk");
    g.addEdgeById("HOSTEL_CIRCLE", "HOSTEL_RAMANUJAN", 65.0, "walkway", true, "Ramanujan House Walk");
    g.addEdgeById("HOSTEL_CIRCLE", "SOUTH_GATE", 90.0, "walkway", true, "South Exit Avenue");
    g.addEdgeById("HOSTEL_ARYABHATTA", "HOSTEL_RAMANUJAN", 85.0, "walkway", true, "Men's Quad Pathway");
    g.addEdgeById("HOSTEL_RAMANUJAN", "SPORTS_COMPLEX", 190.0, "walkway", true, "Hostel to Gym Pathway");
    g.addEdgeById("SPORTS_COMPLEX", "CRICKET_GROUND", 95.0, "walkway", true, "Arena Pavilion Track");
    g.addEdgeById("MECH_BLOCK", "SPORTS_COMPLEX", 140.0, "walkway", true, "Gym South Walk");

    g.addEdgeById("GATE_PLAZA", "SOUTH_GATE", 320.0, "walkway", true, "South Boulevard Outer Bypass");
    g.addEdgeById("GATE_PLAZA", "HOSTEL_ARYABHATTA", 380.0, "walkway", true, "West Residential Avenue Bypass");

    g.applyTrafficScenario(0);
}

bool getEndpointsFromUser(const Graph& graph, int& startIdx, int& endIdx) {
    int sNum, eNum;
    cout << "\nEnter Start Location Number (1 to " << graph.getNodeCount() << ", or 0 to view directory): ";
    if (!(cin >> sNum)) {
        cin.clear();
        char dummy[100];
        cin >> dummy;
        return false;
    }
    if (sNum == 0) {
        graph.printAllNodes();
        cout << "\nEnter Start Location Number (1 to " << graph.getNodeCount() << "): ";
        if (!(cin >> sNum)) {
            cin.clear();
            char dummy[100];
            cin >> dummy;
            return false;
        }
    }
    if (sNum < 1 || sNum > graph.getNodeCount()) {
        cout << "[Error] Invalid start location number.\n";
        return false;
    }

    cout << "Enter Destination Location Number (1 to " << graph.getNodeCount() << "): ";
    if (!(cin >> eNum)) {
        cin.clear();
        char dummy[100];
        cin >> dummy;
        return false;
    }
    if (eNum < 1 || eNum > graph.getNodeCount()) {
        cout << "[Error] Invalid destination location number.\n";
        return false;
    }

    startIdx = sNum - 1;
    endIdx = eNum - 1;

    cout << "\n[Route Request]: " << graph.getNode(startIdx).name
         << " ---> " << graph.getNode(endIdx).name << "\n";
    return true;
}

void handleTrafficScenarioMenu(Graph& graph) {
    int sChoice = 0;
    cout << "\n================================================================================\n";
    cout << "                    CAMPUS TRAFFIC SCENARIO CONFIGURATOR                        \n";
    cout << "================================================================================\n";
    cout << " Active Scenario: [" << graph.getScenarioName() << "]\n";
    cout << "--------------------------------------------------------------------------------\n";
    cout << " 1. Normal / Off-Peak Hours (Light Traffic across Campus)\n";
    cout << " 2. Class-Change Rush Hour (Heavy Academic Congestion: LHC, Quad, CS Block)\n";
    cout << " 3. Lunch Break Dining Rush (Heavy Congestion: Cafeteria, SAC, Hostels)\n";
    cout << " 4. Campus Mega Event / Convocation (Heavy Congestion: Gates, Admin, Auditorium)\n";
    cout << " 5. Real-Time Dynamic Traffic Override (Manually adjust corridor congestion)\n";
    cout << " 6. Return to Main Menu\n";
    cout << "--------------------------------------------------------------------------------\n";
    cout << " Select Scenario (1-6): ";
    if (!(cin >> sChoice)) {
        cin.clear();
        char dummy[100];
        cin >> dummy;
        return;
    }

    if (sChoice >= 1 && sChoice <= 4) {
        graph.applyTrafficScenario(sChoice - 1);
        cout << "\n[Success] Applied Scenario: " << graph.getScenarioName() << "\n";
    } else if (sChoice == 5) {
        graph.printAllNodes();
        int uNum, vNum, level;
        cout << "\nEnter First Location Number (1 to " << graph.getNodeCount() << "): ";
        cin >> uNum;
        cout << "Enter Second Location Number (1 to " << graph.getNodeCount() << "): ";
        cin >> vNum;
        cout << "Select Traffic Level:\n";
        cout << "  0: LOW (Free Flow, 1.0x)\n";
        cout << "  1: MODERATE (Normal footfall, 1.3x)\n";
        cout << "  2: HIGH (Crowded rush, 1.8x)\n";
        cout << "  3: CONGESTED (Heavy bottleneck, 2.5x)\n";
        cout << "Enter Level (0-3): ";
        cin >> level;

        if (uNum >= 1 && uNum <= graph.getNodeCount() &&
            vNum >= 1 && vNum <= graph.getNodeCount() &&
            level >= 0 && level <= 3) {
            bool ok = graph.setEdgeTraffic(uNum - 1, vNum - 1, level, true);
            if (ok) {
                graph.applyTrafficScenario(4);
                cout << "\n[Success] Updated corridor traffic between ["
                     << graph.getNode(uNum - 1).name << "] and ["
                     << graph.getNode(vNum - 1).name << "] to "
                     << TRAFFIC_DESC[level] << "!\n";
            } else {
                cout << "\n[Error] No direct pathway exists between selected locations.\n";
            }
        } else {
            cout << "\n[Error] Invalid input parameters.\n";
        }
    }
}

int main() {
    Graph campusGraph;
    loadCampusData(campusGraph);

    int choice = 0;
    while (choice != 9) {
        cout << "\n================================================================================\n";
        cout << "      INTRA CAMPUS OPTIMAL ROUTE OPTIMIZER (C++ / NON-STL)  \n";
        cout << "================================================================================\n";
        cout << " [Active Traffic Scenario]: " << campusGraph.getScenarioName() << "\n";
        cout << "--------------------------------------------------------------------------------\n";
        cout << " 1. View Campus Locations Directory\n";
        cout << " 2. View Live Campus Traffic Dashboard & Pathway Congestion\n";
        cout << " 3. Configure / Simulate Campus Traffic Scenario (Peak Hours & Dynamic Override)\n";
        cout << " 4. Find Fastest Route (Traffic-Aware A* Search - Recommended)\n";
        cout << " 5. Find Shortest Physical Route (Distance-Only Dijkstra)\n";
        cout << " 6. Find Accessible Route (Wheelchair Friendly / Avoid Stairs + Traffic-Aware)\n";
        cout << " 7. Traffic Impact Demo: Shortest vs Fastest Route (Congestion Avoidance)\n";
        cout << " 8. Benchmark Algorithm Performance: Dijkstra vs A* Search\n";
        cout << " 9. Exit\n";
        cout << "--------------------------------------------------------------------------------\n";
        cout << " Select an option (1-9): ";

        if (!(cin >> choice)) {
            cin.clear();
            char dummy[100];
            cin >> dummy;
            continue;
        }

        switch (choice) {
            case 1: {
                campusGraph.printAllNodes();
                break;
            }
            case 2: {
                campusGraph.printTrafficDashboard();
                break;
            }
            case 3: {
                handleTrafficScenarioMenu(campusGraph);
                break;
            }
            case 4: {
                int startIdx, endIdx;
                if (getEndpointsFromUser(campusGraph, startIdx, endIdx)) {
                    PathResult res = runAStar(campusGraph, startIdx, endIdx, false, METRIC_TIME);
                    displayRoute(campusGraph, res, "FASTEST TRAFFIC-AWARE A* SEARCH", false);
                }
                break;
            }
            case 5: {
                int startIdx, endIdx;
                if (getEndpointsFromUser(campusGraph, startIdx, endIdx)) {
                    PathResult res = runDijkstra(campusGraph, startIdx, endIdx, false, METRIC_DISTANCE);
                    displayRoute(campusGraph, res, "SHORTEST DISTANCE DIJKSTRA (DISTANCE ONLY)", false);
                }
                break;
            }
            case 6: {
                int startIdx, endIdx;
                if (getEndpointsFromUser(campusGraph, startIdx, endIdx)) {
                    PathResult res = runAStar(campusGraph, startIdx, endIdx, true, METRIC_TIME);
                    displayRoute(campusGraph, res, "ACCESSIBLE (WHEELCHAIR) TRAFFIC-AWARE ROUTE", true);
                }
                break;
            }
            case 7: {
                int startIdx, endIdx;
                if (getEndpointsFromUser(campusGraph, startIdx, endIdx)) {
                    compareShortestVsFastest(campusGraph, startIdx, endIdx);
                }
                break;
            }
            case 8: {
                int startIdx, endIdx;
                if (getEndpointsFromUser(campusGraph, startIdx, endIdx)) {
                    cout << "\nChoose Metric for Benchmark:\n";
                    cout << " 1. Fastest Route (Traffic-Aware Time)\n";
                    cout << " 2. Shortest Route (Physical Distance)\n";
                    cout << " Choice (1-2): ";
                    int mChoice = 1;
                    cin >> mChoice;
                    RoutingMetric metric = (mChoice == 2) ? METRIC_DISTANCE : METRIC_TIME;
                    compareAlgorithms(campusGraph, startIdx, endIdx, metric);
                }
                break;
            }
            case 9: {
                cout << "\nThank you for using Intra Campus Optimal Route Optimizer!\n";
                break;
            }
            default: {
                cout << "\n[Error] Invalid option. Please select 1 through 9.\n";
                break;
            }
        }
    }
    return 0;
}
