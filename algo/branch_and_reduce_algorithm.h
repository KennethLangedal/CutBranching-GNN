/******************************************************************************
 * branch_and_reduce_algorithm.h
 *
 * Copyright (C) 2015-2017 Darren Strash <strash@kit.edu>
 * Copyright (C) 2019 Demian Hespe <hespe@kit.edu>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/

#ifndef BRANCH_AND_REDUCE_SOLVER_H
#define BRANCH_AND_REDUCE_SOLVER_H

// local includes
#include "fast_set.h"
#include "modified.h"
#include "timer.h"

#include "max_flow.h"
#ifdef USE_IFC
#include "../extern/InertialFlowCutter/include/inertial_flow_cutter.h"
#endif
#include "metis.h"

#include "tools/debug_info_logger.h"

// system includes
#include <vector>
#include <stack> 
#include <set>
#include <string>
#include <memory>
#include <cstring>

#include <iostream>
#include <fstream>
#include <regex.h>
#include <memory>

//void get_nd_separators_cutter(std::vector<std::vector<int>> &adj, double balance, std::vector<std::pair<int, std::vector<int>>> &separators);
#include "separator_refinement/fm_ns_local_search.h"
#include "data_structures/dyn_fast_set.h"
#include "data_structures/ArraySet.h"

#include "../gnn/gcn.h"

class branch_and_reduce_algorithm
{
	friend class modified;
	friend class fold;
	friend class alternative;

public:
	static int REDUCTION;
	static int LOWER_BOUND;
	static int BRANCHING;
	static bool outputLP;

	// 1 for extra decompose step
	static int EXTRA_DECOMP;
	static int ND_LEVEL;
	static long TUNING_PARAM1;
	static double TUNING_PARAM2;
	static long TUNING_PARAM3;

	// statistics
	static long defaultBranchings;
	static bool defaultBranch;
	static long defaultPicks;
	static long stratPicks;
	static long nDecomps;

	debug_info_logger* logger;

	static long prunes;
	static long ro;

	static void resetStatistics()
	{
		nBranchings = 0;
		defaultBranch = false;
		defaultBranchings = 0;
		defaultPicks = 0;
		stratPicks = 0;
		nDecomps = 0;
	}

	//std::vector<int> optBranchOrder;
	//std::vector<std::vector<int>>  branchTree;

	std::vector<std::vector<int>> adj;
	static long nBranchings;
	static int debug;
	double SHRINK;
	int depth;
	int maxDepth;
	int max_depth = 0;
	int rootDepth;
	int n;
	int N;

	gcn model;
	gcn_data model_data;

	/**
	 * current best solution
	 */
	int opt;
	std::vector<int> y;

	/**
	 * current solution (-1: not determined, 0: not in the vc, 1: in the vc, 2: removed by foldings)
	 */
	int crt;
	std::vector<int> x;

	/**
	 * #remaining vertices
	 */
	int rn;

	/**
	 * max flow
	 */
	std::vector<int> in;
	std::vector<int> out;

	/**
	 * lower bound
	 */
	int lb;

	std::vector<int> que, level, iter;

	std::vector<int> modTmp;

	std::vector<std::shared_ptr<modified>> modifieds;
	int modifiedN;

	/**
	 * Packing constraints
	 */
	////	std::list<std::vector<int>> packing;
	std::vector<std::vector<int>> packing;

	std::vector<int> vRestore;

	int reductionSnapshotSize;
	std::vector<int> snapshotX;

	branch_and_reduce_algorithm(std::vector<std::vector<int>>& _adj, int const _N);

	int deg(int v);
	void set(int v, int a);

	fast_set used;

	// helpers for modifying the graph
	void compute_fold(std::vector<int> const &S, std::vector<int> const &NS);
	void compute_alternative(std::vector<int> const &A, std::vector<int> const &B);
	void restore(int n);
	void reverse();

	// helpers for lpReduction
	bool dinicDFS(int v);
	void updateLP();

	// reduction methods
	bool lpReduction();
	bool deg1Reduction();
	bool dominateReduction();
	bool fold2Reduction();
	bool twinReduction();
	bool funnelReduction();
	bool funnelReduction_a();
	bool checkFunnel(int v);
	bool deskReduction();
	bool unconfinedReduction();
	bool unconfinedReduction_a();
	int packingReduction();

	// lower bounds for pruning
	int lpLowerBound();
	int cycleLowerBound();
	int cliqueLowerBound();
	int lowerBound();

	// recursive methods
	void branching(timer &t, double time_limit);
	bool decompose(timer &t, double time_limit);
	bool reduce();
	void rec(timer &t, double time_limit);

	// Track how much the starting solution helped
	bool startingSolutionIsBest = false;
	int numBranchesPrunedByStartingSolution = 0;

	// vestiges of original Java code
#if 0
	
	void debug(String str, Object...os) {
		StringBuilder sb = new StringBuilder();
		Calendar c = Calendar.getInstance();
		sb.append(String.format("%02d:%02d:%02d  ", c.get(Calendar.HOUR_OF_DAY), c.get(Calendar.MINUTE), c.get(Calendar.SECOND)));
		for (int i = 0; i < depth && i <= maxDepth; i++) sb.append(' ');
		System.err.print(sb);
		System.err.printf(str, os);
	}
#endif // 0

	int solve(timer &t, double time_limit);

	void initial_reduce_graph();
	void reduce_graph();

	void restore_to_snapshot();

	std::string debugString() const;
	void PrintState() const;

	size_t get_current_is_size() const;
	size_t get_current_is_size_with_folds() const;
	bool folded_vertices_exist() const;
	std::vector<int> compute_maximal_is();
	size_t compute_alternative_maximal_is_size();
	size_t number_of_nodes_remaining() const;
	size_t number_of_edges_remaining() const;
	//void force_into_independent_set(std::vector<NodeID> const &nodes);
	void extend_finer_is(std::vector<bool> &independent_set);
	void get_solved_is(std::vector<bool> &independent_set);

	//void convert_adj_lists(graph_access &G, std::vector<int> &reverse_mapping) const;

	// void convert_to_ga(std::shared_ptr<graph_access> G, std::vector<int> &reverse_mapping, std::vector<int> &mapping);
	void convert_to_metis(int32_t* nNodes, std::vector<int32_t> &xadj, std::vector<int32_t> &adjncy, std::vector<int> &reverse_mapping);
	void convert_to_adj(std::vector<std::vector<int>>& G, std::vector<int> &reverse_mapping, std::vector<int> &mapping);

	void addStartingSolution(std::vector<int> solution, int solutionSize);


	// NEW BRANCHING RULES      

	// articulation points
    std::vector<int> articulation_points;
	std::vector<int> artics;
	std::vector<int> visited;
    std::vector<int> minNr;      
    int current_dfs_num = 0;

	int get_articulation_point();
	void get_articulation_points();
	void dfs_root(int s);
	void dfs(int v, int in);

	void get_articulation_points_iteratively();
	void dfs_iteratively(int v);
	std::stack<std::pair<int,int>> dfs_stack;

	// st cuts
	int s, t;
	int branch_t = 0;
	std::vector<int> cut;
	void get_stcut_vertices();
	void get_stcut_vertices_n();

	// utility
	inline int get_max_deg_vtx();

	// Nested Dissection - Inertial Flow Cutter
	bool nd_computed = false;
	std::vector<int> nd_order;
	void compute_nd_order_cutter();

	std::vector<int> domin_vtcs;
	bool almost_dominated();

	std::vector<int> unconf_vtcs;
	std::vector<int> twin_vtcs;
	std::vector<int> funnel_vtcs;
	std::vector<int> packing_vtcs;

	int packing_thresh = 5;
	int funnel_thresh = 2;
	int twin_thresh = 2;
	int unconf_thresh = 2;


	std::vector<int> b_vtcs;

	int max_nh_vtx();

	// Nested Dissection - Metis
	void compute_nd_order();
	std::vector<std::pair<std::vector<int>, int>> get_nd_separators(int32_t* perm, int32_t* part_sizes, int32_t* sep_sizes, int n, int p, int32_t* weights);


	// refinement
	std::vector<int> partition_index;
	fm_ns_local_search ls_refinement;
	bool perform_refinement = false;

	void compute_metis_sep();
	std::vector<int> separator;
	//PartialBoundary separator;



	// dependency checking
	static bool USE_DEPENDENCY_CHECKING;

	ArraySet dc_candidates;	


	bool deg1Reduction_dc();
	bool reduce_dc();
	bool fold2Reduction_dc();
	bool twinReduction_dc();
	bool funnelReduction_dc();
	bool funnelReduction_a_dc();
	int packingReduction_dc();

	void getPackingCandidates(int v);
	fast_set candidatesChecked;

	std::vector<std::vector<int>> packingMap;
	std::vector<std::vector<int>> foldingMap;
	std::set<int> packingCandidates;
	void inline pushPacking(std::vector<int> &packing); 
	void inline popPacking(); 


#if 0
}
#endif // 0
};

#endif //BRANCH_AND_REDUCE_SOLVER_H
