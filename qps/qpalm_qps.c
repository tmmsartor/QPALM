#include "qpalm.h"
#include "constants.h"
#include "global_opts.h"
#include "cholmod.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* Print a cholmod matrix so the output can be entered into matlab */
void print_cholmod_matlab(cholmod_sparse *M) {
    printf("M = sparse(%ld, %ld);\n", M->nrow, M->ncol);
    size_t col, index = 0, row;
    double *Mx = M->x;
    long int *Mi = M->i;
    long int *Mp = M->p;


    for (col = 1; col <= M->ncol; col++) {
        for (row = Mp[col-1]; row < Mp[col]; row++) {
            printf("M(%ld, %ld) = %.16le;\n", Mi[index]+1, col, Mx[index]);
            index++;
        }
    }
}

void print_dense_vector_matlab(double* x, size_t len) {
    size_t k;
    printf("x = zeros(%lu, 1);", len);
    for (k = 0; k < len; k++) {
        printf("x(%lu) = %.16le;", k+1, x[k]);
    }
    printf("\n");
}

int get_next_command_and_check(char* command, char* check, char next_char, FILE* fp) {
    command[0] = next_char;
    char line[100];
    fgets(line, 100, fp);
    sscanf(line, "%s", &command[1]);

    if (strcmp(command, check)) {
        fprintf(stderr, "Wrong command. Expected next command to be %s, but was %s.\n", check, command);
        return 1;
    }
    return 0;
}

int get_next_command(char* command, char next_char, FILE* fp) {
    command[0] = next_char;
    char line[100];
    fgets(line, 100, fp);
    sscanf(line, "%s", &command[1]);

    return strcmp(command, "ENDATA");
}

long convert_to_long(char *s) {
    char c;
    long i, digit, number = 0;
    for(i=0;i<strlen(s);i++)
    {
    	c = s[i];
    	if(c>='0' && c<='9') //to confirm it's a digit
    	{
    		digit = c - '0';
    		number = number*10 + digit;
    	}
    }
    return number;
}

int main(int argc, char*argv[]){

    if (argc != 2) {
        fprintf(stderr, "Wrong number of arguments. Correct usage is qpalm_qps problem.qps\n");
    }
    
    // Load problem data
    size_t n, m, Annz, Qnnz, n_bounds;
    n = 0; m = 0; Annz = 0; Qnnz = 0; n_bounds = 0;
    FILE* fp;

    fp = fopen(argv[1], "r");
    if(fp == NULL) {
        fprintf(stderr, "Could not open file %s\n", argv[1]);
        return 1;
    }

    char line[100], command[20], name[50];
    fgets(line, 100, fp);
    if (sscanf(line, "%s %s", command, name) != 2) {
        fprintf(stderr, "Wrong file format. Expected first line to contain NAME problem_name nnz.\n");
        return 1;
    }

    if (strcmp(command, "NAME")) {
        fprintf(stderr, "Wrong file format. Expected first line to contain NAME problem_name nnz.\n");
        return 1;
    }

    printf("Reading problem %s\n", name);

    char next_char;
    next_char = fgetc(fp);
    char NLGE[1], buf[20], objective[20];
    c_float temp, temp2;
    char colchar[20], rowchar[20], prev_colchar[20], second_rowchar[20];
    prev_colchar[0] = '\0';
    second_rowchar[0] = '\0';

    // c_int *bounds = c_calloc(n, sizeof(c_int));
    c_int *bounds;
    size_t k;
    
    char bound_type[20];
    char prev_rowchar[20];
    prev_rowchar[0] = '\0';
    long row;
    

    /*First pass through the file to get the sizes*/
    while(get_next_command(command, next_char, fp)){
        next_char = fgetc(fp);

        if (!strcmp(command, "ROWS")) {
    
            while(next_char == ' ') {
                fgets(line, 100, fp);
                sscanf(line, "%s %s", NLGE, buf);
                if (!strcmp(NLGE, "N")) {
                    strcpy(objective, buf);
                } else {
                    m++;
                }
                
                next_char = fgetc(fp);
            }

        } else if (!strcmp(command, "COLUMNS")) {

            while(next_char == ' ') {
                fgets(line, 100, fp);
                sscanf(line, "%s %s %le %s %le", colchar, rowchar, &temp, second_rowchar, &temp2);

                if (strcmp(colchar, prev_colchar)) {
                    n++;
                    strcpy(prev_colchar, colchar);
                }

                if (!strcmp(rowchar, objective)) {            
                } else {
                    Annz++;
                }

                if (!strcmp(second_rowchar, objective)){
                } else if(!strcmp(second_rowchar, "")) {
                } else {
                    Annz++;
                    second_rowchar[0] = '\0';
                }
                
                next_char = fgetc(fp);
            }

        } else if (!strcmp(command, "RHS") || !strcmp(command, "RANGES")) {
            while(next_char == ' ') {
                fgets(line, 100, fp);
                next_char = fgetc(fp);
            }

        } else if (!strcmp(command, "BOUNDS")) {
            bounds = c_calloc(n, sizeof(c_int));
            for (k = 0; k < n; k++) {
                bounds[k] = TRUE; //default lower bound = 0
            }
            n_bounds = n;

            while(next_char == ' ') {
                fgets(line, 100, fp);
                sscanf(line, "%s %*s %s %le", bound_type, rowchar, &temp);
                if (!strcmp(bound_type, "FR")) {
                    row = convert_to_long(rowchar)-1;
                    bounds[row] = FALSE;
                    n_bounds -= 1;
                } 

                next_char = fgetc(fp);
            }
            Annz += n_bounds;
            m += n_bounds;

        } else if (!strcmp(command, "QUADOBJ")) {
            while(next_char == ' ') {
                fgets(line, 100, fp);
                next_char = fgetc(fp);
                Qnnz++;
            }
        }
    }


    // if(get_next_command_and_check(command, "ROWS", next_char, fp))
    //     return 1;

    
    // if(get_next_command_and_check(command, "COLUMNS", next_char, fp))
    //     return 1;
    
    
    // if(get_next_command_and_check(command, "RHS", next_char, fp))
    //     return 1;

    

    // if(get_next_command_and_check(command, "RANGES", next_char, fp))
    //     return 1;

    // if(get_next_command_and_check(command, "BOUNDS", next_char, fp))
    //     return 1;

    // if(get_next_command_and_check(command, "QUADOBJ", next_char, fp))
    //     return 1;
    // next_char = fgetc(fp);
    // while(next_char == ' ') {
    //     fgets(line, 100, fp);
    //     next_char = fgetc(fp);
    //     Qnnz++;
    // }


    // if(get_next_command_and_check(command, "ENDATA", next_char, fp))
    //     return 1;
    fclose(fp);

    // printf("Results: m = %lu, n = %lu, Qnnz = %lu, Annz = %lu\n", m, n, Qnnz, Annz);

    QPALMData* data = c_calloc(1, sizeof(QPALMData));
    data->m = m;
    data->n = n;
    data->c = 0;
    data->q = c_calloc(n, sizeof(c_float));
    
    for (k = 0; k < n; k++) {
        data->q[k] = 0;
    }
    data->bmin = c_calloc(m, sizeof(c_float));
    data->bmax = c_calloc(m, sizeof(c_float));    

    cholmod_common c;
    CHOLMOD(start)(&c);
    data->A = CHOLMOD(allocate_sparse)(m, n, Annz, TRUE, TRUE, 0, CHOLMOD_REAL, &c);
    data->Q = CHOLMOD(allocate_sparse)(n, n, Qnnz, TRUE, TRUE, -1, CHOLMOD_REAL, &c);

    c_float *Ax = data->A->x;
    c_int *Ai = data->A->i;
    c_int *Ap = data->A->p;

    c_float *Qx = data->Q->x;
    c_int *Qi = data->Q->i;
    c_int *Qp = data->Q->p;

    fp = fopen(argv[1], "r");
    if(fp == NULL) {
        fprintf(stderr, "Could not open file %s\n", argv[1]);
        return 1;
    }
    
    fgets(line, 100, fp);
    next_char = fgetc(fp);

    if(get_next_command_and_check(command, "ROWS", next_char, fp))
        return 1;

    next_char = fgetc(fp);

    char constraint_signs[m-n_bounds];
    size_t index = 0;
    while(next_char == ' ') {
        fgets(line, 100, fp);
        sscanf(line, "%s %s", NLGE, buf);
        if (!strcmp(NLGE, "E")) { //default in equality for bmax = bmin = 0
            data->bmax[index] = 0;
        }
        if (strcmp(NLGE, "N")) {
            constraint_signs[index] = NLGE[0];
            index++;
        } 
        
        
        next_char = fgetc(fp);
    }

    for (k = 0; k < m; k++) {
        if (k >= m-n_bounds) {
            data->bmin[k] = 0;
            data->bmax[k] = QPALM_INFTY;
        }
        else switch (constraint_signs[k]) {
                case 'L':
                    data->bmax[k] = 0;
                    data->bmin[k] = -QPALM_INFTY;
                    break; 
                case 'G':
                    data->bmin[k] = 0;
                    data->bmax[k] = QPALM_INFTY;
                    break;
                case 'E':
                    data->bmin[k] = 0;
                    data->bmax[k] = 0;
                    break;
            
        }
    }

    if(get_next_command_and_check(command, "COLUMNS", next_char, fp))
        return 1;
    
    next_char = fgetc(fp);

    Ap[0] = 0;
    size_t elemA = 0;
    long bounds_row = m-n_bounds, col, prev_col = 1; 
    while(next_char == ' ') {
        fgets(line, 100, fp);
        sscanf(line, "%s %s %le %s %le", colchar, rowchar, &temp, second_rowchar, &temp2);
        row = convert_to_long(rowchar)-1;
        col = convert_to_long(colchar);
        if (col > prev_col) {
            for (; prev_col < col; prev_col++) {
                if (bounds[prev_col-1]) { //take into account identity matrix for bounds
                // p = Ap[col+1];
                    Ai[Ap[prev_col]] = bounds_row;
                    Ax[Ap[prev_col]] = 1;
                    Ap[prev_col]++;
                    elemA++;
                    bounds_row++;
                }
                Ap[prev_col+1] = Ap[prev_col];
            }
        }

        if (!strcmp(rowchar, objective)) { 
            data->q[col-1] = temp;            
        } else {
            Ai[elemA] = row;
            Ap[col]++;
            if (temp > QPALM_INFTY) {
                temp = QPALM_INFTY;
            } else if (temp < -QPALM_INFTY) {
                temp = -QPALM_INFTY;
            }
            Ax[elemA] = temp;
            elemA++;
        }

        if(!strcmp(second_rowchar, "")) {
        } else {
            row = convert_to_long(second_rowchar)-1;
            Ai[elemA] = row;
            Ap[col]++;
            if (temp2 > QPALM_INFTY) {
                temp2 = QPALM_INFTY;
            } else if (temp2 < -QPALM_INFTY) {
                temp2 = -QPALM_INFTY;
            }
            Ax[elemA] = temp2;
            elemA++;
            second_rowchar[0] = '\0';
        }
        
        next_char = fgetc(fp);
    }
    col = n;
    if (col > prev_col) {
        for (; prev_col < col; prev_col++) {
            if (bounds[prev_col-1]) { //take into account identity matrix for bounds
                Ai[Ap[prev_col]] = bounds_row;
                    Ax[Ap[prev_col]] = 1;
                    Ap[prev_col]++;
                    elemA++;
                    bounds_row++;
            }
            Ap[prev_col+1] = Ap[prev_col];
        }
    } else if ((col == prev_col) && bounds[prev_col-1]) { //take into account identity matrix for bounds
        Ai[Ap[prev_col]] = bounds_row;
        Ax[Ap[prev_col]] = 1;
        Ap[prev_col]++;
        elemA++;
        bounds_row++;
    }

    
    if(get_next_command_and_check(command, "RHS", next_char, fp))
        return 1;

    next_char = fgetc(fp);
    k = 0;
    while(next_char == ' ') {
        fgets(line, 100, fp);
        sscanf(line, "%*s %s %le", rowchar, &temp);
        row = convert_to_long(rowchar)-1;
        if (!strcmp(rowchar, objective))
            data->c = -temp;
        else {
            switch (constraint_signs[row]) {
                case 'L':
                    data->bmax[row] = temp;
                    data->bmin[row] = -QPALM_INFTY;
                    break; 
                case 'G':
                    data->bmin[row] = temp;
                    break;
                case 'E':
                    data->bmin[row] = temp;
                    data->bmax[row] = temp;
                    break;
            }
             
        }
        next_char = fgetc(fp);
    }

    long prev_row = m-n_bounds;
    if(get_next_command_and_check(command, "RANGES", next_char, fp))
        return 1;

    next_char = fgetc(fp);

    while(next_char == ' ') {
        fgets(line, 100, fp);
        sscanf(line, "%*s %s %le", rowchar, &temp);
        row = convert_to_long(rowchar)-1;
        switch (constraint_signs[row]) {
            case 'L':
                data->bmin[row] = data->bmax[row] - temp;
                break; 
            case 'G':
                data->bmax[row] = data->bmin[row] + temp;
                break;
        }
        next_char = fgetc(fp);
    }

    if(get_next_command_and_check(command, "BOUNDS", next_char, fp))
        return 1;

    // printf("Reading BOUNDS\n");

    next_char = fgetc(fp);
    long p; row = prev_row; prev_col = -1; index = prev_row;
    while(next_char == ' ') {
        fgets(line, 100, fp);
        sscanf(line, "%s %*s %s %le", bound_type, colchar, &temp);
        col = convert_to_long(colchar)-1;
        
        if (!strcmp(bound_type, "FR")) {
            index--;
        } else if (!strcmp(bound_type, "UP")) {
            data->bmax[index+col] = temp;
        } else if (!strcmp(bound_type, "LO")) {
            data->bmin[index+col] = temp;
        } else if (!strcmp(bound_type, "FX")) {
            data->bmin[index+col] = temp;
            data->bmax[index+col] = temp;
        }
        
        next_char = fgetc(fp);
    }


    if(get_next_command_and_check(command, "QUADOBJ", next_char, fp))
        return 1;
    next_char = fgetc(fp);
    prev_col = 1;
    size_t elemQ = 0;
    Qp[0] = 0;
    while(next_char == ' ') {
        fgets(line, 100, fp);
        sscanf(line, "%s %s %le", colchar, rowchar, &temp);
        col = convert_to_long(colchar);
        row = convert_to_long(rowchar);

        Qi[elemQ] = --row;
        
        if (col > prev_col) {
            for (; prev_col < col; prev_col++) {
                Qp[prev_col+1] = Qp[prev_col];
            }          
        }
        Qp[col]++;
        if (temp > QPALM_INFTY) {
            temp = QPALM_INFTY;
        } else if (temp < -QPALM_INFTY) {
            temp = -QPALM_INFTY;
        }
        Qx[elemQ] = temp;

        elemQ++;
        next_char = fgetc(fp);
    }
    col = n;
    if (col > prev_col) {
            for (; prev_col < col; prev_col++) {
                Qp[prev_col+1] = Qp[prev_col];
            }          
    }



    if(get_next_command_and_check(command, "ENDATA", next_char, fp))
        return 1;

    fclose(fp);

    CHOLMOD(finish)(&c);

    // print_cholmod_matlab(data->Q);
    // print_dense_vector_matlab(data->bmin, m);

    // for (k = 0; k < m; k++) {
    //     printf("bmin[%ld] = %le, bmax[%ld] = %le\n", k, data->bmin[k], k, data->bmax[k]);
    // }
    
    // for (k = 0; k < n; k++) {
    //     printf("q[%ld] = %le\n", k, data->q[k]);
    // }
    // for (k = 0; k <= n; k++) {
    //     printf("Qp[%ld] = %ld\n", k, Qp[k]);
    // }
    // for (k = 0; k < Qnnz; k++) {
    //     printf("Qi[%ld] = %ld\n", k, Qi[k]);
    //     printf("Qx[%ld] = %le\n", k, Qx[k]);
    // }
    // printf("data->c, %le\n", data->c);

    // c.print = 4;
    // CHOLMOD(print_sparse)(data->A, "Amatrix", &c);
    // printf("Check A: %d\n", CHOLMOD(check_sparse)(data->A, &c));
    // CHOLMOD(print_sparse)(data->Q, "Qmatrix", &c);
    // printf("Check Q: %d\n", CHOLMOD(check_sparse)(data->Q, &c));

    // for (k = 0; k <= n; k++) {
    //     printf("Ap[%ld] = %ld\n", k, Ap[k]);
    // }
    // for (k = 0; k < Annz; k++) {
    //     printf("Ai[%ld] = %ld\n", k, Ai[k]);
    //     printf("Ax[%ld] = %le\n", k, Ax[k]);
    // }


    // Setup and solve problem
    // printf("Before setup\n");

    // Problem settings
    QPALMSettings *settings = (QPALMSettings *)c_malloc(sizeof(QPALMSettings));
    // Define Solver settings as default
    qpalm_set_default_settings(settings);
    settings->eps_abs = 1e-6;
    settings->eps_rel = 1e-6;
    settings->eps_dual_inf = 1e-6;
    settings->eps_prim_inf = 1e-6;
    settings->max_iter = 10000;
    settings->verbose = FALSE;
    settings->scaling = FALSE;
    // settings->proximal = TRUE;

    QPALMWorkspace *work = qpalm_setup(data, settings, &c);

    // printf("Before solve\n");

    // Solve Problem
    qpalm_solve(work);
    // printf("After solve\n");

    // for (k = 0; k < n; k++) {
    //     printf("x_sol[%ld] = %le\n", k, work->solution->x[k]);
    // }
    printf("Iter: %ld\n", work->info->iter);
    printf("Status: %s\n", work->info->status);
    printf("Objective: %le\n", work->info->objective);
    
    strcpy(line, argv[1]);
    size_t last_slash = 0;
    for (k = 0; k < strlen(line); k++) {
        if (line[k] == '/') /*assume linux directory*/
            last_slash = k+1;
    }
    strcpy(name, &line[last_slash]);
    // name = &name[78];
    name[strlen(name)-4] = '\0'; /*delete .qps*/
    // printf("%s\n", name);
    fp = fopen("out.tex", "a");
    fprintf(fp, "%s & %lu & %lu & %lu & %lu & %lu & %le \\\\", name, n, m, Annz, Qnnz, work->info->iter, work->info->objective);
    fclose(fp);

    // Clean workspace
    CHOLMOD(start)(&c);
    CHOLMOD(free_sparse)(&data->Q, &c);
    CHOLMOD(free_sparse)(&data->A, &c);
    CHOLMOD(finish)(&c);
    qpalm_cleanup(work);

    c_free(data->q);
    c_free(data->bmin);
    c_free(data->bmax);
    c_free(data);
    c_free(settings);

    c_free(bounds);

    return 0;
}