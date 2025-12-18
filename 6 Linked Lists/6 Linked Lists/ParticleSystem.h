#pragma once

#include "Particle.h"
#include "GUI/SimpleTest.h"
#include "GUI/MemoryDiagnostics.h"

/* Dimensions of a graphics scene. Particles that move outside the bounding
 * box [0, SCENE_WIDTH) x [0, SCENE_HEIGHT) disappear.
 */
const double SCENE_WIDTH  = 800;
const double SCENE_HEIGHT = 600;

/* Type representing a particle system: a collection of particles that can
 * be moved around the screen.
 */
class ParticleSystem {
public:
    /* Creates a new, empty particle system. */
    ParticleSystem();

    /* Cleans up all memory used by the particle system. */
    ~ParticleSystem();

    /* Adds a new particle to the scene. If the particle is out of bounds or
     * has a negative lifetime, has no effect. The particle is placed at
     * the end of the list of particles, and this function runs in time O(1).
     */
    void add(const Particle& particle);

    /* Returns how many particles are in the particle system. Runs in time
     * O(1).
     */
    int numParticles() const;

    /* Draws all the particles in the system. */
    void drawParticles() const;

    /* Moves all particles in the system. This may cause some particles
     * to be removed (if their lifetimes end or the particles move out of
     * bounds) or added (if firework particles explode).
     */
    void moveParticles();

private:
    /* Doubly-linked list type made of particles. */
    struct ParticleCell {
        Particle particle;
        ParticleCell* next;
        ParticleCell* prev;

        TRACK_ALLOCATIONS_OF(ParticleCell);
    };

    /* Pointer to the first cell in the list of particles, or nullptr if there
     * are no particles in the list.
     */
    ParticleCell* _head;

    //a particle cell that keeps track of the tail of the doubly linked list to add in O(1) time
    //Pointer to the last cell of particles, or nullptr if there are no particles in the list
    ParticleCell* _tail;

    //an integer that keeps track of the number of particles in the particle system
    int _numParticles;

    /* returns true if the particle is outside of the screen and false otherwise
     */
    bool outOfBounds(const Particle& data);

    /* returns true if the particle has a negative lifetime and false otherwise
     */
    bool endOfLife(const Particle& data);

    /* removes the cell passed in from the linked list
     */
    void removeCell(ParticleCell* cell);

    /* executes the movement of a streamer
     * 1) x coordinate increases by dx
     * 2) y coordinate increases by dy
     * 3) lifetime decreases by 1
     * removes the streamer if it is out of bounds or has negative lifetime
     */
    void streamerMove(ParticleCell* cell);

    /* executes the movement of a ballistic
     * 1) x coordinate increases by dx
     * 2) y coordinate increases by dy
     * 3) dy increases by 1
     * 3) lifetime decreases by 1
     * removes the ballistic if it is out of bounds or has negative lifetime
     */
    void ballisticMove(ParticleCell* cell);

    /* executes the movement of a firework
     * 1) x coordinate increases by dx
     * 2) y coordinate increases by dy
     * 3) dy increases by 1
     * 3) lifetime decreases by 1
     * removes the firework if it is out of bounds
     * explodes the firework into 50 streamers if the lifespan is negative and it is not
     * out of bounds
     * returns a reference to the next particle cell.
     */
    ParticleSystem::ParticleCell* fireworkMove(ParticleCell* cell);

    /* Returns a slightly cooler color (more blues and less red)
     */
    Color coolColor(Color color);

    /* executes the movement of a branch
     * 1) x coordinate increases by dx
     * 2) y coordinate increases by dy
     * 3) if life time is greater than 2 it becomes 2 otherwise lifetime decreases by 1
     * removes the branch if it is out of bounds
     * branches the branch off into two if the lifespan is negative and it is not out of bounds
     * returns a reference to the next particle cell
     */
    ParticleSystem::ParticleCell* branchMove(ParticleCell* cell);



    /* Allows SimpleTest to peek inside the ParticleSystem type. */
    ALLOW_TEST_ACCESS();
};
