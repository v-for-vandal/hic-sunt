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
1. Each city must have one starting region.
2. To add region to the city, this region must be 'close'. Closeness is
   determined by time-to-reach, not by distance.
   Example: one world map, two regions is 30km apart. In ancient time, this is
   above the threshold and this region can not be attached. However, once
   teleportation and/or monorail is invented, distance is negligible and region
   can be attached.

