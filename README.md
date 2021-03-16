# ALGORITHMS AND DATA STRUCTURES PROJECT 2020

This is the final project of the course "Algoritmi e principi dell'informatica" ("Algorithms and principles of computer science") in 2020 at Politecnico di Milano.


| TestSetName | Passed | Number of Tests | time execution[^1] | RAM space[^2]|
| ----------- | ----------- | ----------- | ----------- | ----------- |
| WriteOnly | 5 | 5 | 7,4 s | 3.00 GiB |
| BulkReads | 5 | 5 | 1.250 s | 240 MiB |
| TimeForAChange | 5 | 5 | 2.100 s | 540 MiB |
| RollingBack | 3 | 5 | 2.100 s | 550 MiB |
| Alteringhistory | 5 | 5 | 2.100 s | 400 MiB |
| RollerCoaster | 5 | 5 | 2.700 s | 1.03 GiB |
| Laude | 1 | 1 | 2.000 s | 340 MiB |

Final score 28/30

[^1]: max time execution to pass a test
[^2]: max RAM space used during the execution to pass a test
---
##Little description of tests[^3]
WriteOnly
: it stresses writings at the bottom of the text
BulkReads
: it stresses line's readings and changes
TimeForAChange
: it stresses delete operations
RollingBack
: undo and redo operations only at the end
AlteringHistory
: it stresses all the operations
RollerCoaster
: it stresses all the operations
Laude
: it stresses all the operations

[^3]: test sets were uknown during the development of the code. 
