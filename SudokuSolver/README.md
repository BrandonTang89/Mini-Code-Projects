# SudokuSolver
A Python application to solve the standard 9 by 9 Sudoku puzzle. <br>
Both GUI and CLI versions available.

<b>Input Format</b><br>
<ol>
  <li>The input Sudoku board is written as a 9x9 grid of integers in range [0,9].<br></li>
  <li>Rows are separated by a newline character (enter).<br></li>
  <li>0 represents an empty square.<br></li>
</ol>

Example:
<pre>
240000009
090031000
001590080
000000076
000803000
960000000
030045100
000620090
600000052
</pre>

<b>Mechanism</b><br>
Solves via recursion.<br>
Upon attempting to solve a Sudoku board:
<ul>
  <li>Check if the board is already solved (base case of recursion)</li>
  <li>Find the different possibilities of each empty square</li>
  <li>Try substituting the square with the least possiblilies with each of its possibilities and solving the newly subsituted board</li>
</ul>

We can quantify the difficulty of a specific Sudoku puzzle by counting how many substitutions are needed for the program to solve it :)
  
<b>Note: Python 3.7+ and numpy are required for running the .py or .pyw version.</b><br>
Numpy can be installed with:
<pre>pip install numpy</pre>

<b>More Examples of Input:</b>
<pre>
764000100
000070806
809604030
000800063
090351020
580006000
050902301
902010000
006000289
</pre>
<pre>
063400000
200000090
000870106
000040300
420000057
007080000
902056000
050000009
000008570
</pre>
