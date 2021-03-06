function [ x, timings, iter, status, options, stats ] = compare_QP_solvers( prob, options )
%Run QPALM (Matlab), QPALM (C), OSQP, qpoases, and GUROBI on the given problem 
%n times and return the solution and timings

% if isfield(options, 'qrqp') && options.qrqp
%     import casadi.*
% end

n = 1; %to get an average timing
t = zeros(n,1);

x = {};
stats = {};
timings = {};
iter = {};
status = {};

if ~isfield(options, 'VERBOSE')
    VERBOSE = false;
else
    VERBOSE = options.VERBOSE;
end
if ~isfield(options, 'SCALING_ITER')
    SCALING_ITER = 2;
else
    SCALING_ITER = options.SCALING_ITER;
end
if ~isfield(options, 'MAXITER')
    MAXITER = 5000;
else
    MAXITER = options.MAXITER;
end

if ~isfield(options, 'TIME_LIMIT')
    TIME_LIMIT = 3600;
else
    TIME_LIMIT = options.TIME_LIMIT;
end

if ~isfield(options, 'EPS_ABS')
    EPS_ABS = 1e-6;
    EPS_REL = EPS_ABS;
else
    EPS_ABS = options.EPS_ABS;
    EPS_REL = EPS_ABS;
end
%% QPALM Matlab

if isfield(options, 'x')
    x_warm_start = x;
else 
    x_warm_start = [];
end
if isfield(options, 'y')
    y_warm_start = y;
else 
    y_warm_start = [];
end

%Prep A and lbA and ubA for QPALM and OSQP
if isfield(prob, 'l') || isfield(prob, 'u')
    A = [prob.A; eye(size(prob.A,2))];
    A_combined = true;
else
    A = prob.A;
    lbA = prob.lb;
    ubA = prob.ub;
end
if isfield(prob, 'l') && isfield(prob, 'u')
    lbA = [prob.lb; prob.l];
    ubA = [prob.ub; prob.u];
elseif isfield(prob, 'l') && ~isfield(prob, 'u')
    lbA = [prob.lb; prob.l];
    ubA = [prob.ub; inf*ones(length(prob.ub),1)];
elseif ~isfield(prob, 'l') && isfield(prob, 'u')
    lbA = [prob.lb; -inf*ones(length(prob.ub),1)];
    ubA = [prob.ub; prob.u];
end

if options.qpalm_matlab       
    
    for k = 1:n
        opts.solver = 'newton';
        opts.scalar_sig = false;
        opts.maxiter = MAXITER;
        opts.eps_abs = EPS_ABS;
        opts.eps_rel = EPS_REL;
        opts.eps_abs_in = min(EPS_ABS*1e6, 1);
        opts.eps_rel_in = min(EPS_REL*1e6, 1);
        opts.eps_pinf = EPS_ABS;
        opts.eps_dinf = EPS_ABS;
        opts.proximal = true;
        opts.gamma    = 1e1;
        opts.gammaUpd = 10;
        opts.gammaMax = 1e7;
        opts.verbose = VERBOSE;
        opts.Delta   = 100;
        opts.scaling = 'simple';
        opts.scaling_iter = SCALING_ITER;
        tic;[x_qpalm,y_qpalm,stats_qpalm] = qpalm_matlab(prob.Q,prob.q,A,lbA,ubA,x_warm_start,y_warm_start,opts);
        qpalm_time = toc;
        t(k) = qpalm_time;
        
    end
    
    status.qpalm_matlab = stats_qpalm.status;
    iter.qpalm_matlab = stats_qpalm.iter;
    timings.qpalm_matlab = sum(t)/n;
    x.qpalm_matlab = x_qpalm;
    stats.qpalm_matlab = stats_qpalm;
    
    
%     if timings.qpalm_matlab > MAX_TIME
%         options.qpalm_matlab = false;
%     end
    
end

%% QPALM C

if options.qpalm_c
    
    for k = 1:n
        solver = qpalm;
        settings = solver.default_settings();
        
        settings.verbose = VERBOSE;
        settings.scaling = SCALING_ITER;
        settings.max_iter = 50000;
        settings.eps_abs_in = min(EPS_ABS*1e6, 1);
        settings.eps_rel_in = min(EPS_REL*1e6, 1);
        settings.eps_abs = EPS_ABS;
        settings.eps_rel = EPS_REL;
        settings.eps_prim_inf = EPS_ABS;
        settings.eps_dual_inf = EPS_ABS;
        settings.time_limit = TIME_LIMIT;
%         settings.proximal = true;
%         settings.gamma_init = 1e6;
%         settings.gamma_max = 1e6;

        solver.setup(prob.Q, prob.q, A,lbA,ubA, settings);
        try
            res_qpalm = solver.solve();
            t(k) = res_qpalm.info.run_time;
        catch
            t(k) = TIME_LIMIT;
            res_qpalm.info.status = 'failed';
            res_qpalm.info.iter = inf;
            res_qpalm.x = nan;
        end
        solver.delete();
    end

    status.qpalm_c = res_qpalm.info.status;
    iter.qpalm_c = res_qpalm.info.iter;
    timings.qpalm_c = sum(t)/n;
    x.qpalm_c = res_qpalm.x;
    
%     if timings.qpalm_c > MAX_TIME
%         options.qpalm_c = false;
%     end
    
end
%% OSQP
% 
if options.osqp
    for k = 1:n
        solver = osqp;
        osqp_settings = solver.default_settings();

%         osqp_settings.scaling = SCALING_ITER;
        osqp_settings.max_iter = 10000000000;
        osqp_settings.time_limit = TIME_LIMIT;
        osqp_settings.eps_abs = EPS_ABS;
        osqp_settings.eps_rel = EPS_REL;
        osqp_settings.eps_prim_inf = EPS_ABS;
        osqp_settings.eps_dual_inf = EPS_ABS;
        
        osqp_settings.verbose = VERBOSE;
%         osqp_settings.polish = true;
        solver.setup(prob.Q, prob.q, A,lbA,ubA, osqp_settings);
        try
            res_osqp = solver.solve();
            t(k) = res_osqp.info.run_time;
        catch
            t(k) = TIME_LIMIT;
            res_osqp.info.status = 'failed';
            res_osqp.info.iter = inf;
            res_osqp.x = nan;
        end

        solver.delete();
        
    end

    status.osqp = res_osqp.info.status;
    iter.osqp = res_osqp.info.iter;
    timings.osqp = sum(t)/n;
%     if timings.osqp > MAX_TIME
%         options.osqp = false;
%     end
    x.osqp = res_osqp.x;
end
%% qpoases
if isfield(prob, 'l') && isfield(prob, 'u')
    l = prob.l;
    u = prob.u;
elseif isfield(prob, 'l') && ~isfield(prob, 'u')
    l = prob.l;
    u = [];
elseif ~isfield(prob, 'l') && isfield(prob, 'u')
    l = [];
    u = prob.u;
else
    l = [];
    u = [];
end

if options.qpoases
    qpoases_options = qpOASES_options('default', 'printLevel', 0, 'terminationTolerance', 1e-6, 'maxCpuTime', TIME_LIMIT);

    for k = 1:n
        [x.qpoases,fval,status.qpoases,iter.qpoases,lambda,auxOutput] = qpOASES(prob.Q,prob.q,prob.A,l,u,prob.lb,prob.ub,qpoases_options);
        t(k) = auxOutput.cpuTime;
    end
    timings.qpoases = sum(t)/n;
    
%     if timings.qpoases > MAX_TIME
%         options.qpoases = false;
%     end

end

if isfield(options, 'qrqp') && options.qrqp
    qrqp_options = struct;
    qrqp_options.print_iter = false;
    qrqp_options.record_time = true;
    qrqp_options.print_header = false;
%     qrqp_options.max_iter = 1000;
    qp_struct = struct;
    qp_struct.h = sparsity(casadi.DM(prob.Q));
    qp_struct.a = sparsity(casadi.DM(prob.A));
    solver_qrqp = casadi.conic('solver','qrqp',qp_struct,qrqp_options);
    prob_qrqp.g = prob.q;
    prob_qrqp.lbx = -inf*ones(size(prob.q));
    prob_qrqp.ubx = inf*ones(size(prob.q));
    prob_qrqp.lba = prob.lb;
    prob_qrqp.uba = prob.ub;
    prob_qrqp.h = prob.Q;
    prob_qrqp.a = prob.A;
    
    
    for k = 1:n
        try
            res_qrqp = solver_qrqp.call(prob_qrqp);
        catch
            fprintf('QRQP failed: %s\n', solver_qrqp.stats.return_status);
            res_qrqp.x = 0;
        end
        t(k) = solver_qrqp.stats.t_wall_total;
    end
    x.qrqp = full(res_qrqp.x);
    status.qrqp = solver_qrqp.stats.return_status;
    iter.qrqp = solver_qrqp.stats.n_call_total; %not available
    timings.qrqp = sum(t)/n;
end
    
if options.gurobi
    if isfield(options, 'lp') && options.lp
       %do not define model.Q 
    else
        model.Q = 0.5*prob.Q;
    end
    model.obj = prob.q;
    model.A = [prob.A;-prob.A];
    model.rhs = [prob.ub;-prob.lb];
    model.lb = -inf*ones(size(prob.q)); %gurobi uses default lb = 0 on the vars
    if isfield(prob, 'l') && isfield(prob, 'u')
        model.lb = prob.l;
        model.ub = prob.u;
    elseif isfield(prob, 'l') && ~isfield(prob, 'u')
        model.lb = prob.l;
    elseif ~isfield(prob, 'l') && isfield(prob, 'u')
        model.ub = prob.u;
    end
    
    model.sense = '<';
    
    params.outputflag = 0;
    params.OptimalityTol = EPS_ABS;
    params.FeasibilityTol = EPS_ABS;
    params.TimeLimit = TIME_LIMIT;
    
    for k = 1:n
        try
            results = gurobi(model,params);
            t(k) = results.runtime;
        catch
            results.status = 'FAILED';
            results.baritercount = inf;
            t(k) = TIME_LIMIT;
        end
        
    end
    timings.gurobi = sum(t)/n;
    status.gurobi = results.status;
    iter.gurobi = results.baritercount;
    if isfield(results,'x')
        x.gurobi = results.x;
    else
        x.gurobi = nan;
    end
%     if timings.gurobi > MAX_TIME
%         options.gurobi = false;
%     end
    

end

