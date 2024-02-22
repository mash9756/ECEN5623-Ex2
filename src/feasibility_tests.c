// Sam Siewert, August 2020
//
// This example code provides feasibiltiy decision tests for single core fixed priority rate monontic systems only (not dyanmic priority such as deadline driven
// EDF and LLF).  These are standard algorithms which either estimate feasibility (as the RM LUB does) or automate exact analysis (scheduling point, completion test) for
// a set services sharing one CPU core.  This can be emulated on Linux SMP multi-core systemes by use of POSIX thread affinity, to "pin" a thread to a specific core.
//
// Coded based upon standard definition of:
//
// 1) RM LUB based upon model by Liu and Layland
// 2) Scheduling Point - an exact feasibility algorithm based upon Lehoczky, Sha, and Ding exact analysis
// 3) Completion Test - an exact feasibility algorithm
//
// All 3 are also covered in RTECS with Linux and RTOS p. 84 to p. 89
//
// Original references for single core AMP systems:
//
// 1) RM LUB - Liu, Chung Laung, and James W. Layland. "Scheduling algorithms for multiprogramming in a hard-real-time environment." Journal of the ACM (JACM) 20.1 (1973): 46-61.
// 2) Scheduling Point - Lehoczky, John, Lui Sha, and Yuqin Ding. "The rate monotonic scheduling algorithm: Exact characterization and average case behavior." RTSS. Vol. 89. 1989.
// 3) Completion Test - Joseph, Mathai, and Paritosh Pandya. "Finding response times in a real-time system." The Computer Journal 29.5 (1986): 390-395.
//
// References for mulit-core systems:
//
// 1) Bertossi, Alan A., Luigi V. Mancini, and Federico Rossini. "Fault-tolerant rate-monotonic first-fit scheduling in hard-real-time systems."
//    IEEE Transactions on Parallel and Distributed Systems 10.9 (1999): 934-945.
// 2) Burchard, Almut, et al. "New strategies for assigning real-time tasks to multiprocessor systems." IEEE transactions on computers 44.12 (1995): 1429-1442.
// 3) Dhall, Sudarshan K., and Chung Laung Liu. "On a real-time scheduling problem." Operations research 26.1 (1978): 127-140.
//
//
// Deadline Montonic (not implemented in this example, but covered in class and notes):
//
// 1) Audsley, Neil C., et al. "Hard real-time scheduling: The deadline-monotonic approach." IFAC Proceedings Volumes 24.2 (1991): 127-132.
//
// Note that Deadline Monotoic simply uses the deadine interval, D(i) to assign priority, rather than the period interval, T(i) and relaxes T=D constraint.  Anlaysis can
// be done as it is done for RM, but with evaluation of feasbility based upon modified D(i) and with modified fixed priorities.  This is covered by manual analysis examples.
//
// For a more interactive tool, students can use Cheddar:
//
// http://beru.univ-brest.fr/~singhoff/cheddar/
//
// This open source tool handles single and multi-core and allows for modeling of the platform hardware, RTOS/OS, and scheduler with a particular fixed priority or dynamic
// priority policy.
//
// This code is provided primarily so students can learn the methods of worst case analysis and compare exact and estimated feasibility decision testing.
//

#include <math.h>
#include <stdio.h>

#define TRUE 1
#define FALSE 0
#define U32_T unsigned int

// U=0.7333
U32_T ex0_period[] = {2, 10, 15};
U32_T ex0_wcet[] = {1, 1, 2};

// U=0.9857
U32_T ex1_period[] = {2, 5, 7};
U32_T ex1_wcet[] = {1, 1, 2};

// U=0.9967
U32_T ex2_period[] = {2, 5, 7, 13};
U32_T ex2_wcet[] = {1, 1, 1, 2};

// U=0.93
U32_T ex3_period[] = {3, 5, 15};
U32_T ex3_wcet[] = {1, 2, 3};

// U=1.0
U32_T ex4_period[]  = {2, 4, 16};
U32_T ex4_wcet[]    = {1, 1, 4};

U32_T ex5_period[]  = {2, 5, 10};
U32_T ex5_wcet[]    = {1, 2, 1};

U32_T ex6_period[]  = {2, 5, 7, 13};
U32_T ex6_wcet[]    = {1, 1, 1, 2};

U32_T ex7_period[]  = {3, 5, 15};
U32_T ex7_wcet[]    = {1, 2, 4};

U32_T ex8_period[]  = {2, 5, 7, 13};
U32_T ex8_wcet[]    = {1, 1, 1, 2};

U32_T ex9_period[]  = {6, 8, 12, 24};
U32_T ex9_wcet[]    = {1, 2, 4, 6};

int completion_time_feasibility(U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[]);
int scheduling_point_feasibility(U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[]);
int rate_monotonic_least_upper_bound(U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[]);

void print_test_results(U32_T numServices, U32_T period[], U32_T wcet[], double util){

    printf("\nCompletion Time:  ");
    if(completion_time_feasibility(numServices, period, wcet, period) == TRUE)
        printf("FEASIBLE\n");
    else
        printf("INFEASIBLE\n");

    printf("Scheduling Point: ");
    if(scheduling_point_feasibility(numServices, period, wcet, period) == TRUE)
        printf("FEASIBLE\n\n");
    else
        printf("INFEASIBLE\n\n");

    if(rate_monotonic_least_upper_bound(numServices, period, wcet, period) == TRUE)
        printf("\nRM LUB: FEASIBLE\n");
    else
        printf("\nRM LUB: INFEASIBLE\n");

    printf("EDF: \t");
    if(util < 100)
        printf("FEASIBLE\n");
    else
        printf("INFEASIBLE\n");

    printf("LLF: \t");
    if(util < 100)
        printf("FEASIBLE\n");
    else
        printf("INFEASIBLE\n");
}

int main(void)
{ 
    double utilization  = 0;
	U32_T numServices   = 0;

/*****************************************************************************************************************/
    utilization = (double) ((double)((double)(ex0_wcet[0]/(double)ex0_period[0]) * 100) +
                            (double)((double)(ex0_wcet[1]/(double)ex0_period[1]) * 100) +
                            (double)(((double)ex0_wcet[2]/(double)ex0_period[2]) * 100));
    numServices = 3;

    printf("************************************************************************\n");
    printf("Ex-0 U=%4.2f\% (C1=%d, C2=%d, C3=%d; T1=%d, T2=%d, T3=%d; T=D)",
            utilization, ex0_wcet[0], ex0_wcet[1], ex0_wcet[2], 
            ex0_period[0], ex0_period[1], ex0_period[2]);

    print_test_results(numServices, ex0_period, ex0_wcet, utilization);
/*****************************************************************************************************************/

/*****************************************************************************************************************/
    utilization = (double) ((double)((double)(ex1_wcet[0]/(double)ex1_period[0]) * 100) +
                            (double)((double)(ex1_wcet[1]/(double)ex1_period[1]) * 100) +
                            (double)(((double)ex1_wcet[2]/(double)ex1_period[2]) * 100));
    numServices = 3;

    printf("************************************************************************\n");
    printf("Ex-1 U=%4.2f\% (C1=%d, C2=%d, C3=%d; T1=%d, T2=%d, T3=%d; T=D)",
            utilization, ex1_wcet[0], ex1_wcet[1], ex1_wcet[2], 
            ex1_period[0], ex1_period[1], ex1_period[2]);

    print_test_results(numServices, ex1_period, ex1_wcet, utilization);
/*****************************************************************************************************************/

/*****************************************************************************************************************/
    utilization = (double) ((double)((double)(ex2_wcet[0]/(double)ex2_period[0]) * 100) +
                            (double)((double)(ex2_wcet[1]/(double)ex2_period[1]) * 100) +
                            (double)(((double)ex2_wcet[2]/(double)ex2_period[2]) * 100) +
                            (double)(((double)ex2_wcet[3]/(double)ex2_period[3]) * 100));
    numServices = 4;

    printf("************************************************************************\n");
    printf("Ex-2 U=%4.2f\% (C1=%d, C2=%d, C3=%d, C4=%d; T1=%d, T2=%d, T3=%d, T4=%d; T=D)",
            utilization, ex2_wcet[0], ex2_wcet[1], ex2_wcet[2], ex2_wcet[3],
            ex2_period[0], ex2_period[1], ex2_period[2], ex2_period[3]);

    print_test_results(numServices, ex2_period, ex2_wcet, utilization);
/*****************************************************************************************************************/

/*****************************************************************************************************************/
    utilization = (double) ((double)((double)(ex3_wcet[0]/(double)ex3_period[0]) * 100) +
                            (double)((double)(ex3_wcet[1]/(double)ex3_period[1]) * 100) +
                            (double)(((double)ex3_wcet[2]/(double)ex3_period[2]) * 100));
    numServices = 3;

    printf("************************************************************************\n");
    printf("Ex-3 U=%4.2f\% (C1=%d, C2=%d, C3=%d; T1=%d, T2=%d, T3=%d; T=D)",
            utilization, ex3_wcet[0], ex3_wcet[1], ex3_wcet[2], 
            ex3_period[0], ex3_period[1], ex3_period[2]);

    print_test_results(numServices, ex3_period, ex3_wcet, utilization);
/*****************************************************************************************************************/

/*****************************************************************************************************************/
    utilization = (double) ((double)((double)(ex4_wcet[0]/(double)ex4_period[0]) * 100) +
                            (double)((double)(ex4_wcet[1]/(double)ex4_period[1]) * 100) +
                            (double)(((double)ex4_wcet[2]/(double)ex4_period[2]) * 100));
    numServices = 3;

    printf("************************************************************************\n");
    printf("Ex-4 U=%4.2f\% (C1=%d, C2=%d, C3=%d; T1=%d, T2=%d, T3=%d; T=D)",
            utilization, ex4_wcet[0], ex4_wcet[1], ex4_wcet[2], 
            ex4_period[0], ex4_period[1], ex4_period[2]);

    print_test_results(numServices, ex4_period, ex4_wcet, utilization);
/*****************************************************************************************************************/

/*****************************************************************************************************************/
    utilization = (double) ((double)((double)(ex5_wcet[0]/(double)ex5_period[0]) * 100) +
                            (double)((double)(ex5_wcet[1]/(double)ex5_period[1]) * 100) +
                            (double)(((double)ex5_wcet[2]/(double)ex5_period[2]) * 100));
    numServices = 3;

    printf("************************************************************************\n");
    printf("Ex-5 U=%4.2f\% (C1=%d, C2=%d, C3=%d; T1=%d, T2=%d, T3=%d; T=D)",
            utilization, ex5_wcet[0], ex5_wcet[1], ex5_wcet[2], 
            ex5_period[0], ex5_period[1], ex5_period[2]);

    print_test_results(numServices, ex5_period, ex5_wcet, utilization);
/*****************************************************************************************************************/

/*****************************************************************************************************************/
    utilization = (double) ((double)((double)(ex6_wcet[0]/(double)ex6_period[0]) * 100) +
                            (double)((double)(ex6_wcet[1]/(double)ex6_period[1]) * 100) +
                            (double)((double)(ex6_wcet[2]/(double)ex6_period[2]) * 100) +
                            (double)(((double)ex6_wcet[3]/(double)ex6_period[3]) * 100));
    numServices = 4;
    printf("************************************************************************\n");
    printf("Ex-6 U=%4.2f\% (C1=%d, C2=%d, C3=%d C4=%d; T1=%d, T2=%d, T3=%d T4=%d; T=D): ",
            utilization, ex6_wcet[0], ex6_wcet[1], ex6_wcet[2], ex6_wcet[3],
            ex6_period[0], ex6_period[1], ex6_period[2], ex6_period[3]);

    print_test_results(numServices, ex6_period, ex6_wcet, utilization);
/*****************************************************************************************************************/

/*****************************************************************************************************************/
    utilization = (double) ((double)((double)(ex7_wcet[0]/(double)ex7_period[0]) * 100) +
                            (double)((double)(ex7_wcet[1]/(double)ex7_period[1]) * 100) +
                            (double)(((double)ex7_wcet[2]/(double)ex7_period[2]) * 100));
    numServices = 3;
    printf("************************************************************************\n");
    printf("Ex-7 U=%4.2f\% (C1=%d, C2=%d, C3=%d; T1=%d, T2=%d, T3=%d; T=D): ",
            utilization, ex7_wcet[0], ex7_wcet[1], ex7_wcet[2],
            ex7_period[0], ex7_period[1], ex7_period[2]);

    print_test_results(numServices, ex7_period, ex7_wcet, utilization);
/*****************************************************************************************************************/

/*****************************************************************************************************************/
    utilization = (double) ((double)((double)(ex8_wcet[0]/(double)ex8_period[0]) * 100) +
                            (double)((double)(ex8_wcet[1]/(double)ex8_period[1]) * 100) +
                            (double)((double)(ex8_wcet[2]/(double)ex8_period[2]) * 100) +
                            (double)((double)(ex8_wcet[3]/(double)ex8_period[3]) * 100));
    numServices = 4;
    printf("************************************************************************\n");
    printf("Ex-8 U=%4.2f\% (C1=%d, C2=%d, C3=%d C4=%d; T1=%d, T2=%d, T3=%d T4=%d; T=D): ",
            utilization, ex8_wcet[0], ex8_wcet[1], ex8_wcet[2], ex8_wcet[3],
            ex8_period[0], ex8_period[1], ex8_period[2], ex8_period[3]);

    print_test_results(numServices, ex8_period, ex8_wcet, utilization);
/*****************************************************************************************************************/

/*****************************************************************************************************************/
    utilization = (double) ((double)((double)(ex9_wcet[0]/(double)ex9_period[0]) * 100) +
                            (double)((double)(ex9_wcet[1]/(double)ex9_period[1]) * 100) +
                            (double)((double)(ex9_wcet[2]/(double)ex9_period[2]) * 100) +
                            (double)((double)(ex9_wcet[3]/(double)ex9_period[3]) * 100));
    numServices = 4;    
    printf("************************************************************************\n");
    printf("Ex-9 U=%4.2f\% (C1=%d, C2=%d, C3=%d C4=%d; T1=%d, T2=%d, T3=%d T4=%d; T=D)",
            utilization, ex9_wcet[0], ex9_wcet[1], ex9_wcet[2], ex9_wcet[3],
            ex9_period[0], ex9_period[1], ex9_period[2], ex9_period[3]);

    print_test_results(numServices, ex9_period, ex9_wcet, utilization);
    printf("************************************************************************\n");

/*****************************************************************************************************************/
}


int rate_monotonic_least_upper_bound(U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[])
{
  double utility_sum=0.0, lub=0.0;
  int idx;
  // Sum the C(i) over the T(i)
  for(idx=0; idx < numServices; idx++)
  {
    utility_sum += ((double)wcet[idx] / (double)period[idx]);
    printf("for %d, wcet=%lf, period=%lf, utility_sum = %lf\n", idx, (double)wcet[idx], (double)period[idx], utility_sum);
  }
  printf("utility_sum = %lf\n", utility_sum);

  // Compute LUB for number of services
  lub = (double)numServices * (pow(2.0, (1.0/((double)numServices))) - 1.0);
  printf("LUB = %lf\n", lub);

  // Compare the utilty to the bound and return feasibility
  if(utility_sum <= lub)
	  return TRUE;
  else
	  return FALSE;
}


int completion_time_feasibility(U32_T numServices, U32_T period[], U32_T wcet[], U32_T deadline[])
{
  int i, j;
  U32_T an, anext;
  
  // assume feasible until we find otherwise
  int set_feasible=TRUE;
   
  //printf("numServices=%d\n", numServices);
 
  // For all services in the analysis 
  for (i=0; i < numServices; i++)
  {
       an=0; anext=0;
       
       for (j=0; j <= i; j++)
       {
           an+=wcet[j];
       }
       
	   //printf("i=%d, an=%d\n", i, an);

       while(1)
       {
             anext=wcet[i];
	     
             for (j=0; j < i; j++)
                 anext += ceil(((double)an)/((double)period[j]))*wcet[j];
		 
             if (anext == an)
                break;
             else
                an=anext;

			 //printf("an=%d, anext=%d\n", an, anext);
       }
       
	   //printf("an=%d, deadline[%d]=%d\n", an, i, deadline[i]);

       if (an > deadline[i])
       {
          set_feasible=FALSE;
       }
  }
  
  return set_feasible;
}


int scheduling_point_feasibility(U32_T numServices, U32_T period[], 
				 U32_T wcet[], U32_T deadline[])
{
   int rc = TRUE, i, j, k, l, status, temp;

   // For all services in the analysis
   for (i=0; i < numServices; i++) // iterate from highest to lowest priority
   {
      status=0;

      // Look for all available CPU minus what has been used by higher priority services
      for (k=0; k<=i; k++) 
      {
	  // find available CPU windows and take them
          for (l=1; l <= (floor((double)period[i]/(double)period[k])); l++)
          {
               temp=0;

               for (j=0; j<=i; j++) temp += wcet[j] * ceil((double)l*(double)period[k]/(double)period[j]);

	       // Can we get the CPU we need or not?
               if (temp <= (l*period[k]))
			   {
				   // insufficient CPU during our period, therefore infeasible
				   status=1;
				   break;
			   }
           }
           if (status) break;
      }

      if (!status) rc=FALSE;
   }
   return rc;
}