// A C++ program to find strongly connected components in a given
// directed graph using Tarjan's algorithm (single DFS)
#include "libs.h"
using namespace std;

// A class that represents an directed graph
class Graph{
    
public:
    
    Graph(int V);   // Constructor
    int Nedges;
    int Nnodes;  // number of nodes, counted while building the graph
    list<int> *adj;    // A dynamic array of adjacency lists
    void addEdge(int v, int w); // function to add an edge to graph
    void addConstraint(int v, int w, vector<double> Q, vector<double> t); // function to add a constraint to graph
    void SCC();// prints strongly connected components
    void initlialisePose( );
    void QuaternionInit( );
    void QuaternionBoxMedian( );
    void QuaternionRobustMean( );
    
    vector<int> CC; // subgraphs of strongest component
    int SS; // subgraph id of strongest component, S=max(CC)
    vector<int> JJ;
    
    struct graphedge {
        int i;
        int j;
        graphedge(int i,int j):i(i),j(j){};
    }; vector<graphedge> edge;
    
    struct subgraph {
        double vertex;
        double index;
        subgraph(int vertex,int index):vertex(vertex),index(index){};
    }; vector<subgraph> sub;
    
    struct Constraints {
        Constraints ();
        vector<int> I;
        vector<double> QQ;  // rotation quaternion vector
        vector<double> e;  // rotational errors while global solving
        vector<double> t;  // translation vector
        Constraints(vector<int> I,vector<double> QQ,vector<double> e,vector<double> t):I(I),QQ(QQ),e(e),t(t){};
    }; vector<Constraints> C;
    // grab a reference to one of the constraints
    // use : Constraints c=C[k]; vector<double> Q=mi.Q;
    // or simply use : vector<double> Q=C[i].Q;
    
    struct Pose {
        //Pose ();
        vector<double> Q;
        vector<double> t;
        Pose(vector<double> Q,vector<double> t):Q(Q),t(t){};
    }; vector<Pose> M;
    // grab a reference to one of the constraints
    // use : Pose m=M[i]; vector<double> Q=m.Q;
    // or simply use : vector<double> Q=M[i].Q;
    
private:
    
    int V;    // No. of vertices (WRONG: this is number of edges)
    // A Recursive DFS based function used by SCC()
    void SCCUtil(int u, int disc[], int low[], stack<int> *st, bool stackMember[]);
    
    vector<double> RtoQuaternion(vector<double> R);
    vector<double> QuaternionFwd(vector<double> Qi, vector<double> Qij);
    vector<double> QuaternionRvs(vector<double> Qj, vector<double> Qij);
    
    Eigen::MatrixXd l1decode_pd(Eigen::VectorXd x0, SpMat A, Eigen::VectorXd y, SpMat AtA, int pdmaxiter);
    
};
