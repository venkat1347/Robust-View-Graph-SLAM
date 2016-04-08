
#include "mex.h"
#include "ceres/ceres.h"
#include "../src/trans_solver.cpp"

using namespace std;

/****************************/

/* The gateway function */
void mexFunction( int nlhs, mxArray *plhs[],
                  int nrhs, const mxArray *prhs[]){
    
    double *dim0;  /* edges   - 2xN */
    dim0 = mxGetPr(prhs[0]);
    double *dim1;  /* poses   - 3xN */
    dim1 = mxGetPr(prhs[1]);
    double *dim2;  /* weights - 1xN */
    dim2 = mxGetPr(prhs[2]);
    if (mxGetN(prhs[0])!=mxGetN(prhs[1])||mxGetN(prhs[0])!=mxGetN(prhs[2])
    ||mxGetN(prhs[1])!=mxGetN(prhs[2])){mexErrMsgIdAndTxt("MATLAB:solve_translation:nargin",
            "All inputs should be column major with equal number of columns.");}
    
    int ncols = mxGetN(prhs[0]); /* matrix dimensions */
    
    /*************************************************************/
    int* edges = new int [2*ncols];
    double* poses = new double [3*ncols];
    double* weights = new double [ncols];
    for (int i=0;i<ncols;i++){
        edges[2*i+0] = dim0[2*i+0];
        edges[2*i+1] = dim0[2*i+1];
        poses[2*i+0] = dim1[2*i+0];
        poses[2*i+1] = dim1[2*i+1];
        poses[2*i+2] = dim1[2*i+2];
        weights[i]   = dim2[i];}
    /*************************************************************/
    int num_edges = ncols;
    int num_nodes = *std::max_element(edges,edges+2*num_edges)+1; // +1 -> first node is 0
    double loss_width          = 0.1;
    double function_tolerance  = 1e-7;
    double parameter_tolerance = 1e-7;
    int max_iterations         = 1000;
    double* X = new double [3*num_nodes]; // output estimated translations
    //double* X = (double*)malloc(3*num_edges*sizeof(double));
    
//     int Nedges = 0;
//     while(Nedges<num_edges){
//         std::cout << poses[3*Nedges+0] << " " << poses[3*Nedges+1] << " " << poses[3*Nedges+2] << std::endl;
//         Nedges++;}
    
    solve_translations_problem(edges, poses, weights, num_edges, loss_width, X,
            function_tolerance, parameter_tolerance, max_iterations);
    /*************************************************************/
    plhs[0] = mxCreateDoubleMatrix(3, num_nodes, mxREAL);
    double *out0 = mxGetPr(plhs[0]);
    for (int i=0;i<num_nodes;++i){
        out0[3*i+0]=X[3*i+0];
        out0[3*i+1]=X[3*i+1];
        out0[3*i+2]=X[3*i+2];}
    /*************************************************************/
    delete edges;
    delete poses;
    delete weights;
    delete X;
}