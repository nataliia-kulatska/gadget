/* Nonlinear Optimization using the algorithm of Hooke and Jeeves  */
/*  12 February 1994    author: Mark G. Johnson                    */

/* Find a point X where the nonlinear function f(X) has a local    */
/* minimum.  X is an n-vector and f(X) is a scalar.  In mathe-     */
/* matical notation  f: R^n -> R^1.  The objective function f()    */
/* is not required to be continuous.  Nor does f() need to be      */
/* differentiable.  The program does not use or require            */
/* derivatives of f().                                             */

/* The software user supplies three things: a subroutine that      */
/* computes f(X), an initial "starting guess" of the minimum point */
/* X, and values for the algorithm convergence parameters.  Then   */
/* the program searches for a local minimum, beginning from the    */
/* starting guess, using the Direct Search algorithm of Hooke and  */
/* Jeeves.                                                         */

/* This C program is adapted from the Algol pseudocode found in    */
/* "Algorithm 178: Direct Search" by Arthur F. Kaupe Jr., Commun-  */
/* ications of the ACM, Vol 6. p.313 (June 1963).  It includes the */
/* improvements suggested by Bell and Pike (CACM v.9, p. 684, Sept */
/* 1966) and those of Tomlin and Smith, "Remark on Algorithm 178"  */
/* (CACM v.12).  The original paper, which I don't recommend as    */
/* highly as the one by A. Kaupe, is:  R. Hooke and T. A. Jeeves,  */
/* "Direct Search Solution of Numerical and Statistical Problems", */
/* Journal of the ACM, Vol. 8, April 1961, pp. 212-229.            */

/* Calling sequence:                                               */
/*  int hooke(nvars, startpt, endpt, rho, epsilon, itermax)        */
/*                                                                 */
/*     nvars {an integer}                                          */
/*         This is the number of dimensions in the domain of f().  */
/*         It is the number of coordinates of the starting point   */
/*         (and the minimum point.)                                */
/*     startpt {an array of doubles}                               */
/*         This is the user-supplied guess at the minimum.         */
/*     endpt {an array of doubles}                                 */
/*         This is the calculated location of the local minimum    */
/*     rho {a double}                                              */
/*         This is a user-supplied convergence parameter (more     */
/*         detail below), which should be set to a value between   */
/*         0.0 and 1.0.  Larger values of rho give greater         */
/*         probability of convergence on highly nonlinear          */
/*         functions, at a cost of more function evaluations.      */
/*         Smaller values of rho reduces the number of evaluations */
/*         (and the program running time), but increases the risk  */
/*         of nonconvergence.  See below.                          */
/*     epsilon {a double}                                          */
/*         This is the criterion for halting the search for a      */
/*         minimum.  When the algorithm begins to make less and    */
/*         less progress on each iteration, it checks the halting  */
/*         criterion: if the stepsize is below epsilon, terminate  */
/*         the iteration and return the current best estimate of   */
/*         the minimum.  Larger values of epsilon (such as 1.0e-4) */
/*         give quicker running time, but a less accurate estimate */
/*         of the minimum.  Smaller values of epsilon (such as     */
/*         1.0e-7) give longer running time, but a more accurate   */
/*         estimate of the minimum.                                */
/*     itermax     {an integer}  A second, rarely used, halting    */
/*         criterion.  If the algorithm uses >= itermax            */
/*         iterations, halt.                                       */

/* The user-supplied objective function f(x,n) should return a C   */
/* "double".  Its  arguments are  x -- an array of doubles, and    */
/* n -- an integer.  x is the point at which f(x) should be        */
/* evaluated, and n is the number of coordinates of x.  That is,   */
/* n is the number of coefficients being fitted.                   */

/*             rho, the algorithm convergence control              */

/*  The algorithm works by taking "steps" from one estimate of     */
/*    a minimum, to another (hopefully better) estimate.  Taking   */
/*    big steps gets to the minimum more quickly, at the risk of   */
/*    "stepping right over" an excellent point.  The stepsize is   */
/*    controlled by a user supplied parameter called rho.  At each */
/*    iteration, the stepsize is multiplied by rho  (0 < rho < 1), */
/*    so the stepsize is successively reduced.                     */
/*  Small values of rho correspond to big stepsize changes,        */
/*    which make the algorithm run more quickly.  However, there   */
/*    is a chance (especially with highly nonlinear functions)     */
/*    that these big changes will accidentally overlook a          */
/*    promising search vector, leading to nonconvergence.          */
/*  Large values of rho correspond to small stepsize changes,      */
/*    which force the algorithm to carefully examine nearby points */
/*    instead of optimistically forging ahead.  This improves the  */
/*    probability of convergence.                                  */
/*  The stepsize is reduced until it is equal to (or smaller       */
/*    than) epsilon.  So the number of iterations performed by     */
/*    Hooke-Jeeves is determined by rho and epsilon:               */
/*      rho**(number_of_iterations) = epsilon                      */
/*  In general it is a good idea to set rho to an aggressively     */
/*    small value like 0.5 (hoping for fast convergence).  Then,   */
/*    if the user suspects that the reported minimum is incorrect  */
/*    (or perhaps not accurate enough), the program can be run     */
/*    again with a larger value of rho such as 0.85, using the     */
/*    result of the first minimization as the starting guess to    */
/*    begin the second minimization.                               */

/*                        Normal use:                              */
/*    (1) Code your function f() in the C language                 */
/*    (2) Install your starting guess {or read it in}              */
/*    (3) Run the program                                          */
/*    (4) {for the skeptical}: Use the computed minimum            */
/*        as the starting point for another run                    */

/* Data Fitting:                                                   */
/*  Code your function f() to be the sum of the squares of the     */
/*  errors (differences) between the computed values and the       */
/*  measured values.  Then minimize f() using Hooke-Jeeves.        */
/*  EXAMPLE: you have 20 datapoints (ti, yi) and you want to       */
/*  find A,B,C such that  (A*t*t) + (B*exp(t)) + (C*tan(t))        */
/*  fits the data as closely as possible.  Then f() is just        */
/*  f(x) = SUM (measured_y[i] - ((A*t[i]*t[i]) + (B*exp(t[i]))     */
/*                + (C*tan(t[i]))))^2                              */
/*  where x[] is a 3-vector consisting of {A, B, C}.               */

/*  The author of this software is M.G. Johnson.                   */
/*  Permission to use, copy, modify, and distribute this software  */
/*  for any purpose without fee is hereby granted, provided that   */
/*  this entire notice is included in all copies of any software   */
/*  which is or includes a copy or modification of this software   */
/*  and in all copies of the supporting documentation for such     */
/*  software.  THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT    */
/*  ANY EXPRESS OR IMPLIED WARRANTY.  IN PARTICULAR, NEITHER THE   */
/*  AUTHOR NOR AT&T MAKE ANY REPRESENTATION OR WARRANTY OF ANY     */
/*  KIND CONCERNING THE MERCHANTABILITY OF THIS SOFTWARE OR ITS    */
/*  FITNESS FOR ANY PARTICULAR PURPOSE.                            */

#include "gadget.h"    //All the required standard header files are in here
#include "mathfunc.h"

#define VARS   (350)   //Maximum number of variables.

double oldf = 0.0;

/* global variable --- defined, initialized in main.cc. */
/* Its value is not changed in this file.               */
extern int FuncEval;

/* given a point, look for a better one nearby, one coord at a time */
double best_nearby(double (*f)(double*, int), double delta[], double point[],
  double prevbest, int nvars, int param[]) {

  double       z[VARS];
  double       minf, ftmp;
  int          i;

  minf = prevbest;
  for (i = 0; i < nvars; i++)
    z[i] = point[i];
  for (i = 0; i < nvars; i++) {
    z[param[i]] = point[param[i]] + delta[param[i]];
    ftmp = (*f)(z, nvars);
    if (ftmp < minf)
      minf = ftmp;
    else {
      delta[param[i]] = 0.0 - delta[param[i]];
      z[param[i]] = point[param[i]] + delta[param[i]];
      ftmp = (*f)(z, nvars);
      if (ftmp < minf)
        minf = ftmp;
      else
        z[param[i]] = point[param[i]];
    }
  }
  for (i = 0; i < nvars; i++)
    point[i] = z[i];
  return(minf);
}

int hooke(double (*f)(double* , int), int nvars, double startpt[], double endpt[],
  double upperb[], double lowerb[], double rho, double lambda, double epsilon,
  int itermax) {

  double delta[VARS];
  double newf, fbefore, steplength, tmp;
  double xbefore[VARS], newx[VARS];
  int    i, j, k, h, keep, change, nobds;
  int    iters, iadj;
  int    param[VARS];

  int    lbounds[VARS];     //counts how often it has hit the lowerbounds
  int    rbounds[VARS];     //counts how often it has hit the upperbounds
  double initialstep[VARS]; //the stepsize when it hits the bound first
  int    trapped[VARS];     // = 1 if it is trapped at a bound else = 0

  for (i = 0; i < nvars; i++) {
    lbounds[i] = 0;
    rbounds[i] = 0;
    trapped[i] = 0;
    param[i] = i;
    newx[i] = xbefore[i] = startpt[i];
    delta[i] = absolute(startpt[i] * rho);
    if (delta[i] == 0.0)
      delta[i] = rho;
  }
  steplength = ((lambda <= 0) ? rho : lambda);
  fbefore = (*f)(newx, nvars);
  newf = fbefore;
  iadj = 0;
  nobds = 0;
  iters = 0;
  h = 0;

  cout << "\nStarting Hooke and Jeeves\n";
  while ((iters < itermax) && (steplength > epsilon)) {
    iters++;
    iadj++;

    /* JMB added check for really silly values */
    if (iszero(fbefore)) {
      cout << "\nError in Hooke and Jeeves optimisation after " << FuncEval
       << " function evaluations f(x) = 0\nReturning to calling routine ...\n";
      return(0);
    }

    cout << "\nNew optimum after " << FuncEval << " function evaluations, f(x) = "
      << fbefore << " at\n";
    for (j = 0; j < nvars; j++)
      cout << xbefore[j] << sep;
    cout << endl;

    if (iters % (15 * nvars) == 0) {
      /* Randomize the order of the parameters once in a while, to avoid */
      /* the order having an influence on which changes are accepted.    */
      change = 0;
      while (change < nvars) {
        h = rand() % nvars;
        k = 1;
        for (i = 0; i < change; i++)
          if (param[i] == h)
            k = 0;
        if (k) {
          param[change] = h;
          change++;
        }
      }
    }

    /* find best new point, one coord at a time */
    for (i = 0; i < nvars; i++) {
      newx[i] = xbefore[i];
    }
    newf = best_nearby(f, delta, newx, fbefore, nvars, param);

    /* if we made some improvements, pursue that direction */
    keep = 1;
    while ((newf < fbefore) && (keep == 1)) {
      for (i = 0; i < nvars; i++) {
        /* if it has been trapped but f has now gotten better by 5%  */
        /* we assume that we are out of the trap, reset the counters */
        /* and go back to the stepsize we had when we got trapped    */
        if ((trapped[i] == 1) && (newf < oldf * 0.95)) {
          trapped[i] = 0;
          lbounds[i] = 0;
          rbounds[i] = 0;
          delta[i] = initialstep[i];
        }
        if (newx[i] < lowerb[i]) {
          nobds++;
          lbounds[i]++;        /* counts how often it hits a lower bound */
          if (trapped[i] == 0) {
            initialstep[i] = delta[i];
            trapped[i] = 1;
          }
          /* if it has hit the bounds 2 times then increase the stepsize */
          if (lbounds[i] >= 2)
            delta[i] += rho * 10;

        } else if (newx[i] > upperb[i]) {
          nobds++;
          rbounds[i]++;        /* counts how often it hits a upper bound */
          if (trapped[i] == 0) {
            initialstep[i] = delta[i];
            trapped[i] = 1;
          }
          /* if it has hit the bounds 2 times then increase the stepsize */
          if (rbounds[i] >= 2)
            delta[i] += rho * 10;
        }
      }

      oldf = newf;
      iadj = 0;
      for (i = 0; i < nvars; i++) {
        /* firstly, arrange the sign of delta[] */
        if (newx[i] <= xbefore[i])
          delta[i] = 0.0 - absolute(delta[i]);
        else
          delta[i] = absolute(delta[i]);

        /* now, move further in this direction  */
        tmp = xbefore[i];
        xbefore[i] = newx[i];
        newx[i] = newx[i] + newx[i] - tmp;
      }

      /* only move forward if this is really an improvement[GS] */
      fbefore = newf;
      newf =  (*f)(newx, nvars);
      if (newf >= fbefore)
        break;

      /*  OK, it's better, so update variables and look around  */
      fbefore = newf;
      for (i = 0; i < nvars; i++)
        xbefore[i] = newx[i];
      newf = best_nearby(f, delta, newx, fbefore, nvars, param);

      /* if the further (optimistic) move was bad */
      if (newf >= fbefore)
        break;

      /* make sure that the differences between the new and the */
      /* old points are due to actual displacements - beware of */
      /* roundoff errors that might cause newf < fbefore        */
      keep = 0;
      for (i = 0; i < nvars; i++) {
        keep = 1;
        if (absolute(newx[i] - xbefore[i]) > (0.5 * absolute(delta[i])))
          break;
        else
          keep = 0;
      }
    }

    if ((steplength >= epsilon) && (newf >= fbefore)) {
      steplength = steplength * rho;
      for (i = 0; i < nvars; i++) {
        delta[i] *= rho;
      }
    }
  }

  /* JMB 19.06.02 - adding comments for end of optimisation */
  cout << "\nHooke and Jeeves optimisation completed after " << iters
    << " iterations (max " << itermax << ")\nThe bounds were hit " << nobds
    << " times\nThe steplength was reduced to " << steplength << " (min " << epsilon << ")\n";

  if (iters == itermax)
    cout << "The model terminated because the maximum number of iterations was reached\n"
      << "An optimum has NOT been found for this run\n";
  else
    cout << "The model terminated because is has converged to an optimum\n";

  /* THLTH 30.08.01: Can't see why only checked if iters >= itermax */
  //if (iters >= itermax)
  if (newf > fbefore)
    for (i = 0; i < nvars; i++)
      newx[i] = xbefore[i];

  newf = (*f)(newx, nvars);
  for (i = 0; i < nvars; i++)
    endpt[i] = newx[i];
  return(iters);
}
