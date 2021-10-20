# SGMRTAPSP
All-Pair-Shortest-Path Algorithm on the Singapore MRT System 

<b>Mechanism</b>
<ul>
  <li>Utilises Floyd Warshall's All-Pair Shortest Path on the Singapore MRT System</li>
  <li>Stations are taken as nodes</li>
  <li>Time taken for train to between adjacent stations are weighted edges</li>
  <li>Interchange stations between 2 lines are split into separated into stations with the walk time between them being a weighted edge</li>
  <li>Time the train stops at each stop is considered in the algorithm</li>
  <li>Time for each edge and time at each stop is measured in person with a stopwatch</li>
  <li>Stations are labelled according to the Singapore MRT map (in repository)</li>
</ul>

<b>Limitations</b>
<ul>
  <li>Time for waiting for train not accounted for</li>
  <li>Difference between peak and non-peak not accounted for</li>
  <li>Walking pace while measuring not fully consistent</li>
  <li>Time between specific train lines at stops are merely average values; true values depend on direction of walking; direction of train  being taken and etc</li>
</ul>

Command to compile (requires g++):<br>
<pre>g++ "MRT APSP.cpp"</pre> 
