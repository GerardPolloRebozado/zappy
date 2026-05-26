# Game lab
## Resources

The following resources are available, the resource should be evenly distributed
The amount of a specific resource can be calculated using: map_width * map_height * density

| Item | density |
| :--- | :--- |
| food | 0.5 |
| linemate | 0.3 |
| deraumere | 0.15 |
| sibur | 0.1 |
| mendiane | 0.1 |
| phiras | 0.08 |
| thystame | 0.05 |


## Map

## Leveling Up

| Level | NB players | linemate | deraumere | sibur | mendiane | phiras | thystame |
| :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| 4 | 2 | 2 | 0 | 1 | 0 | 2 | 0 |
| 5 | 4 | 1 | 1 | 2 | 0 | 1 | 0 |
| 6 | 4 | 1 | 2 | 1 | 3 | 0 | 0 |
| 7 | 6 | 1 | 2 | 3 | 0 | 1 | 0 |
| 8 | 6 | 2 | 2 | 2 | 2 | 2 | 1 |


## General rules:

* map
    * flat world to test, are we planning to add things (mountains, lakes, etc...) or not? 
    * we need to take into account the material of the world for the RL and also if some materials are going to appear more in different terrains
    * there is a limit of food and resources in the map. we are going to use the formula in the pdf.
* abilities / user rules
    * if something is blurry we need to change the vision ability of the players
    * move and look are done in the same action. we can create a new command that does these two at the same time or make one call the other. 
    * we need overlap because without it there is no eject
    * we have to discuss if the broadcasting is going to be affected by the map
    * food:
        * when we eat food, we directly add to the lifetime of the player
        * when we have more food, we could move faster or see more. 
* we want to have a config file to setup different strategies
* evolutions
    * every inhabitant in a tile evolves if they have the prerequisites to do so. somebody can eject you and stop your evolution
    * you have to drop on the tile all the ones that you need to evolve 
    * when one starts the enchantation, all the other ones that are there freeze
    * when you want to evolve, you want to be far away from people 
    * you can't see other people's inventory. you can know this by broadcasting 
    * you can only see the inventory of the people from your group 
* no limit for the inventory
* Definitions: 
    * player: physical person that connects to the server 
    * inhabitant: tiny thing that moves around
* GUI
    * gerard wants to do raylib
    * do we want unity or smth like this? probably not, unless somebody wants to spend a lot of time learning it