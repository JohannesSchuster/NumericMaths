#include "lr.h"

#include "../utils.h"
#include "../defines.h"
#include <stdlib.h>

//Normal LR decomposition
int lrInp(mat A, int const n, double const eps)
{
    // Decomposing matrix into Upper and Lower
    // triangular matrix
    for (int i = 1; i < n; ++i) 
    {
        // Upper Triangular
        for (int j = i; j < n; ++j)
        {
            // Summation of L(i, j) * U(j, k)
            double sum = 0;
            for (int k = 0; k < i-1; ++k)
            {
                sum += (A[i*n+k] * A[k*n+j]);
            }
            A[i*n+j] -= sum;
        }
 
        // Lower Triangular
        for (int j = i+1; j < n; ++j) 
        {
            // Summation of L(k, j) * U(j, i)
            double sum = 0;
            for (int k = 1; k < i-1; ++k)
            {
                sum += (A[i*n+k] * A[k*n+j]);
            }
            if (dabs(A[i*n+i]) < eps)
            {
                //Pivot element too small
                return i;
            }

            // Evaluating L(k, i)
            A[j*n+i] = (A[i*n+j] - sum) / A[i*n+i];
        }
    }
    return SUCCESS;
}

//LR decomposition with row permutation 
int lr_pivot_colsInp(mat A, ivec pi, int const n, double const eps)
{
    //Initialize p
    for (int i=0; i<n; ++i) 
    {
        pi[i] = i;
    }

    //Store the number of transpositions in pi[n]
    pi[n] = 0;

    for (int k=0; k<n-1; ++k)
    {
        double max = 0;
        int p = 0;
        for (int j=k; j<n; ++j)
        {
            if (dabs(A[j*n+k]) > max)
            {
                max = A[j*n+k];
                p = j;
            }
        }

        //Only swap when neccessary
        if (p!=k)
        {
            //swap pi[p] and pi[k]
            {
                int buff = pi[k];
                pi[k] = pi[p];
                pi[p] = buff;
            }

            //swap lines A[p,1:n] and A[k,1:n]
            for (int j=0; j<n; ++j)
            {
                double buff = A[k*n+j];
                A[k*n+j] = A[p*n+j];
                A[p*n+j] = buff;
            }

            pi[n]++;
        }

        //Check the pivot element, 
        if (dabs(A[k*n+k]) < eps)
        {
            //If too small A is singular
            return k;
        }

        for (int i=k+1; i<n; ++i)
        {
            A[i*n+k] /= A[k*n+k];
            double sum = 0;
            for (int j=k+1; j<n; ++j)
            {
                A[i*n+j] -= A[i*n+k] * A[k*n+j];
            }
        }
    }

    return SUCCESS;
}

//LR decomposition with line permutation
int lr_pivot_linesInp(mat A, ivec pi, int const n, double const eps)
{
    //Initialize p
    for (int i=0; i<n; ++i) 
    {
        pi[i] = i;
    }
    
    //Store the number of transpositions in pi[n]
    pi[n] = 0;

    for (int k=0; k<n-1; ++k)
    {
        double max = 0;
        int p = 0;
        for (int j=k; j<n; ++j)
        {
            if (dabs(A[k*n+j]) > max)
            {
                max = A[k*n+j];
                p = j;
            }
        }

        //Only swap when neccessary
        if (p!=k)
        {
            //swap pi[p] and pi[k]
            {
                int buff = pi[k];
                pi[k] = pi[p];
                pi[p] = buff;
            }

            //swap cols A[1:n,p] and A[1:n,p]
            for (int j=0; j<n; ++j)
            {
                double buff = A[j*n+k];
                A[j*n+k] = A[j*n+p];
                A[j*n+p] = buff;
            }

            pi[n]++;
        }

        //Check the pivot element, 
        if (dabs(A[k*n+k]) < eps)
        {
            //If too small A is singular
            return k;
        }

        for (int i=k+1; i<n; ++i)
        {
            A[k*n+i] /= A[k*n+k];
            double sum = 0;
            for (int j=k+1; j<n; ++j)
            {
                A[i*n+j] -= A[i*n+k] * A[k*n+j];
            }
        }
    }

    return SUCCESS;
}

//LR decomposition but don't override A
void **lr(mat const A, int const n, double const eps)
{
    //Copy A O(n2)
    mat R = mat_copy(A, n, n);

    //Do LR factorization on the copy O(n3)
    if (lrInp(R, n, eps) != SUCCESS)
    {
        free(R);
        return NULL;
    }
    mat L = mat_identity(n, n);

    //Split the matrices O(n2)
    for (int i=0; i<n; ++i)
    {
        for (int j=0; j<i; ++j)
        {
            L[i*n+j] = R[i*n+j];
            R[i*n+j] = 0;
        }
    }
    
    //Create a suitable return type
    return bind(2, L, R);
}

void **lr_pivot_lines(mat const A, int const n, double const eps)
{
    mat R = mat_copy(A, n, n);
    ivec p = (ivec)malloc((n+1)*sizeof(int));

    if (lr_pivot_linesInp(R, p, n, eps) != SUCCESS)
    {
        free(R);
        free(p);
        return NULL;
    }

    mat L = mat_identity(n, n);

    //Split the matrices O(n2)
    for (int i=0; i<n; ++i)
    {
        for (int j=0; j<i; ++j)
        {
            L[i*n+j] = R[i*n+j];
            R[i*n+j] = 0;
        }
    }

    return bind(3, L, R, p);
}

void **lr_pivot_cols(mat const A, int const n, double const eps)
{
    mat R = mat_copy(A, n, n);
    ivec p = (ivec)malloc((n+1)*sizeof(int));

    if (lr_pivot_colsInp(R, p, n, eps) != SUCCESS)
    {
        free(R);
        free(p);
        return NULL;
    }

    mat L = mat_identity(n, n);

    //Split the matrices O(n2)
    for (int i=0; i<n; ++i)
    {
        for (int j=0; j<i; ++j)
        {
            L[i*n+j] = R[i*n+j];
            R[i*n+j] = 0;
        }
    }

    return bind(3, L, R, p);
}