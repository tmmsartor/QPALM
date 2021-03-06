%Random QP
clear; close all;

current = fileparts(mfilename('fullpath'));
cd(current);

options.qpalm_matlab = false;
options.qpalm_c = true;
options.osqp = true;
options.qpoases = true;
options.gurobi = true;

Tqpalm_matlab = [];
Tqpalm_c = [];
Tosqp = [];
Tqpoases = [];
Tgurobi = [];

Iter_qpalm_matlab = [];
Iter_qpalm_c = [];
Iter_osqp = [];
Iter_qpoases = [];
Iter_gurobi = [];

rc = logspace(0, 5, 20);
% n_values = 100:20:600;
% nb_n = length(n_values);

nbProb = 5; %Number of problems per condition number 

options.EPS_ABS = 1e-6;
options.SCALING_ITER = 2;
options.MAXITER = 10000;

for i = 1:length(rc)
%     n = n_values(i);
    n = 100;
    m = 10*n;
    
    qpalm_matlab_time = 0;
    qpalm_c_time = 0;
    osqp_time = 0;
    qpoases_time = 0;
    gurobi_time = 0;
    
    qpalm_matlab_iter = 0;
    qpalm_c_iter = 0;
    osqp_iter = 0;
    qpoases_iter = 0;
    gurobi_iter = 0;
    
    for k = 1:nbProb
        M = sprandn(n,n,5e-1, (1/rc(i)));
        Q = M*M';

        A = sprandn(m,n,5e-1, (1/rc(i)));
        q = rc(i)*randn(n,1);
        lb = -rand(m,1);
        ub = rand(m,1);

        prob.Q = Q; prob.A = A; prob.lb = lb; prob.ub = ub; prob.q = q;


        [X, timings, iter, status, options] = compare_QP_solvers(prob, options);
        if options.qpalm_matlab , qpalm_matlab_time = qpalm_matlab_time + timings.qpalm_matlab; end
        if options.qpalm_c , qpalm_c_time = qpalm_c_time + timings.qpalm_c; end
        if options.osqp, osqp_time = osqp_time + timings.osqp; end
        if options.qpoases, qpoases_time = qpoases_time + timings.qpoases; end
        if options.gurobi, gurobi_time = gurobi_time + timings.gurobi; end
        
        if options.qpalm_matlab , qpalm_matlab_iter = qpalm_matlab_iter + iter.qpalm_matlab; end
        if options.qpalm_c , qpalm_c_iter = qpalm_c_iter + iter.qpalm_c; end
        if options.osqp, osqp_iter = osqp_iter + iter.osqp; end
        if options.qpoases, qpoases_iter = qpoases_iter + iter.qpoases; end
        if options.gurobi, gurobi_iter = gurobi_iter + iter.gurobi; end
        
        
    end
    
    if options.qpalm_matlab, Tqpalm_matlab(i) = qpalm_matlab_time/nbProb; end
    if options.qpalm_c, Tqpalm_c(i) = qpalm_c_time/nbProb; end
    if options.osqp, Tosqp(i) = osqp_time/nbProb; end
    if options.qpoases, Tqpoases(i) = qpoases_time/nbProb; end
    if options.gurobi, Tgurobi(i) = gurobi_time/nbProb; end

    if options.qpalm_matlab, Iter_qpalm_matlab(i) = qpalm_matlab_iter/nbProb; end
    if options.qpalm_c, Iter_qpalm_c(i) = qpalm_c_iter/nbProb; end
    if options.osqp, Iter_osqp(i) = osqp_iter/nbProb; end
    if options.qpoases, Iter_qpoases(i) = qpoases_iter/nbProb; end
    if options.gurobi, Iter_gurobi(i) = gurobi_iter/nbProb; end
    
    if options.qpalm_matlab, Status_qpalm_matlab{i} = status.qpalm_matlab; end
    if options.qpalm_c, Status_qpalm_c{i} = status.qpalm_c; end
    if options.osqp, Status_osqp{i} = status.osqp; end
    if options.qpoases, Status_qpoases{i} = status.qpoases; end
    if options.gurobi, Status_gurobi{i} = status.gurobi; end
    
    if options.qpalm_matlab, X_qpalm_matlab{i} = X.qpalm_matlab; end
    if options.qpalm_c, X_qpalm_c{i} = X.qpalm_c; end
    if options.osqp, X_osqp{i} = X.osqp; end
    if options.qpoases, X_qpoases{i} = X.qpoases; end
    if options.gurobi, X_gurobi{i} = X.gurobi; end
    
end

save('output/randomQPconditioning');

%% Plot results

plot_QP_comparison_loglog('output/randomQPconditioning')
    