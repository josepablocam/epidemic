epidemic.c is a (very naive) simulation for how a disease could spread through a population. A population consists of a large array of persons (nodes), which in turn are connected to other persons (i.e. have edges). We construct a disease based on a limited set of parameters explained below. We then simulate a series of days of the disease and output to the terminal a csv table that tracks the number of people dead, sick and healthy each day.


There is absolutely no medical or epidemiological basis behind this simulator, but rather was just a fun exercises for me to practice C.

Command line parameters that must be passed in:
 -n size of the population (100k seems to be a good number, 1MM is doable but very slow)
 -d duration of the simulation (days), each represents one iteration of the simulator
 -c connectivity of your population (i.e. maximum number of edges for a given node)
 -b baseline infection rate for entirety of simulation, i.e. chance of someone just randomly getting sick during the entire simulation
 -e exposure infection rate, chance of sickness if someone in your network is sick
 -m mortality rate during sickness period
 -l length of sickness


There are too many simplifying assumptions to state here, but some might be worthwhile highlighting:
1 - you can only infect others when you are sick. However, there is no distinction between the first day of your sickness and the last, i.e. your "infectiousness" is independent of how progressed your disease is. The same is true for mortality. You are just as likely to die on the first day as on the last.

2 - There is no resistance developed to the disease. If you were sick, and became healthy, your odds of getting sick again are the same.

3 - People cannot get sick and infect others in the same simulator iteration (ie. 1 day), but they can certainly infect someone first and then die in the same interation.



Some fun additions I might make at some point:
1 - add events (eg. vaccine is developed at day X, reducing mortality and infection rate to Y).
2 - allowing a latent stage for the disease, where you are infectious but no symptoms present. People in your network would then avoid you during your symptomatic stage but be none the wiser prior to that.


I've also included a very brief R script that compares the effects of 3 different diseases on a population.


To build:
just call make in your directory. It will output one executable, simulator.



