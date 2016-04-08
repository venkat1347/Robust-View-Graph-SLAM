#include "trans_solver.h"

using ceres::AutoDiffCostFunction;
using ceres::CostFunction;
using ceres::Problem;
using ceres::Solver;
using ceres::Solve;
using ceres::HuberLoss;

void solve_translations_problem(
        const int* edges,
        const double* poses,
        const double* weights,
        int num_edges,
        double loss_width,
        double* X,
        double function_tolerance,
        double parameter_tolerance,
        int max_iterations){
    
    // seed the random number generator
    std::srand( std::time( NULL ) );
    
    // reindex the edges to be a sequential set
    int *_edges = new int[2*num_edges];
    memcpy(_edges, edges, 2*num_edges*sizeof(int));
    std::vector<int> reindex_lookup;
    reindex_problem(_edges, num_edges, reindex_lookup);
    int num_nodes = reindex_lookup.size();
    
    // Make a random guess solution
    double *x = new double[3*num_nodes];
    for (int i=0; i<3*num_nodes; ++i){
        x[i] = 0;}
    //    x[i] = (double)rand() / RAND_MAX;}
    
    // initialise x from a spanning-tree
    std::vector<int> it; it.assign(num_nodes,0); // fill the vector i with all zeros
    int a=0,SpanFlag,Nnodes=1;
    it[a] = 1; // first node
    while(Nnodes<num_nodes){
        SpanFlag=0;
        for(int k=0;k<num_edges;k++){
            int i = _edges[2*k+0];
            int j = _edges[2*k+1];
            if (it[i]==1&&it[j]==0){
                it[j] = 1;
                SpanFlag=1;
                x[3*j+0] = x[3*i+0] + poses[3*k+0];
                x[3*j+1] = x[3*i+1] + poses[3*k+1];
                x[3*j+2] = x[3*i+2] + poses[3*k+2];}
            if (it[i]==0&&it[j]==1){
                it[i] = 1;
                SpanFlag=1;
                x[3*i+0] = x[3*j+0] - poses[3*k+0];
                x[3*i+1] = x[3*j+1] - poses[3*k+1];
                x[3*i+2] = x[3*j+2] - poses[3*k+2];}}
        Nnodes = std::accumulate(it.begin(),it.end(),0);
        if (SpanFlag==0&&Nnodes<num_nodes){
            std::cout << "Relative translations DO NOT SPAN all the nodes in the VIEW GRAPH" << std::endl;
            std::cout << "Number of nodes in Spanning Tree = " << Nnodes << std::endl;
            std::cout << "Connected Nodes are : ";
            for (int k=0;k<it.size();k++){ std::cout << it[k] << " ";} std::cout << std::endl;
            std::cout << "Remove extra nodes and retry" << std::endl;
            exit(1);}}
    
    // add the parameter blocks (a 3-vector for each node)
    Problem problem;
    for (int i=0; i<num_nodes; ++i)
        problem.AddParameterBlock(x+3*i, 3);
    
    // set the residual function (chordal distance for each edge)
    for (int i=0; i<num_edges; ++i) {
        CostFunction* cost_function =
                new AutoDiffCostFunction<ChordFunctor, 3, 3, 3>(
                new ChordFunctor(poses+3*i,weights[i]));
        if (loss_width == 0.0){// No robust loss function
            problem.AddResidualBlock(cost_function,NULL,x+3*_edges[2*i+0],x+3*_edges[2*i+1]);} 
        else { 
            problem.AddResidualBlock(cost_function,new ceres::HuberLoss(loss_width),x+3*_edges[2*i+0],x+3*_edges[2*i+1]);}}
    
    // solve
    Solver::Options options;
    options.num_threads = 16;
    options.num_linear_solver_threads = 16;
    //options.minimizer_progress_to_stdout = true;
    options.max_num_iterations  = max_iterations;
    options.function_tolerance  = function_tolerance;
    options.parameter_tolerance = parameter_tolerance;
    options.linear_solver_type  = ceres::ITERATIVE_SCHUR;
    options.preconditioner_type = ceres::SCHUR_JACOBI;
    
    //Solver::Summary summary;
    //Solve(options, &problem, &summary);
    //std::cout << summary.FullReport() << "\n";
    
    // undo the reindexing
    for (int i=0; i<num_nodes; ++i) {
        int j = reindex_lookup[i];
        X[3*j+0] = x[3*i+0];
        X[3*j+1] = x[3*i+1];
        X[3*j+2] = x[3*i+2];}
    
    delete[] _edges;
    delete[] x;}

template <typename T>
        bool ChordFunctor::operator()(
        const T* const x0,
        const T* const x1,
        T* residual) const {
    
    // compute ||x1 - x0||_2
    T norm = sqrt(  (x1[0]-x0[0])*(x1[0]-x0[0]) +
                    (x1[1]-x0[1])*(x1[1]-x0[1]) +
                    (x1[2]-x0[2])*(x1[2]-x0[2]));
    residual[0] = w_*((x1[0]-x0[0]) / norm - T(u_[0]));
    residual[1] = w_*((x1[1]-x0[1]) / norm - T(u_[1]));
    residual[2] = w_*((x1[2]-x0[2]) / norm - T(u_[2]));
    return true;}

void reindex_problem(int* edges, int num_edges, std::vector<int> &reindex_lookup) {
    
    reindex_lookup.clear();
    
    // get the unique set of nodes
    std::set<int> nodes;
    for (int i=0; i<2*num_edges; ++i)
        nodes.insert(edges[i]);
    
    std::map<int, int> reindexing_key;
    
    // iterator through them and assign a new Id to each vertex
    std::set<int>::const_iterator it;
    int n=0;
    for (it = nodes.begin(); it != nodes.end(); ++it) {
        reindex_lookup.push_back(*it);
        reindexing_key[*it] = n;
        ++n;}
    
    // now renumber the edges
    for (int i=0; i<2*num_edges; ++i)
        edges[i]  = reindexing_key[edges[i]];}