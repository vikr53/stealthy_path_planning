#include <iostream>
#include <vector>
#include <map>
#include <list>
#include <cmath>
 
class Node {
    public:
        int x;
        int y;
        double priority;
 
    Node() {
        x = 0;
        y = 0;
    }
 
    Node(int xpos, int ypos) {
        x = xpos;
        y = ypos;
    }
 
    bool operator==(const Node& node) const {
        return (node.x == x && node.y == y);
    }
 
    bool operator<(const Node &node) const {
        if ((x + y) < (node.x + node.y)) {
            return true;
        } else if ((x + y) == (node.x + node.y)){
            // x takes priority
            if (x < node.x) {
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }
 
    int getX() {
        return x;
    }
 
    int getY() {
        return y;
    }
};
 
// Enable the printing of Nodes to console
std::ostream& operator<<(std::ostream& os, const Node& n) {
    os << "(" << n.x << "," << n.y << ")";
    return os;
}
 
// Grid class
class Grid {
    public:
        int rows;
        int cols;
        int num_monsters;
        int mov_cost;
        std::vector<Node> nodelist;
        std::vector<Node> monsterLocs;
 
    Grid(int r, int c, int mons, int w) {
        rows = r;
        cols = c;
        num_monsters = mons;
        mov_cost = w;
    }
 
    // Makes a graph to represent the Grid
    // RETURNS a map that associates a Node in the graph, say n, to another map that relates the neighbors of n
    // to their weights
    std::map< Node, std::map<Node,int> > makeGridGraph() {
        std::map< Node, std::map<Node,int> > neighbors;
 
        int monstercount = num_monsters;
        while (monstercount != 0) {
            // Randomly set locations of the monsters - monsters can move so actively change weights of the nodes near it
            srand(time(NULL));
            int x_rand = rand() % (rows - 1);
            int y_rand = rand() % (cols - 1);
 
            if ( !isMonster(Node(x_rand, y_rand)) ) {
                monsterLocs.push_back(Node(x_rand, y_rand));
                std::cout << "Monster " << num_monsters - monstercount + 1 << " Location: " << Node(x_rand, y_rand) << std::endl;
                monstercount--;
            }
        }
 
        // Add Nodes to nodelist and define the neighbors of each node
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                nodelist.push_back(Node(j,i));
 
                // For better readability, xpos and ypos are defined
                int xpos = j;
                int ypos = i;
 
                // Instantiate a vector of possible neighbors of the node
                std::vector<Node> currNeighs{Node(xpos-1, ypos+1), Node(xpos, ypos+1), Node(xpos+1, ypos+1),
                Node(xpos-1, ypos), Node(xpos+1, ypos), Node(xpos-1, ypos-1),
                Node(xpos, ypos-1), Node(xpos+1, ypos-1)};
 
                std::map<Node,int> neighWeights;
 
                for (std::vector<Node>::iterator it = currNeighs.begin(); it != currNeighs.end(); ++it) {
                    // can't have negative x,y positions
                    if ( it -> getX() < 0 || it -> getY() < 0) {
                        it = currNeighs.erase(it);
                        --it;
                    } else if ( (it -> getX()) > (cols - 1) || (it -> getY()) > (rows - 1) ) {
                        it = currNeighs.erase(it);
                        --it;
                    }
                }
 
                // Set the weights associated with the edges between the current node and its neighbors
                for (std::vector<Node>::iterator it = currNeighs.begin(); it != currNeighs.end(); ++it) {
                    Node n = *it;
                    std::pair <std::map<Node,int>::iterator,bool> ptr;
                    ptr = neighWeights.insert(std::pair<Node,int>(n,mov_cost));
 
                    /*DEBUG checking if the key was already present or newly inserted
                    if(ptr.second)
                        std::cout << "The key was newly inserted" ;
                    else
                        std::cout << "The key was already present" ;
 
                    std::cout << std::endl ;*/
                }
 
                neighbors.insert(std::pair< Node,std::map<Node,int> >(Node(xpos,ypos),neighWeights));
            }
 
        }
 
        return neighbors;
 
    }
 
    struct priority_comparator {
        bool operator()(const Node& node1, const Node& node2) {
            return node1.priority < node2.priority;
        }
    };
 
    bool isMonster(Node n) {
        for (std::vector<Node>::iterator it = monsterLocs.begin(); it != monsterLocs.end(); it++ ){
            if (n == *it) {
                return true;
            } else {
                return false;
            }
         }
        return false;
    }
 
    std::vector<Node> shortestPath(Node start, Node end, std::map< Node, std::map<Node,int> > neighbors) {
        //Check if start or end nodes have a monster
        if ( isMonster(start) ) {
            std::cout << "START NODE ALREADY HAS A MONSTER ON IT" << std::endl;
            return {};
        }
        if ( isMonster(end) ) {
            std::cout << "END NODE ALREADY HAS A MONSTER ON IT" << std::endl;
            return {};
        }
 
        std::list <Node> frontier;
        frontier.push_front(start);
 
        std::map<Node,Node> prev_node;
        std::map< Node, int > cost_to_node;
 
        cost_to_node[start] = 0;
 
        while (!frontier.empty()) {
            Node current = frontier.front();
            frontier.pop_front();
 
            // DEBUG std::cout << "FRONTIER NODE UNDER ANALYSIS: " << current << std::endl;
            if ( current == end ) {
                break;
            }
 
            for (std::map<Node,int>::iterator it = neighbors[current].begin(); it != neighbors[current].end(); it++) {
                Node next = it -> first;
                int weight = it -> second;
                int new_cost = cost_to_node[current] + weight;
 
                if ( (cost_to_node.find(next) == cost_to_node.end()) || (new_cost < cost_to_node[next]) ) {
                    cost_to_node[next] = new_cost;
 
                    double h_monstprox = 0.0;
                    double curr_min = std::sqrt( std::pow((rows -1),2) + std::pow((cols -1),2) );
 
                    for (std::vector<Node>::iterator it2 = monsterLocs.begin(); it2 != monsterLocs.end(); it2++) {
                        Node monster = (*it2);
                        double current_mons_dist = std::sqrt( std::pow((next.x - monster.x),2) + std::pow((next.y - monster.y),2) );
                        if (current_mons_dist < curr_min) {
                            curr_min = current_mons_dist;
                        }
                    }
                    h_monstprox = curr_min;
                    double h_endprox = std::sqrt( std::pow((next.x - end.x),2) + std::pow((next.y - end.y),2) );
                    /* DEBUG - find every node's monstprox and endprox scores
                        std::cout << next << "::" << "monstprox: " << h_monstprox << std::endl;
                        std::cout << next << "::" << "endprox: " << h_endprox << std::endl;
                    */
                    double heuristic = h_endprox - h_monstprox;
                    double priority = new_cost + heuristic;
                    next.priority = priority;
                    frontier.push_back(next);
 
                    // Sort by node priority
                    frontier.sort(priority_comparator());
                    prev_node[next] = current;
                }
 
            }
 
            /* DEBUG - print frontier
            for (auto v : frontier)
                std::cout << v << "\n" << std::endl;
            */
 
        }
 
        // Construct path
        // RETURNS path in reverse order
        Node curr_node = end;
        std::vector<Node> path;
        while (!(curr_node == start)) {
            path.push_back(curr_node);
            curr_node = prev_node[curr_node];
        }
        path.push_back(start);
 
        return path;
    }
};
 
int main() {
    int weight = 1;
    int rows = 20;
    int cols = 20;
    int mons = 6;
    Grid grid1(rows,cols,mons,weight);
    std::map< Node, std::map<Node,int> > neighbors = grid1.makeGridGraph();
 
    /* DEBUG - Check neighbors of particular nodes as outputed by makeGridGraph
    for (std::map<Node, int>::iterator it = neighbors[Node(2,1)].begin(); it != neighbors[Node(2,1)].end(); ++it){
        std::cout << "MAIN TEST: " << it -> first << it -> second << std::endl;
    }*/
 
    std::vector<Node> spath = grid1.shortestPath(Node(1,2), Node(18,7), neighbors);
 
    std::cout << "STEALTHY PATH WITH MOVEMENT COST = " << weight << " : " << std::endl;
 
    for (std::vector<Node>::iterator it = spath.begin(); it != spath.end(); ++it) {
        std::cout << *it << std::endl;
    }
 
}
