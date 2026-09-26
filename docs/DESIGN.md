## improvements


Hierarchy for improvements:

```
world
 ^
 |
region  civ
 ^       ^
 |       |
cell    city
 ^       ^
 |       |
improvement
```

What is connection between improvement and city ?
Should all improvements belong to the city ? Should they be built only
on the regions that belong to the city?

User scenarios:
1. Normal city. It has population, user can build all improvements that has jobs
   User should also be able to build fortress-related improvements in the city.

2. Military fortress.
   User can build fortifications and military buildings in fortress.
   Fortress requires maintenance. Should maintenance be different from normal
   city?
   If fortress is in the mountains, why user should not be able to build an
   iron mine in the same region ?
   What is the difference between military fortress city and normal city
   with fortress building.

3. Trade caravansarai
   Used to extend trade routes. Same question - what is the principal difference
   between normal city with trade buildings and trade city ?


So, upon consideration: there is no need for different type of cities.
City 'focus', if needed, can be implemented as separate property in game
rules.

Ruleset:
 * Any city can build any improvement.
 * Improvement belongs to the city. Not the cell (cell can have multiple
   improvements). This allows for some 'hidden' improvements, like buried
   ancient temple
 * Open question: improvement belonging to 'civ' itself - like remnant
   of the ancient civilization. Most likely solution - either one 'virtual' city
   for all such improvements, or one city for every cell with such improvement


## city and region

Hierarchy is

```
civ      world
 |        |
city -> region1
        region2
```

That is city can contain one or mulitple regions

*Open question - should 'city' be proper part of 'region' hierarchy*

Motivation: this allows for things like:
1. Cave (separate region) inside region is part of a city
2. Giant tree, where each layer, branch is separate region and it could be
   one city

What region could be and could not be part of the city:
1. Each city must have one primayr region. This region contains
   some administrative building (city capital). It should be used (somehow)
   for city conquest.
2. To add region to the city, this region must be 'close'. Closeness is
   determined by time-to-reach, not by distance.
   Example: one world map, two regions is 30km apart. In ancient time, this is
   above the threshold and this region can not be attached. However, once
   teleportation and/or monorail is invented, distance is negligible and region
   can be attached.


### Multiple cities in one region


Use case:
1. Ancient city with some buildings left, fully automated, but somewhat ruined.
   Atop of those half-ruined cities, new civilization builds its own city.
   Until former habitants come back, awaken their guardians and so on, and so
   forth

2. Invasion insect-like civ, that doesn't want diplomatic relations with you.
   Instead it comes to your city, builds a hive in your region and starts
   consuming your people (by placing eggs inside them to reproduce).

3. Miniature (or spirit-like) civilization that leaves in the cities of other
   civilizations. It can build its building inside building of this city.
