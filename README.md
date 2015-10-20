This is the in-progress code for Mob, a distributed computing framework based on autonomic computing principles and swarm intelligence.

It was developed for a course project at Memorial University of Newfoundland. The report may be found [here](http://dac456.webfactional.com/dl/reports/EN9861_Report.pdf).

It depends on Boost libraries (ASIO, Threading, Serialization, Filesystem). 'mobtest_host' depends on [Canis](http://www.github.com/dac456/canis) for rendering.

It is in an unfinished and unorganized state, but portions may still be useful to others in its current form.

### TODO
* 'Host' needs to be renamed to 'Client' throughout the code.
* Not 100% sure if the nearest-neighbours approach to filtering particle interaction in mobtest actually works.
* Commenting & documentation (!!!)
* More error handling and fault-tolerance mechanisms
* Refactoring and code cleanup