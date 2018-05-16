Documentation for Warmup Assignment 2
=====================================

+-------+
| BUILD |
+-------+

Comments: Just type "make" to compile

+---------+
| GRADING |
+---------+

Basic running of the code : 100

Missing required section(s) in README file : No
Cannot compile : No
Compiler warnings : No
"make clean" : Yes
Segmentation faults : No
Separate compilation : Yes
Using busy-wait : No
Handling of commandline arguments:
    1) -n : Yes
    2) -lambda : Yes
    3) -mu : Yes
    4) -r : Yes
    5) -B : Yes
    6) -P : Yes
Trace output :
    1) regular packets: Yes
    2) dropped packets: Yes
    3) removed packets: Yes
    4) token arrival (dropped or not dropped): Yes
Statistics output :
    1) inter-arrival time : Yes
    2) service time : Yes
    3) number of customers in Q1 : Yes
    4) number of customers in Q2 : Yes
    5) number of customers at a server : Yes
    6) time in system : Yes
    7) standard deviation for time in system : Yes
    8) drop probability : Yes
Output bad format : No
Output wrong precision for statistics (should be 6-8 significant digits) : No
Large service time test : Yes
Large inter-arrival time test : Yes
Tiny inter-arrival time test : Yes
Tiny service time test : Yes
Large total number of customers test : Yes
Large total number of customers with high arrival rate test : Yes
Dropped tokens test : Yes
Cannot handle <Cntrl+C> at all (ignored or no statistics) : No
Can handle <Cntrl+C> but statistics way off : No
Not using condition variables and do some kind of busy-wait : No
Synchronization check : No
Deadlocks : No

+----------------------+
| BUGS / TESTS TO SKIP |
+----------------------+

Is there are any tests in the standard test suite that you know that it's not
working and you don't want the grader to run it at all so you won't get extra
deductions, please list them here.  (Of course, if the grader won't run these
tests, you will not get plus points for them.)

Comments: No

+------------------+
| OTHER (Optional) |
+------------------+

Comments on design decisions: No
Comments on deviation from spec: No
