#include "pathing.hpp"
#include <algorithm>

// Comparison funcion for djikstras to find cheapest node.
bool compare_nodes(const Node* a, const Node* b) {
    return a->cost > b->cost;
}

std::ostream& operator<<(std::ostream& os, PathAction c) {
    switch(c)
    {
        case PathAction::LEFT   : os << "LEFT";    break;
        case PathAction::RIGHT: os << "RIGHT"; break;
        case PathAction::STRAIGHT : os << "STRAIGHT";  break;
        case PathAction::NODE  : os << "NODE";   break;
        case PathAction::STOP : os << "STOP"; break;
        default    : os.setstate(std::ios_base::failbit);
    }
    return os;
}

Node* find_path(Node* start, Node* goal) {
    std::vector<Node*> found_nodes;
    start->cost = 0;
    start->prev = nullptr;
    found_nodes.push_back(start);
    while (found_nodes.size() > 0) {
        Node* current = found_nodes.front();
        std::pop_heap(found_nodes.begin(), found_nodes.end(), compare_nodes);
        found_nodes.pop_back();
        if (current == goal) {
            return current;
        }

        for (Edge &edge : current->edges) {
            if (current->cost + edge.cost < edge.end->cost) {
                edge.end->cost = current->cost + edge.cost;
                edge.end->prev = &edge;
                found_nodes.push_back(edge.end);
                std::push_heap(found_nodes.begin(), found_nodes.end(), compare_nodes);
            }
        }
    }
    return nullptr;
}

void parse_path(Node* node, std::vector<std::pair<Node*, PathAction>> &path) {
    const size_t first = path.size();
    Edge* last_edge = nullptr;
    while (node->prev != nullptr) {
        PathAction action;
        switch (node->type) {
            case Node::PICKUP:
                action = PathAction::NODE;
                break;
            case Node::LEFT:
                action = last_edge->type == Edge::MIDDLE ? PathAction::RIGHT : PathAction::STRAIGHT;
                break;
            case Node::RIGHT:
                action = last_edge->type == Edge::MIDDLE ? PathAction::LEFT : PathAction::STRAIGHT;
                break;
            case Node::MIDDLE:
                action = last_edge->type == Edge::LEFT ? PathAction::LEFT : PathAction::RIGHT;
                break;
        }

        path.emplace_back(node, action);
        last_edge = node->prev;
        node = last_edge->start;
    }
    path[first].second = PathAction::STOP;
}

/**
 * Creates an Edge starting at the node with id start and ending at id end, with cost UNEXPLORED_COST and type type.
 */
Edge make_edge(std::vector<Node> &nodes, const char start, const char end, const Edge::EdgeType type) {
    return {type, UNEXPLORED_COST, &nodes[to_index(start)], &nodes[to_index(end)]};
}

void reset_nodes(std::vector<Node> &nodes) {
	for (Node &node : nodes) {
		node.cost = INT_MAX;
	}
}

void create_nodes(std::vector<Node> &nodes) {
    nodes.clear();

    nodes.emplace_back('A');
    nodes.emplace_back('B');
    nodes.emplace_back('C');
    nodes.emplace_back('D');
    nodes.emplace_back('E');
    nodes.emplace_back('F');

    nodes.emplace_back('l', Node::LEFT);
    nodes.emplace_back('r', Node::RIGHT);
    nodes.emplace_back('m', Node::MIDDLE);
    nodes.emplace_back('L', Node::LEFT);
    nodes.emplace_back('R', Node::RIGHT);
    nodes.emplace_back('M', Node::MIDDLE);

    nodes[to_index('A')].edges = {make_edge(nodes, 'A', 'R', Edge::RIGHT)};
    nodes[to_index('B')].edges = {make_edge(nodes, 'B', 'l', Edge::LEFT)};
    nodes[to_index('C')].edges = {make_edge(nodes, 'C', 'L', Edge::LEFT)};
    nodes[to_index('D')].edges = {make_edge(nodes, 'D', 'r', Edge::RIGHT)};
    nodes[to_index('E')].edges = {make_edge(nodes, 'E', 'M', Edge::MIDDLE)};
    nodes[to_index('F')].edges = {make_edge(nodes, 'F', 'm', Edge::MIDDLE)};

    nodes[to_index('l')].edges = {
        make_edge(nodes, 'l', 'E', Edge::MIDDLE),
        make_edge(nodes, 'l', 'C', Edge::RIGHT),
    };
    nodes[to_index('r')].edges = {
        make_edge(nodes, 'r', 'A', Edge::LEFT),
        make_edge(nodes, 'r', 'E', Edge::MIDDLE),
    };
    nodes[to_index('m')].edges = {
        make_edge(nodes, 'm', 'C', Edge::RIGHT),
        make_edge(nodes, 'm', 'A', Edge::LEFT)
    };
    nodes[to_index('L')].edges = {
        make_edge(nodes, 'L', 'F', Edge::MIDDLE),
        make_edge(nodes, 'L', 'B', Edge::RIGHT)
    };
    nodes[to_index('R')].edges = {
        make_edge(nodes, 'R', 'D', Edge::LEFT),
        make_edge(nodes, 'R', 'F', Edge::MIDDLE)
    };
    nodes[to_index('M')].edges = {
        make_edge(nodes, 'M', 'D', Edge::LEFT),
        make_edge(nodes, 'M', 'B', Edge::RIGHT)
    };
}

void set_path_cost(Node* a, Node* b, const int cost) {
    for (Edge &e : a->edges) {
        if (e.end == b) {
            e.cost = cost;
            break;
        } 
    }
    for (Edge &e : b->edges) {
        if (e.end == a) {
            e.cost = cost;
            break;
        } 
    }
}
