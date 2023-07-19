# physicsEngine
Camera controls: 
* Mouse scroll: zoom
* Mouse middle click: camera movement

Click play for update the physics, the particles can be moved holding click in them. 
Toggle "immobile" for pinned particles (Useful for pendulums)
Toggle "spring" for creating edges with spring-like behaviour, useful for soft bodies.
Holding right click you can erase particles or edges.

If you compile this code as it is, it'll start with a system of particles which are meant to behave as a cloth, the particles have no radius and the edges have a very low thickness to achieve this effect.

You can change the forces in the constructors of the code, in the future I may be updating this for making the particles more configurable.

I'll probably add the possibility of making rigidbodies which can act as floors or walls.
