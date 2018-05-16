Documentation for Warmup Assignment 1
=====================================

+-------+
| BUILD |
+-------+

My My402List.c program is to define the functions so that we can implement the list
in the part b, all the requirements from the websites have been successfully satisfied.

My warmup1.c program consists of four functions, the main function is to collect the statement of command line, it 
will then check if the user enter the command in correct format. It will also call the other three functions
respectively. The process function is to read the transaction elements within the tfile, and then check if the 
contents of the tfile's elements display in proper format. After the function ran, it will then store all the 
transaction information into a list. Then, sorting function calls the list, and then sort the list based on the 
data of the transaction. Finally, the printList function print the elements of the list in a formatted table so
that we can look at the final result.

+-----------------+
| SKIP (Optional) |
+-----------------+

No.

Is there are any tests in the standard test suite that you know that it's not
working and you don't want the grader to run it at all so you won't get extra
deductions, please list them here.  (Of course, if the grader won't run these
tests, you will not get plus points for them.)

+---------+
| GRADING |
+---------+

100

(A) Doubly-linked Circular List : 40

(B.1) Sort (file) : 30
(B.2) Sort (stdin) : 30

Missing required section(s) in README file : No
Cannot compile : No
Compiler warnings : No
"make clean" : No
Segmentation faults : No
Separate compilation : No
Malformed input : No
Too slow : No
Bad commandline : No
Did not use My402List and My402ListElem to implement "sort" in (B) : No

+------+
| BUGS |
+------+

No

+------------------+
| OTHER (Optional) |
+------------------+

Comments on design decisions: No
Comments on deviation from spec: No