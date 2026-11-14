# Intra Campus Optimal Route Optimizer

A console-based campus route-planning project written in C++. It models campus locations as a graph and finds useful routes using custom pointer-based adjacency lists (without STL containers).

## Features

- Campus location directory with IDs, names, categories and coordinates.
- Graph pathways represented using a linked-list adjacency list.
- Fastest-route search using traffic-aware A*.
- Shortest physical-route search using Dijkstra's algorithm.
- Accessible route option that avoids stair-only pathways.
- Traffic levels: LOW, MODERATE, HIGH and CONGESTED.
- Predefined traffic scenarios for normal hours, class-change rush, lunch rush and campus events.
- Manual traffic override for a selected corridor.
- Traffic impact demonstration and Dijkstra-vs-A* benchmark.

## Requirements

A C++ compiler supporting C++11 or later (for example, g++).

## Compile

```bash
g++ -std=c++11 -Wall -Wextra -pedantic final_project.cpp -o intra_campus_route_optimizer
```

## Run

On Linux/macOS:

```bash
./intra_campus_route_optimizer
```

On Windows (MinGW):

```bash
g++ -std=c++11 final_project.cpp -o intra_campus_route_optimizer.exe
.\intra_campus_route_optimizer.exe
```

Follow the numbered menu displayed in the terminal. Choose option 9 to exit.

## Main menu

1. View Campus Locations Directory
2. View Live Campus Traffic Dashboard & Pathway Congestion
3. Configure / Simulate Campus Traffic Scenario
4. Find Fastest Route (Traffic-Aware A* Search)
5. Find Shortest Physical Route (Distance-Only Dijkstra)
6. Find Accessible Route (Wheelchair Friendly / Avoid Stairs)
7. Traffic Impact Demo: Shortest vs Fastest Route
8. Benchmark Dijkstra vs A* Search
9. Exit

## Data structures and algorithms

- **Graph:** fixed-size node array plus adjacency lists made from dynamically allocated `EdgeNode` pointers.
- **Dijkstra:** distance-only route optimization.
- **A* search:** traffic-aware route optimization using campus coordinates for its heuristic.
- **Linked lists:** store each node's outgoing pathways.

## Project files

- `final_project.cpp` — complete C++ source code.
- `README.md` — project overview and build/run instructions.
- `output_screenshot.png` — screenshot-style capture of the console output.
- `program_output.txt` — raw console output used for the screenshot.

## Note

The campus locations and coordinates are sample data embedded in the source code. Review or replace them with verified campus data before using this as a real navigation system.
