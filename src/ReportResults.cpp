//@HEADER
// ************************************************************************
//
//               HPCG: Simple Conjugate Gradient Benchmark Code
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
//@HEADER
// Input
// size - Number of MPI processes
// rank - My process id
//

#include "ReportResults.hpp"
#include "YAML_Element.hpp"
#include "YAML_Doc.hpp"

#ifndef HPCG_NOMPI
#include <mpi.h> // If this routine is not compiled with HPCG_NOMPI
#endif

#ifdef DEBUG
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#endif

void ReportResults(const Geometry & geom, const SparseMatrix & A, int niters, double normr, double times[]) {

#ifndef HPCG_NOMPI
    double t4 = times[4];
    double t4min = 0.0;
    double t4max = 0.0;
    double t4avg = 0.0;
    MPI_Allreduce(&t4, &t4min, 1, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD);
    MPI_Allreduce(&t4, &t4max, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
    MPI_Allreduce(&t4, &t4avg, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    t4avg = t4avg/((double) geom.size);
#endif
    
    // initialize YAML doc
    
    if (geom.rank==0) { // Only PE 0 needs to compute and report timing results


        double fniters = niters;
        double fnrow = A.totalNumberOfRows;
        double fnnz = A.totalNumberOfNonzeros;

        // Op counts come from implementation of CG in CG.cpp
        double fnops_ddot = fniters*6*fnrow; // 3 ddots with nrow adds and nrow mults
        double fnops_waxpby = fniters*6*fnrow; // 3 waxpbys with nrow adds and nrow mults
        double fnops_sparsemv = fniters*2*fnnz; // 1 spmv with nnz adds and nnz mults
        double fnops_precond = fniters*3*fnnz; // Two GS sweeps, but only use lower triangle for first sweep
        double fnops = fnops_ddot+fnops_waxpby+fnops_sparsemv+fnops_precond;
        
        YAML_Doc doc("benchmark-hpcg", "0.1");
        
        doc.add("Machine Summary","");
        doc.get("Machine Summary")->add("Distributed Processes",geom.size);
        doc.get("Machine Summary")->add("Threaded Processes   ",geom.numThreads);

        doc.add("Dimensions","");
        doc.get("Dimensions")->add("nx",geom.nx);
        doc.get("Dimensions")->add("ny",geom.ny);
        doc.get("Dimensions")->add("nz",geom.nz);
        
        
        
        doc.add("Number of iterations: ", niters);
        doc.add("Final residual: ", normr);
        doc.add("********** Performance Summary (times in sec) ***********","");
        
        doc.add("Time Summary","");
        doc.get("Time Summary")->add("Total   ",times[0]);
        doc.get("Time Summary")->add("DDOT    ",times[1]);
        doc.get("Time Summary")->add("WAXPBY  ",times[2]);
        doc.get("Time Summary")->add("SPARSEMV",times[3]);
        doc.get("Time Summary")->add("PRECOND ",times[5]);
        
        doc.add("FLOPS Summary","");
        doc.get("FLOPS Summary")->add("Total   ",fnops);
        doc.get("FLOPS Summary")->add("DDOT    ",fnops_ddot);
        doc.get("FLOPS Summary")->add("WAXPBY  ",fnops_waxpby);
        doc.get("FLOPS Summary")->add("SPARSEMV",fnops_sparsemv);
        doc.get("FLOPS Summary")->add("PRECOND ",fnops_precond);
        
        doc.add("MFLOPS Summary","");
        doc.get("MFLOPS Summary")->add("Total   ",fnops/times[0]/1.0E6);
        doc.get("MFLOPS Summary")->add("DDOT    ",fnops_ddot/times[1]/1.0E6);
        doc.get("MFLOPS Summary")->add("WAXPBY  ",fnops_waxpby/times[2]/1.0E6);
        doc.get("MFLOPS Summary")->add("SPARSEMV",fnops_sparsemv/(times[3])/1.0E6);
        doc.get("MFLOPS Summary")->add("PRECOND ",fnops_precond/(times[5])/1.0E6);
        
#ifndef HPCG_NOMPI
        doc.add("DDOT Timing Variations","");
        doc.get("DDOT Timing Variations")->add("Min DDOT MPI_Allreduce time",t4min);
        doc.get("DDOT Timing Variations")->add("Max DDOT MPI_Allreduce time",t4max);
        doc.get("DDOT Timing Variations")->add("Avg DDOT MPI_Allreduce time",t4avg);
        
        double totalSparseMVTime = times[3] + times[5]+ times[6];
        doc.add("SPARSEMV OVERHEADS","");
        doc.get("SPARSEMV OVERHEADS")->add("SPARSEMV MFLOPS W OVERHEAD",fnops_sparsemv/(totalSparseMVTime)/1.0E6);
        doc.get("SPARSEMV OVERHEADS")->add("SPARSEMV PARALLEL OVERHEAD Time", (times[7]+times[6]));
        doc.get("SPARSEMV OVERHEADS")->add("SPARSEMV PARALLEL OVERHEAD Pct", (times[7]+times[6])/totalSparseMVTime*100.0);
        doc.get("SPARSEMV OVERHEADS")->add("SPARSEMV PARALLEL OVERHEAD Setup Time", (times[7]));
        doc.get("SPARSEMV OVERHEADS")->add("SPARSEMV PARALLEL OVERHEAD Setup Pct", (times[7])/totalSparseMVTime*100.0);
        doc.get("SPARSEMV OVERHEADS")->add("SPARSEMV PARALLEL OVERHEAD Bdry Exch Time", (times[6]));
        doc.get("SPARSEMV OVERHEADS")->add("SPARSEMV PARALLEL OVERHEAD Bdry Exch Pct", (times[6])/totalSparseMVTime*100.0);
#endif
        
            std::string yaml = doc.generateYAML();
#ifdef DEBUG
            cout << yaml;
#endif
    }
	return;
}
