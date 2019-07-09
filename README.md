# Shinzui

## Multipurpouse server core

Shinzui as is does nothing, it implements a basic protocol, server and map, but needs a derived project (see https://github.com/gpascualg/AuraServer) to work.

Most functionalty is left as virtual on purpouse, to keep this core generic while allowing the derived project to specialize it.


### What's inside

* Graph-based clustering of hexagonal cells, which compose the map. Each cluster is updated in parallel (threaded)

* Input-only based protocol (thought mostly for rpg games)

* Basic physics through Separating Axes Theorem (SAT) + Quadtrees for local collisions

* WIP lag-compensation techniques through time-caching transforms and bounding boxes (see https://github.com/gpascualg/Shinzui/tree/time-transform)

* Async database operations with MongoDB as backend

* Build through CMake, dependency management through CMake + CMake Utils (https://github.com/gpascualg/cmake-utils/)



### Needs improvements

This project has served as a learning base for a new one (as of now private) server, アイアンハート (Ironheart), and is no longer maintained.
The main motives behind droping this one an starting from zero are:

* As cool as it is using connected-components on a graph to do threading, its complexity and cost is too high for real applications (ie. at 5000 entities the servers starts to miss its 50ms update tick).

  * A more efficient implementation is to batch up to N contiguous cells into a single cluster, independently of whereas they are connected or not. As a reference, Ironheart can easily handle 5k entities at ~1ms per tick.

* Boost::asio can do multi-threaded IO, but right now it would (probably) not work. Most of the server code is thread-safe, but the callback on async-socket operations are not. Using `strand`s might be an easy way to fix it.

* Some thread-safety relies on the use of atomic data structures (`boost:lockfree`). This might prove beneficial in coding (ie. no need to worry about mutexes, locks, deadlocks...), but contentious operations (as CAS and similiar) might make a huge impact in performance.

  * Ironheart switches to clear multi-threading and specifically plans when (and where) to lock for shared resources.

* Some opcodes must be reserved on Shinzui as opposed to the derived project (spawn, movement). Ironheart drops this generalization capability in favor of performance (less virtual, less dependencies), thus does not suffer from it.

* It is not cache-friendly. This server lacks a good memory layout, consecutive accesses are usually done to non-contiguous memory. Although object pools are widely used (in the derived project mostly, Shinzui does not allocate entities), they will be often moved and fragment accesses.

  * A new project has been started from zero to solve this problem, although PRs are welcome in this repo.


### Note

You might find a github token over the repo, it is no longer valid.
