#ifndef PATHING_H
#define PATHING_H
#include <vector>
#include <utility>
#include <climits>
#include <iostream>


constexpr int UNEXPLORED_COST = 5;
constexpr int PICKUP_NODES = 6;


/**
 * Gets the index in the nodes vector of the node with id id.
 */
constexpr int to_index(const char id) {
   switch(id) {
        case 'l':
            return PICKUP_NODES;
        case 'r':
            return PICKUP_NODES + 1;
        case 'm':
            return PICKUP_NODES + 2;
        case 'L':
            return PICKUP_NODES + 3;
        case 'R':
            return PICKUP_NODES + 4;
        case 'M':
            return PICKUP_NODES + 5;
        default:
            return static_cast<int>(id - 'A');
   }
}

/*
 * Retruns the position in the track of given node id.
*/
constexpr char to_track_id(const char id) {
    switch(id) {
        case 'r':
        case 'l':
        case 'm':
            return '1';
        case 'R':
        case 'L':
        case 'M':
            return '2';
        default:
            return id;
    }
}

enum class PathAction {
    LEFT, RIGHT, STRAIGHT, NODE, STOP
};

std::ostream& operator<<(std::ostream& os, PathAction c);


struct Node;

struct Edge {
    /*
     * Names from which part of a crossing the edge comes from. 
    */
    enum EdgeType {
        LEFT, RIGHT, MIDDLE
    } type;

    int cost;

    Node* start;
    Node* end;
};

/**
 * A struct for Nodes in the graph representing the track. Each pickup spot is represented as a Node with type = PICKUP.
 * Each crossing is repsressented as 3 Nodes, one for each direction you can enter the crossing from.
 * Crossings have type according to :  L _____ R (rotated to match).
 *                                         |
 *                                         M
 */
struct Node {
    /*
     * Names if a node is a pickup spot of which part of a crossing the node represent.
    */
	enum NodeType {
        PICKUP, LEFT, RIGHT, MIDDLE
    } type = PICKUP;

    Node(const char id) : type(PICKUP), id(id) {}
    Node(const char id, const NodeType type) : type(type), id(id) {}

    std::vector<Edge> edges;

    Edge* prev = nullptr;
    int cost = INT_MAX;
    char id = 'u';
};

/**
 * Finds an optimal path from start to goal using djikstras.
 * Node cost have to be reset to INT_MAX before calling this.
*/
Node* find_path(Node* start, Node* goal);

/**
 * Goes backwards through every node in the path to node, and decides the action to be made there.
 * The actions and nodes are added to path, from start to node, without including start. 
*/
void parse_path(Node* node, std::vector<std::pair<Node*, PathAction>> &path);

/**
 * Sets all node cost to INT_MAX.
 */
void reset_nodes(std::vector<Node> &nodes);



/**
 * Populates the nodes vector with all the nodes and edges making up the track.
 */
void create_nodes(std::vector<Node> &nodes);

/**
 * Updates the cost of traveling from a to b to cost.
*/
void set_path_cost(Node* a, Node* b, const int cost);

#endif