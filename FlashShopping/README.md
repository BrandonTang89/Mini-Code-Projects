# FlashShopping
Code for IMMC 2020

IMMC 2020 has its task on predicting the behaviour of shoppers during a flash sale and the level of damage to the products that these behaviours cause. Furthermore, participants are tasked to come up with a new store layout to reduce the level of damage to the shop.
*More information available in at /Documents/2020_IMMC_Problem.pdf*

This repository contains code that conducts a Python 3.7 simulation ot meet the requirements of the task.

## Information About Simulation
Please read the full report in /Documents/TemaekJuniorCollege_TeamReport2_2020.pdf for information

The simulation conducted is of shoppers walking to grab products off shelves during flash sale. We use the A* algorithm to simulate the shoppers and monitor their movements in terms of the size of products they purchase as well as the price of items they purchase to get an accurate map of where in the shop exist a high density of shoppers.

## Organisation of Repository
"IMMC Code 2.ipynb" contains code to run a simulation with debugging output

"IMMC Layout Evaluation Code.ipynb" contains code that runs a simulation and nicely presents the output variables of a test layout.

"Store Layout Generator.ipynb" contains code that generates layouts following Figure 1 but with the "Appliances" and the "TV and Home Theatre" sections being swapped to various places.


## Dependencies
Python 3.7+ is required to run the simulation. The required python libraries can be installed using
<pre>pip3 install -r requirements.txt</pre>
