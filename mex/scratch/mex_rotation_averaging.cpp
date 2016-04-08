#include "../src/graph.cpp"
using namespace std;

/****************************/

/* The gateway function */
void mexFunction( int nlhs, mxArray *plhs[],
        int nrhs, const mxArray *prhs[]){
    
//     /* Check for proper number of arguments */
//     if (nrhs != 1) {
//         mexErrMsgIdAndTxt("MATLAB:triangulate:nargin",
//                 "least-squares requires three input arguments.");}
//     else if (nlhs != 3) {
//         mexErrMsgIdAndTxt("MATLAB:triangulate:nargout",
//                 "least-squares requires four output argument.");}
    
    /* create a pointer to the real data in the input matrix  */
    double *g;   /* constraint edge in the graph */
    g = mxGetPr(prhs[0]);
    double *Qin;   /* Quaternion constraint */
    Qin = mxGetPr(prhs[1]);
    double *tin;   /* Translation constraint */
    tin = mxGetPr(prhs[2]);
    
    /* get dimensions of the input matrix */
    size_t n; /* number of constraints */
    n = mxGetN(prhs[0]);
        
    /****************************/
    
    /* Notice that for this demo, constraints are provided in the form:
     * Q: Quaternion constraint (nx4)
     * t: Translation constraint (nx3)
     * g: List of edges (nx2)
     * Later when implemented in real-time, this wont be the case, but
     * instead, the constraints will be fed into the graph sequentially.
     * The graph is then called for motion averaging whenever needed.
     */
    
    std::clock_t start = std::clock();
    double duration;
    
    /* construct the graph */
    int V = 1, k = 0; // V counts from 1 because nodes counts from 0
    while (k<n){
        V=MAX(V,g[2*k+0]);
        V=MAX(V,g[2*k+1]);
        k++;}
    Graph graph(V); // n is the number of nodes.
    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
    cout << "[RotAvg] Building graph using \t" << V << " nodes.\n";
    
    /* fill in the graph */
    // Graph::addEdge( reference node, adjacent node )
    // C(i,j) are constraints from reference node to adjacent node
    vector<double> t;
    t.assign(3,0);
    vector<double> Q;
    Q.assign(4,0);
    k = 0;
    while (k<n){
        t[0]=tin[0*n+k]; // only for mex, not needed in real-time
        t[1]=tin[1*n+k]; // only for mex, not needed in real-time 
        t[2]=tin[2*n+k]; // only for mex, not needed in real-time
        Q[0]=Qin[0*n+k]; // only for mex, not needed in real-time 
        Q[1]=Qin[1*n+k]; // only for mex, not needed in real-time 
        Q[2]=Qin[2*n+k]; // only for mex, not needed in real-time
        Q[3]=Qin[3*n+k]; // only for mex, not needed in real-time
        graph.addConstraint(g[2*k+0],g[2*k+1],Q,t);
        k++;}
    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
    cout << "[RotAvg] " << graph.C.size() << " constraints.\t" << duration << "s.\n";
    
    /* Find Strongest Connected Component and chose inputs accordingly */
    // This is not used for now, but the code of SCC is implemented using
    // Graph::SCC(). A demo is available using demo_SCC.m
    
    /* initialise the graph */
    graph.initlialisePose( );
    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
    cout << "[RotAvg] " << graph.M.size() << " poses.\t\t" << duration << "s.\n";
            
    /* perform rotation averaging */
    graph.QuaternionInit( );
    graph.QuaternionBoxMedian( );
    graph.QuaternionRobustMean( );
          
    /****************************/
    
    /* get output pointers */
    /* this goes through the graph, and gets the state of each node */
    size_t rows = graph.M.size();
    plhs[0] = mxCreateDoubleMatrix(rows, 4, mxREAL);
    double *Qout = mxGetPr(plhs[0]);
    for (k=0; k<rows; k++){
       Qout[0*rows+k] = graph.M[k].Q[0];
       Qout[1*rows+k] = graph.M[k].Q[1];
       Qout[2*rows+k] = graph.M[k].Q[2];
       Qout[3*rows+k] = graph.M[k].Q[3];}

}
