#include "ParticleSystem.h"
#include "DrawParticle.h"
using namespace std;

/* The constructor initializes the essential member variables
 */
ParticleSystem::ParticleSystem() {
    //initialize the head and tail to be nullptrs
    _head = nullptr;
    _tail = nullptr;

    //the system starts with 0 particles
    _numParticles = 0;

}

/* The destructor cleans up all memory used by the particle system.
 */
ParticleSystem::~ParticleSystem() {
    //while the head is not pointing to null delete the next particle
    while (_head != nullptr) {
        //variable to store the next pointer
        ParticleCell* next = _head->next;

        //delete the particle head is pointing to
        delete _head;

        //point head to the next pointer
        _head = next;
    }
}

/* Returns how many particles are in the particle system. Runs in time
 * O(1).
 */
int ParticleSystem::numParticles() const {
    return _numParticles;
}

/* Adds a new particle to the scene. If the particle is out of bounds or
 * has a negative lifetime, has no effect. The particle is placed at
 * the end of the list of particles, and this function runs in time O(1).
 */
void ParticleSystem::add(const Particle& data) {

    //if the particle is in bounds and has a non-negative lifespan
    if (!(outOfBounds(data) || endOfLife(data))) {
        //create a temporary pointer to a new particle cell
        ParticleCell* temp = new ParticleCell;

        //initialize the data
        temp->particle = data;
        temp->next = nullptr;

        //if this is the first particle point head and tail to it
        if (_head == nullptr) {
            _head = temp;
            _tail = temp;

            //point the previous to nullptr
            temp->prev = nullptr;
        } else {
            //if it is not the first particle

            //point the previous element to the new particle cell
            _tail->next = temp;

            //point the previous for our new particle cell to where tail is pointing
            temp->prev = _tail;

            //point tail to the new particle cell
            _tail = temp;
        }

        _numParticles++;
    }
}

void ParticleSystem::drawParticles() const {
    //create a new Particle cell pointer and set it equal to where head is pointing
    ParticleCell* draw = _head;

    //iterate until draw points to a null pointer to loop through particles
    while (draw != nullptr) {
        drawParticle(draw->particle.x, draw->particle.y, draw->particle.color);

        //point draw at the next particle in the system
        draw = draw->next;
    }
}

void ParticleSystem::moveParticles() {

    //create a new Particle cell pointer and set it equal to where head is pointing
    ParticleCell* move = _head;

    //iterate until draw points to a null pointer to loop through particles
    while (move != nullptr) {
        //store the next movement
        ParticleCell* next = move->next;

        //extension branch
        if (move->particle.type == ParticleType::BRANCH) {
            next = branchMove(move);
        } else if (move->particle.type == ParticleType::FIREWORK) {
            next = fireworkMove(move);
        } else if (move->particle.type == ParticleType::BALLISTIC) {
            ballisticMove(move);
        } else {
            streamerMove(move);
        }

        //point move at the next particle in the system
        move = next;
    }
}

bool ParticleSystem::outOfBounds(const Particle& data) {
    return (data.x < 0 || data.x >= SCENE_WIDTH || data.y < 0 || data.y >= SCENE_HEIGHT);
}

bool ParticleSystem::endOfLife(const Particle& data) {
    return data.lifetime < 0;
}

void ParticleSystem::removeCell(ParticleCell* cell) {
    //conditions for all four cases (removing the head and tail, removing head, tail, and from the middle)
    if (_head == cell && _tail == cell) {
        //set the head and tail to null
        _head = nullptr;
        _tail = nullptr;

        delete cell;

    } else if (_head == cell) {
        //if we are removing the first cell

        //point head to the next cell and change the previous of that cell to null
        _head = cell->next;
        _head->prev = nullptr;

        delete cell;

    } else if (_tail == cell) {
        //if we are removing the last cell

        //point tail to the previous cell and change the next of that cell to null
        _tail = cell->prev;
        _tail->next = nullptr;

        delete cell;

    } else {
        //if we are removing in the middle

        //set the previous cell's next to point to the next cell
        //and set the next cell's previous to point to the previous cell
        cell->prev->next = cell->next;
        cell->next->prev = cell->prev;

        delete cell;
    }
    _numParticles--;
}

void ParticleSystem::streamerMove(ParticleCell* cell) {
    //create a reference to the particle to move it
    Particle &cellParticle = cell->particle;

    cellParticle.x += cellParticle.dx;
    cellParticle.y += cellParticle.dy;
    cellParticle.lifetime--;

    //check if it is invalid now
    if (outOfBounds(cellParticle) || endOfLife(cellParticle)) {
        removeCell(cell);
    }
}

void ParticleSystem::ballisticMove(ParticleCell* cell) {
    //create a reference to the particle to move it
    Particle &cellParticle = cell->particle;

    cellParticle.x += cellParticle.dx;
    cellParticle.y += cellParticle.dy;
    cellParticle.lifetime--;
    cellParticle.dy++;

    //check if it is invalid now
    if (outOfBounds(cellParticle) || endOfLife(cellParticle)) {
        removeCell(cell);
    }
}

ParticleSystem::ParticleCell* ParticleSystem::fireworkMove(ParticleCell* cell) {
    //create a new particle cell pointer to return
    ParticleCell* next = nullptr;

    //create a reference to the particle to move it
    Particle &cellParticle = cell->particle;

    cellParticle.x += cellParticle.dx;
    cellParticle.y += cellParticle.dy;
    cellParticle.lifetime--;
    cellParticle.dy++;

    //remove the particle if it is out of bounds
    if (outOfBounds(cellParticle)) {
        //store the next cell before deleting the cell
        next = cell->next;

        //delete the cell
        removeCell(cell);

        return next;
    } else if (endOfLife(cellParticle)) {
        //if it is not out of bounds but it runs out of lifespan, explode the firework

        //find a random color for all the debris
        Color debrisColor = Color::RANDOM();

        //create 50 unique streamers by looping 50 times
        for (int i = 0; i < 50; i++) {
            Particle debris;
            debris.x = cellParticle.x;
            debris.y = cellParticle.y;
            debris.color = debrisColor;
            debris.type = ParticleType::STREAMER;

            debris.dx = randomReal(-3, 3);
            debris.dy = randomReal(-3, 3);
            debris.lifetime = randomInteger(2, 10);

            add(debris);
        }

        //store the next cell before deleting the cell
        next = cell->next;

        //delete the cell
        removeCell(cell);

        return next;
    }

    return cell->next;
}

//extension particle
ParticleSystem::ParticleCell* ParticleSystem::branchMove(ParticleCell* cell) {
    //create a new particle cell pointer to return
    ParticleCell* next = nullptr;

    //create a reference to the particle to move it
    Particle &cellParticle = cell->particle;

    cellParticle.x += cellParticle.dx;
    cellParticle.y += cellParticle.dy;
    //make sure it branches within 3 moves
    if (cellParticle.lifetime > 1) {
        cellParticle.lifetime = 1;
    } else {
        cellParticle.lifetime--;
    }

    //remove the particle if it is out of bounds
    if (outOfBounds(cellParticle)) {
        //store the next cell before deleting the cell
        next = cell->next;

        //delete the cell
        removeCell(cell);

        return next;
    } else if (endOfLife(cellParticle)) {
        //if it is not out of bounds but it runs out of lifespan

        //find a random color for the branches and a common lifetime
        Color branchColor = coolColor(cellParticle.color);
        int life = randomInteger(1, 2);

        //give it a 60 percent chance of branching and cool the color of the two branches
        if (randomInteger(1, 10) < 7) {
            //store the x, y, dx, dy of cell particle in doubles
            double x = cellParticle.x;
            double y = cellParticle.y;
            double dx = cellParticle.dx;
            double dy = cellParticle.dy;

            //branch one
            Particle branchOne;
            branchOne.x = x;
            branchOne.y = y;

            //using rotations make branch one move 30 degrees counterclockwise
            branchOne.dx = (dx * cos(30 * PI / 180)) - (dy * sin(30 * PI / 180));
            branchOne.dy = (dx * sin(30 * PI / 180)) + (dy * cos(30 * PI / 180));

            branchOne.color = branchColor;
            branchOne.lifetime = life;

            //add the branch
            add(branchOne);

            //branch two
            Particle branchTwo;
            branchTwo.x = x;
            branchTwo.y = y;

            //using rotations make branch one move 30 degrees clockwise
            branchTwo.dx = (dx * cos(30 * PI / 180)) + (dy * sin(30 * PI / 180));
            branchTwo.dy = - (dx * sin(30 * PI / 180)) + (dy * cos(30 * PI / 180));

            branchTwo.color = branchColor;
            branchTwo.lifetime = life;

            //add the branch
            add(branchTwo);
        }

        //store the next cell before deleting the cell
        next = cell->next;

        //delete the cell
        removeCell(cell);

        return next;
    }

    return cell->next;
}

Color ParticleSystem::coolColor(Color color) {
    double scale = 0.05;

    //looked through the color.h file to figure this out
    int r = color.red();
    int g = color.green();
    int b = color.blue();

    //cool the color
    r = max(0, int(r * (1 - scale)));
    b = min(255, int(b + (255 - b) * scale));

    return Color(r, g, b);
}

/* * * * * Test Cases Below This Point * * * * */

//milestone 1 test cases

STUDENT_TEST("Milestone 1: testing constructor and one particle") {
    ParticleSystem system;

    //testing the constructor
    EXPECT_EQUAL(system._head, nullptr);
    EXPECT_EQUAL(system._tail, nullptr);
    EXPECT_EQUAL(system._numParticles, 0);

    //adding a particle
    Particle particle;
    particle.x = 10;
    particle.y = 20;
    particle.color = Color::BLACK();

    system.add(particle);

    EXPECT_EQUAL(system.numParticles(), 1);
    EXPECT_NOT_EQUAL(system._head, nullptr);
    EXPECT_NOT_EQUAL(system._tail, nullptr);
}

STUDENT_TEST("Milestone 1: adding multiple particles") {
    ParticleSystem system;

    //add 10 particles to the system
    const int kNumParticles = 10;
    for (int i = 0; i < kNumParticles; i++) {
        Particle particle;
        particle.x = i * 2;
        system.add(particle);
    }

    //check that the system has 10 particles
    EXPECT_EQUAL(system.numParticles(), kNumParticles);

    //Confirm the particles are there and in the right order.
    int numSeen = 0;
    ParticleSystem::ParticleCell* prev = nullptr;
    ParticleSystem::ParticleCell* curr = system._head;

    //check each element
    while (curr != nullptr) {
        //use the x coordinate to track our particles
        EXPECT_EQUAL(curr->particle.x, numSeen * 2);

        //confirm the list is wired correctly
        EXPECT_EQUAL(curr->prev, prev);

        //move along the particles
        prev = curr;
        curr = curr->next;
        numSeen++;
    }

    //confirm there are the right number of particles
    EXPECT_EQUAL(numSeen, kNumParticles);
    EXPECT_EQUAL(system.numParticles(), kNumParticles);
}

//milestone 5 test cases

STUDENT_TEST("Milestone 5: Fireworks doesn't explode if it goes out of bounds") {
    ParticleSystem system;

    /* Create and add a firework. */
    Particle firework;
    firework.type = ParticleType::FIREWORK;
    firework.lifetime = 0; // Explodes as soon as it moves
    firework.x = 100;
    firework.y = 100;
    firework.dx = -101;
    system.add(firework);

    EXPECT_EQUAL(system.numParticles(), 1);

    //moving the particle should make it go out of bounds and not trigger an explosion
    //even though the lifetime is negative
    system.moveParticles();

    //there should be no particles in the system
    EXPECT_EQUAL(system.numParticles(), 0);
    EXPECT_EQUAL(system._head, nullptr);
}

STUDENT_TEST("Milestone 5: The new streamer particles also move by one step when the firework explodes") {
    ParticleSystem system;

    //create and add a new firework
    Particle firework;
    firework.type = ParticleType::FIREWORK;
    firework.lifetime = 0; // Explodes as soon as it moves
    firework.x = 100;
    firework.y = 100;
    system.add(firework);

    EXPECT_EQUAL(system.numParticles(), 1);

    //Move the particles. This should trigger the explosion.
    system.moveParticles();

    //there should now be 50 streamers
    EXPECT_EQUAL(system.numParticles(), 50);

    //loop through each particle and check that they are not in the same position as the firework
    //was if dx and dy weren't 0

    ParticleSystem::ParticleCell* cur = system._head;
    while (cur != nullptr) {
        if (cur->particle.dx != 0) {
            EXPECT_NOT_EQUAL(cur->particle.x, 100);
        }

        if (cur->particle.dy != 0) {
            EXPECT_NOT_EQUAL(cur->particle.y, 100);
        }

        cur = cur->next;
    }
}

STUDENT_TEST("Milestone 5: The new streamer particles also move by one step when the firework explodes when there are things that come after the firework") {
    ParticleSystem system;

    //create and add a new firework
    Particle firework;
    firework.type = ParticleType::FIREWORK;
    firework.lifetime = 0; // Explodes as soon as it moves
    firework.x = 100;
    firework.y = 100;
    system.add(firework);

    Particle particle;
    particle.x = 13.7;
    particle.y = 4.2;
    particle.color = Color::CYAN();
    system.add(particle);

    EXPECT_EQUAL(system.numParticles(), 2);

    //Move the particles. This should trigger the explosion.
    system.moveParticles();

    //there should now be 50 streamers
    EXPECT_EQUAL(system.numParticles(), 51);

    //loop through each particle and check that they are not in the same position as the firework
    //was if dx and dy weren't 0

    ParticleSystem::ParticleCell* cur = system._head->next;
    while (cur != nullptr) {
        if (cur->particle.dx != 0) {
            EXPECT_NOT_EQUAL(cur->particle.x, 100);
        }

        if (cur->particle.dy != 0) {
            EXPECT_NOT_EQUAL(cur->particle.y, 100);
        }

        cur = cur->next;
    }
}

//extension: test for my branches
STUDENT_TEST("does my branches, branch after three movements no matter how large the lifespan") {
    ParticleSystem system;

    //create and add a new branch
    Particle branch;
    branch.type = ParticleType::BRANCH;
    branch.lifetime = 10;
    branch.x = SCENE_WIDTH / 2;
    branch.y = SCENE_HEIGHT - 1;
    branch.dy = -2;
    system.add(branch);

    system.moveParticles();
    EXPECT_EQUAL(system.numParticles(), 1);

    system.moveParticles();
    EXPECT_EQUAL(system.numParticles(), 1);

    system.moveParticles();

    if (!(system.numParticles() == 0 || system.numParticles() == 2)) {
        error("branch didn't branch in 3 moves");
    }
}


/* * * * * Provided Tests Below This Point * * * * */

PROVIDED_TEST("Milestone 1: Constructor creates an empty particle system.") {
    ParticleSystem system;
    EXPECT_EQUAL(system._head, nullptr);
}

PROVIDED_TEST("Milestone 1: Empty system has no particles.") {
    ParticleSystem system;
    EXPECT_EQUAL(system.numParticles(), 0);
}

PROVIDED_TEST("Milestone 1: Can add a single particle.") {
    ParticleSystem system;

    Particle particle;
    particle.x = 13.7;
    particle.y = 4.2;
    particle.color = Color::CYAN();

    system.add(particle);

    /* Should have one particle. */
    EXPECT_EQUAL(system.numParticles(), 1);

    /* Invasively check to make sure the head pointer isn't null,
     * since it needs to hold our particle.
     */
    EXPECT_NOT_EQUAL(system._head, nullptr);

    /* Make sure the particle's x, y, and color are copied over. */
    EXPECT_EQUAL(system._head->particle.x, particle.x);
    EXPECT_EQUAL(system._head->particle.y, particle.y);
    EXPECT_EQUAL(system._head->particle.color, particle.color);

    /* Make sure the list is wired correctly. */
    EXPECT_EQUAL(system._head->prev, nullptr);
    EXPECT_EQUAL(system._head->next, nullptr);
}

PROVIDED_TEST("Milestone 1: Can add two particles.") {
    ParticleSystem system;

    /* Make two particles. Use the x and y coordinates to tell them apart. */
    Particle one, two;
    one.x = 1;
    two.x = 2;

    /* Add each to the particle system. The order should be one, then two. */
    system.add(one);
    system.add(two);

    /* Make sure we see two particles. */
    EXPECT_EQUAL(system.numParticles(), 2);

    /* Make sure the list has two items in it. */
    EXPECT_NOT_EQUAL(system._head, nullptr);
    EXPECT_NOT_EQUAL(system._head->next, nullptr);
    EXPECT_EQUAL(system._head->next->next, nullptr);

    /* Make sure previous pointers work correctly. */
    EXPECT_EQUAL(system._head->prev, nullptr);
    EXPECT_EQUAL(system._head->next->prev, system._head);

    /* Make sure the particles are in the right order. */
    EXPECT_EQUAL(system._head->particle.x, 1);
    EXPECT_EQUAL(system._head->next->particle.x, 2);
}

PROVIDED_TEST("Milestone 1: Can add multiple particles.") {
    ParticleSystem system;

    /* Add some particles. */
    const int kNumParticles = 5;
    for (int i = 0; i < kNumParticles; i++) {
        Particle particle;
        particle.x = i;
        system.add(particle);
    }

    EXPECT_EQUAL(system.numParticles(), kNumParticles);

    /* Confirm they're there and in the right order. */
    int numSeen = 0;
    ParticleSystem::ParticleCell* prev = nullptr;
    ParticleSystem::ParticleCell* curr = system._head;

    /* Walk the list checking particles. */
    while (curr != nullptr) {
        /* x coordinate tracks which particle this is, so this is a way of
         * checking whether we've got the particles in the right order.
         */
        EXPECT_EQUAL(curr->particle.x, numSeen);

        /* Check the list wiring - we should point back to the cell before us,
         * or to nullptr if there are no cells here.
         */
        EXPECT_EQUAL(curr->prev, prev);

        /* Advance forward. */
        prev = curr;
        curr = curr->next;
        numSeen++;
    }

    /* Confirm we saw the right number. */
    EXPECT_EQUAL(numSeen, kNumParticles);
    EXPECT_EQUAL(system.numParticles(), kNumParticles);
}

PROVIDED_TEST("Milestone 1: Stress Test: Efficiently adds and counts particles.") {
    ParticleSystem system;

    /* Add a bunch of particles. If the add operation takes time O(n), this
     * will run very, very slowly. If the add operation takes time O(1),
     * this will run very quickly.
     */
    const int kNumParticles = 100000;
    EXPECT_COMPLETES_IN(5.0, {
        for (int i = 0; i < kNumParticles; i++) {
            Particle particle;
            particle.x = i / (kNumParticles * 1.0);
            system.add(particle);
        }
    });

    /* Count how many particles there are. If this takes time O(n), this
     * will run very, very slowly. If the numParticles operation takes
     * time O(1), this will run almost instantaneously.
     */
    EXPECT_COMPLETES_IN(1.0, {
        const int kNumTimes = 100000;
        for (int i = 0; i < kNumTimes; i++) {
            EXPECT_EQUAL(system.numParticles(), kNumParticles);
        }
    });
}

#include "Demos/ParticleCatcher.h"

PROVIDED_TEST("Milestone 2: Single particle gets drawn.") {
    ParticleSystem system;

    /* This ParticleCatcher will write down all the calls to the global
     * drawParticle function so that we can see what's drawn.
     */
    ParticleCatcher catcher;

    /* Draw all the particles. This should do nothing. */
    system.drawParticles();

    /* Make sure nothing was drawn. */
    EXPECT_EQUAL(catcher.numDrawn(), 0);

    /* Now add a particle. */
    Particle particle;
    particle.x = 13.7;
    particle.y = 4.2;
    particle.color = Color::RANDOM();
    system.add(particle);

    /* Draw everything again. We should see this particle. */
    system.drawParticles();
    EXPECT_EQUAL(catcher.numDrawn(), 1);
    EXPECT_EQUAL(catcher[0].x,     particle.x);
    EXPECT_EQUAL(catcher[0].y,     particle.y);
    EXPECT_EQUAL(catcher[0].color, particle.color);
}

PROVIDED_TEST("Milestone 2: Multiple particles are drawn in order.") {
    ParticleSystem system;

    /* Add three particles in a specific order. */
    Particle one, two, three;
    one.color   = Color::CYAN();
    two.color   = Color::YELLOW();
    three.color = Color::MAGENTA();

    system.add(one);
    system.add(two);
    system.add(three);

    /* Draw the particles and confirm they come back in the right order. */
    ParticleCatcher catcher;
    system.drawParticles();

    EXPECT_EQUAL(catcher.numDrawn(), 3);
    EXPECT_EQUAL(catcher[0].color, Color::CYAN());
    EXPECT_EQUAL(catcher[1].color, Color::YELLOW());
    EXPECT_EQUAL(catcher[2].color, Color::MAGENTA());
}

PROVIDED_TEST("Milestone 3: Can move a single particle.") {
    ParticleSystem system;

    /* The particle we'll move around. */
    Particle particle;
    particle.x = 100;
    particle.y = 100;
    particle.dx = 3;
    particle.dy = -4;
    system.add(particle);
    EXPECT_EQUAL(system.numParticles(), 1);

    /* Confirm we have all the right information stored. */
    EXPECT_NOT_EQUAL(system._head, nullptr);
    EXPECT_EQUAL(system._head->particle.x,     particle.x);
    EXPECT_EQUAL(system._head->particle.y,     particle.y);
    EXPECT_EQUAL(system._head->particle.color, particle.color);
    EXPECT_EQUAL(system._head->particle.dx,    particle.dx);
    EXPECT_EQUAL(system._head->particle.dy,    particle.dy);

    /* Move the particle. */
    system.moveParticles();

    /* The particle should be in a new spot with the same initial velocity. */
    EXPECT_EQUAL(system._head->particle.x,     particle.x + particle.dx);
    EXPECT_EQUAL(system._head->particle.y,     particle.y + particle.dy);
    EXPECT_EQUAL(system._head->particle.color, particle.color);
    EXPECT_EQUAL(system._head->particle.dx,    particle.dx);
    EXPECT_EQUAL(system._head->particle.dy,    particle.dy);

    /* Move the particle again. */
    system.moveParticles();

    /* The particle should be in a new spot with the same initial velocity. */
    EXPECT_EQUAL(system._head->particle.x,     particle.x + 2 * particle.dx);
    EXPECT_EQUAL(system._head->particle.y,     particle.y + 2 * particle.dy);
    EXPECT_EQUAL(system._head->particle.color, particle.color);
    EXPECT_EQUAL(system._head->particle.dx,    particle.dx);
    EXPECT_EQUAL(system._head->particle.dy,    particle.dy);
}

PROVIDED_TEST("Milestone 3: Can move multiple particles with different velocities.") {
    ParticleSystem system;

    /* Set up the particles. */
    Particle left, down, diag;
    left.dx = 1;
    left.dy = 0;

    down.dx = 0;
    down.dy = 1;

    diag.dx = 1;
    diag.dy = 1;

    system.add(left);
    system.add(down);
    system.add(diag);

    /* Move the particles multiple times and confirm they're in the right
     * places.
     */
    const int kNumSteps = 50;
    for (int i = 0; i < kNumSteps; i++) {
        /* Draw the particles to see where they are. */
        ParticleCatcher catcher;
        system.drawParticles();

        /* Make sure we see all three particles and that they're in the right
         * places.
         */
        EXPECT_EQUAL(catcher.numDrawn(), 3);

        /* Left */
        EXPECT_EQUAL(catcher[0].x, i);
        EXPECT_EQUAL(catcher[0].y, 0);

        /* Down */
        EXPECT_EQUAL(catcher[1].x, 0);
        EXPECT_EQUAL(catcher[1].y, i);

        /* Diag */
        EXPECT_EQUAL(catcher[2].x, i);
        EXPECT_EQUAL(catcher[2].y, i);

        /* Move the particles for the next iteration. */
        system.moveParticles();
    }
}

PROVIDED_TEST("Milestone 4: Don't add invalid particles to the list.") {
    ParticleSystem system;

    /* Add an unobjectionable particle. */
    Particle good;
    good.x = 3;
    good.y = 5;
    good.color = Color::BLUE();
    good.lifetime = 137;

    /* Add the particle; confirm it's there. */
    system.add(good);
    EXPECT_NOT_EQUAL(system._head, nullptr);
    EXPECT_EQUAL(system._head->particle.x,        good.x);
    EXPECT_EQUAL(system._head->particle.y,        good.y);
    EXPECT_EQUAL(system._head->particle.color,    good.color);
    EXPECT_EQUAL(system._head->particle.lifetime, good.lifetime);
    EXPECT_EQUAL(system.numParticles(), 1);

    /* Now make a mix of bad particles that are out of bounds. */
    Particle bad;

    bad = good;
    bad.x = -0.001; // x too low
    system.add(bad);
    EXPECT_EQUAL(system.numParticles(), 1);

    bad = good;
    bad.y = -0.001; // y too low
    system.add(bad);
    EXPECT_EQUAL(system.numParticles(), 1);

    bad = good;
    bad.x = SCENE_WIDTH; // x too high
    system.add(bad);
    EXPECT_EQUAL(system.numParticles(), 1);

    bad = good;
    bad.y = SCENE_HEIGHT; // y too high
    system.add(bad);
    EXPECT_EQUAL(system.numParticles(), 1);

    bad = good;
    bad.lifetime = -1; // Negative lifetime
    system.add(bad);
    EXPECT_EQUAL(system.numParticles(), 1);

    /* Make sure the first particle is still there and unchanged. */
    EXPECT_EQUAL(system._head->particle.x,        good.x);
    EXPECT_EQUAL(system._head->particle.y,        good.y);
    EXPECT_EQUAL(system._head->particle.color,    good.color);
    EXPECT_EQUAL(system._head->particle.lifetime, good.lifetime);

    /* Make sure the list is still valid. */
    EXPECT_EQUAL(system._head->next, nullptr);
    EXPECT_EQUAL(system._head->prev, nullptr);
}

PROVIDED_TEST("Milestone 4: Particle removed when it leaves the screen.") {
    ParticleSystem system;

    /* A particle moving out of the world. */
    Particle escapee;
    escapee.x = 1.5;
    escapee.y = 1.5;
    escapee.dx = -1;
    escapee.dy = -1;
    escapee.lifetime = 1000;
    system.add(escapee);

    /* Use a ParticleCatcher to see which particles are drawn. */
    ParticleCatcher catcher;

    /* Initially, we should find our particle where we created it. */
    system.drawParticles();

    EXPECT_EQUAL(catcher.numDrawn(), 1);
    EXPECT_EQUAL(catcher[0].x, 1.5);
    EXPECT_EQUAL(catcher[0].y, 1.5);
    catcher.reset(); // Forget we saw the particle

    /* Moving the particle one step should leave the particle alive. */
    system.moveParticles();
    system.drawParticles();

    EXPECT_EQUAL(catcher.numDrawn(), 1);
    EXPECT_EQUAL(catcher[0].x, 0.5);
    EXPECT_EQUAL(catcher[0].y, 0.5);
    catcher.reset(); // Forget we saw the particle

    /* Moving the particle once more should remove the particle because
     * it'll be at position (-0.5, -0.5).
     */
    system.moveParticles();
    system.drawParticles();

    EXPECT_EQUAL(catcher.numDrawn(), 0);
    EXPECT_EQUAL(system.numParticles(), 0);
}

PROVIDED_TEST("Milestone 4: Particle removed when its lifetime ends.") {
    ParticleSystem system;

    /* A particle moving out of the world. */
    Particle timeout;
    timeout.x = 1.5;
    timeout.y = 1.5;
    timeout.dx = 1;
    timeout.dy = 1;
    timeout.lifetime = 1;
    system.add(timeout);

    /* Use a ParticleCatcher to see which particles are drawn. */
    ParticleCatcher catcher;

    /* Initially, we should find our particle where we created it. */
    system.drawParticles();

    EXPECT_EQUAL(catcher.numDrawn(), 1);
    EXPECT_EQUAL(catcher[0].x, 1.5);
    EXPECT_EQUAL(catcher[0].y, 1.5);
    catcher.reset(); // Forget we saw the particle

    /* Moving the particle one step should leave the particle alive but with
     * a lifetime of 0.
     */
    system.moveParticles();
    system.drawParticles();

    EXPECT_EQUAL(catcher.numDrawn(), 1);
    EXPECT_EQUAL(catcher[0].x, 2.5);
    EXPECT_EQUAL(catcher[0].y, 2.5);
    catcher.reset(); // Forget we saw the particle

    /* Moving the particle once more should remove the particle because its
     * lifetime becomes negative.
     */
    system.moveParticles();
    system.drawParticles();

    EXPECT_EQUAL(catcher.numDrawn(), 0);
    EXPECT_EQUAL(system.numParticles(), 0);
}

PROVIDED_TEST("Milestone 4: All particles move even if first needs to be removed.") {
    /* Create a list of five particles at positions (0, 0), (1, 0), ... (4, 0).
     * All particles move downward. All particles have a long lifetime except
     * for the first, which has a lifetime of 0 and thus disappears as soon as
     * it moves. We should see all the others shifted down by one spot.
     */
    ParticleSystem system;

    /* Used to identify particles. */
    const Vector<Color> colors = {
        Color::BLACK(), Color::WHITE(), Color::RED(), Color::GREEN(), Color::YELLOW()
    };

    /* Initialize the particles. */
    Vector<Particle> particles;
    for (int i = 0; i < 5; i++) {
        Particle particle;
        particle.x = i;
        particle.dx = 0;
        particle.dy = 1;
        particle.lifetime = 100;
        particle.color = colors[i]; // So we can flag it later
        particles += particle;
    }
    particles[0].lifetime = 0;

    /* Add the particles. */
    for (Particle particle: particles) {
        system.add(particle);
    }
    EXPECT_EQUAL(system.numParticles(), 5);

    /* Move them all. */
    system.moveParticles();
    EXPECT_EQUAL(system.numParticles(), 4); // One was removed

    /* See where they are. */
    ParticleCatcher catcher;
    system.drawParticles();

    /* Confirm we have particles 1, 2, 3, and 4, in that order. */
    EXPECT_EQUAL(catcher.numDrawn(), 4);
    EXPECT_EQUAL(catcher[0], { 1, 1, colors[1] });
    EXPECT_EQUAL(catcher[1], { 2, 1, colors[2] });
    EXPECT_EQUAL(catcher[2], { 3, 1, colors[3] });
    EXPECT_EQUAL(catcher[3], { 4, 1, colors[4] });

    /* Check the linked list to make sure the wiring is right. */
    ParticleSystem::ParticleCell* curr = system._head;
    ParticleSystem::ParticleCell* prev = nullptr;
    for (int i = 0; i < 4; i++) {
        EXPECT_NOT_EQUAL(curr, nullptr);
        EXPECT_EQUAL(curr->prev, prev);

        prev = curr;
        curr = curr->next;
    }
    EXPECT_EQUAL(curr, nullptr);
}

PROVIDED_TEST("Milestone 4: All particles move even if second needs to be removed.") {
    /* Create a list of five particles at positions (0, 0), (1, 0), ... (4, 0).
     * All particles move downward. All particles have a long lifetime except
     * for the second, which has a lifetime of 0 and thus disappears as soon as
     * it moves. We should see all the others shifted down by one spot.
     */
    ParticleSystem system;

    /* Used to identify particles. */
    const Vector<Color> colors = {
        Color::BLACK(), Color::WHITE(), Color::RED(), Color::GREEN(), Color::YELLOW()
    };

    /* Initialize the particles. */
    Vector<Particle> particles;
    for (int i = 0; i < 5; i++) {
        Particle particle;
        particle.x = i;
        particle.dx = 0;
        particle.dy = 1;
        particle.lifetime = 100;
        particle.color = colors[i]; // So we can flag it later
        particles += particle;
    }
    particles[1].lifetime = 0;

    /* Add the particles. */
    for (Particle particle: particles) {
        system.add(particle);
    }
    EXPECT_EQUAL(system.numParticles(), 5);

    /* Move them all. */
    system.moveParticles();
    EXPECT_EQUAL(system.numParticles(), 4); // One was removed

    /* See where they are. */
    ParticleCatcher catcher;
    system.drawParticles();

    /* Confirm we have particles 0, 2, 3, and 4, in that order. */
    EXPECT_EQUAL(catcher.numDrawn(), 4);
    EXPECT_EQUAL(catcher[0], { 0, 1, colors[0] });
    EXPECT_EQUAL(catcher[1], { 2, 1, colors[2] });
    EXPECT_EQUAL(catcher[2], { 3, 1, colors[3] });
    EXPECT_EQUAL(catcher[3], { 4, 1, colors[4] });

    /* Check the linked list to make sure the wiring is right. */
    ParticleSystem::ParticleCell* curr = system._head;
    ParticleSystem::ParticleCell* prev = nullptr;
    for (int i = 0; i < 4; i++) {
        EXPECT_NOT_EQUAL(curr, nullptr);
        EXPECT_EQUAL(curr->prev, prev);

        prev = curr;
        curr = curr->next;
    }
    EXPECT_EQUAL(curr, nullptr);
}

PROVIDED_TEST("Milestone 4: All particles move even if last needs to be removed.") {
    /* Create a list of five particles at positions (0, 0), (1, 0), ... (4, 0).
     * All particles move downward. All particles have a long lifetime except
     * for the last, which has a lifetime of 0 and thus disappears as soon as
     * it moves. We should see all the others shifted down by one spot.
     */
    ParticleSystem system;

    /* Used to identify particles. */
    const Vector<Color> colors = {
        Color::BLACK(), Color::WHITE(), Color::RED(), Color::GREEN(), Color::YELLOW()
    };

    /* Initialize the particles. */
    Vector<Particle> particles;
    for (int i = 0; i < 5; i++) {
        Particle particle;
        particle.x = i;
        particle.dx = 0;
        particle.dy = 1;
        particle.lifetime = 100;
        particle.color = colors[i]; // So we can flag it later
        particles += particle;
    }
    particles[4].lifetime = 0;

    /* Add the particles. */
    for (Particle particle: particles) {
        system.add(particle);
    }
    EXPECT_EQUAL(system.numParticles(), 5);

    /* Move them all. */
    system.moveParticles();
    EXPECT_EQUAL(system.numParticles(), 4); // One was removed

    /* See where they are. */
    ParticleCatcher catcher;
    system.drawParticles();

    /* Confirm we have particles 0, 1, 2, and 3, in that order. */
    EXPECT_EQUAL(catcher.numDrawn(), 4);
    EXPECT_EQUAL(catcher[0], { 0, 1, colors[0] });
    EXPECT_EQUAL(catcher[1], { 1, 1, colors[1] });
    EXPECT_EQUAL(catcher[2], { 2, 1, colors[2] });
    EXPECT_EQUAL(catcher[3], { 3, 1, colors[3] });

    /* Check the linked list to make sure the wiring is right. */
    ParticleSystem::ParticleCell* curr = system._head;
    ParticleSystem::ParticleCell* prev = nullptr;
    for (int i = 0; i < 4; i++) {
        EXPECT_NOT_EQUAL(curr, nullptr);
        EXPECT_EQUAL(curr->prev, prev);

        prev = curr;
        curr = curr->next;
    }
    EXPECT_EQUAL(curr, nullptr);
}

PROVIDED_TEST("Milestone 4: All particles move even if second-to-last needs to be removed.") {
    /* Create a list of five particles at positions (0, 0), (1, 0), ... (4, 0).
     * All particles move downward. All particles have a long lifetime except
     * for the penultimate, which has a lifetime of 0 and thus disappears as soon as
     * it moves. We should see all the others shifted down by one spot.
     */
    ParticleSystem system;

    /* Used to identify particles. */
    const Vector<Color> colors = {
        Color::BLACK(), Color::WHITE(), Color::RED(), Color::GREEN(), Color::YELLOW()
    };

    /* Initialize the particles. */
    Vector<Particle> particles;
    for (int i = 0; i < 5; i++) {
        Particle particle;
        particle.x = i;
        particle.dx = 0;
        particle.dy = 1;
        particle.lifetime = 100;
        particle.color = colors[i]; // So we can flag it later
        particles += particle;
    }
    particles[3].lifetime = 0;

    /* Add the particles. */
    for (Particle particle: particles) {
        system.add(particle);
    }
    EXPECT_EQUAL(system.numParticles(), 5);

    /* Move them all. */
    system.moveParticles();
    EXPECT_EQUAL(system.numParticles(), 4); // One was removed

    /* See where they are. */
    ParticleCatcher catcher;
    system.drawParticles();

    /* Confirm we have particles 0, 1, 2, and 4, in that order. */
    EXPECT_EQUAL(catcher.numDrawn(), 4);
    EXPECT_EQUAL(catcher[0], { 0, 1, colors[0] });
    EXPECT_EQUAL(catcher[1], { 1, 1, colors[1] });
    EXPECT_EQUAL(catcher[2], { 2, 1, colors[2] });
    EXPECT_EQUAL(catcher[3], { 4, 1, colors[4] });

    /* Check the linked list to make sure the wiring is right. */
    ParticleSystem::ParticleCell* curr = system._head;
    ParticleSystem::ParticleCell* prev = nullptr;
    for (int i = 0; i < 4; i++) {
        EXPECT_NOT_EQUAL(curr, nullptr);
        EXPECT_EQUAL(curr->prev, prev);

        prev = curr;
        curr = curr->next;
    }
    EXPECT_EQUAL(curr, nullptr);
}

PROVIDED_TEST("Milestone 4: All particles move even if first needs to be removed.") {
    /* Create a list of five particles at positions (0, 0), (1, 0), ... (4, 0).
     * All particles move downward. All particles have a long lifetime except
     * for the first, which has a lifetime of 0 and thus disappears as soon as
     * it moves. We should see all the others shifted down by one spot.
     */
    ParticleSystem system;

    /* Used to identify particles. */
    const Vector<Color> colors = {
        Color::BLACK(), Color::WHITE(), Color::RED(), Color::GREEN(), Color::YELLOW()
    };

    /* Initialize the particles. */
    Vector<Particle> particles;
    for (int i = 0; i < 5; i++) {
        Particle particle;
        particle.x = i;
        particle.dx = 0;
        particle.dy = 1;
        particle.lifetime = 100;
        particle.color = colors[i]; // So we can flag it later
        particles += particle;
    }
    particles[2].lifetime = 0;

    /* Add the particles. */
    for (Particle particle: particles) {
        system.add(particle);
    }
    EXPECT_EQUAL(system.numParticles(), 5);

    /* Move them all. */
    system.moveParticles();
    EXPECT_EQUAL(system.numParticles(), 4); // One was removed

    /* See where they are. */
    ParticleCatcher catcher;
    system.drawParticles();

    /* Confirm we have particles 0, 1, 3, and 4, in that order. */
    EXPECT_EQUAL(catcher.numDrawn(), 4);
    EXPECT_EQUAL(catcher[0], { 0, 1, colors[0] });
    EXPECT_EQUAL(catcher[1], { 1, 1, colors[1] });
    EXPECT_EQUAL(catcher[2], { 3, 1, colors[3] });
    EXPECT_EQUAL(catcher[3], { 4, 1, colors[4] });

    /* Check the linked list to make sure the wiring is right. */
    ParticleSystem::ParticleCell* curr = system._head;
    ParticleSystem::ParticleCell* prev = nullptr;
    for (int i = 0; i < 4; i++) {
        EXPECT_NOT_EQUAL(curr, nullptr);
        EXPECT_EQUAL(curr->prev, prev);

        prev = curr;
        curr = curr->next;
    }
    EXPECT_EQUAL(curr, nullptr);
}

PROVIDED_TEST("Milestone 4: All particles move even if many need to be removed.") {
    /* Create a list of five particles at positions (0, 0), (1, 0), ... (4, 0).
     * All particles move downward. Particles 0, 2, and 4 have lifetime 0 and
     * thus will disappear after the first step. The other two will live for
     * multiple time steps.
     */
    ParticleSystem system;

    /* Used to identify particles. */
    const Vector<Color> colors = {
        Color::BLACK(), Color::WHITE(), Color::RED(), Color::GREEN(), Color::YELLOW()
    };

    /* Initialize the particles. */
    Vector<Particle> particles;
    for (int i = 0; i < 5; i++) {
        Particle particle;
        particle.x = i;
        particle.dx = 0;
        particle.dy = 1;
        particle.lifetime = 100;
        particle.color = colors[i]; // So we can flag it later
        particles += particle;
    }
    particles[0].lifetime = 0;
    particles[2].lifetime = 0;
    particles[4].lifetime = 0;

    /* Add the particles. */
    for (Particle particle: particles) {
        system.add(particle);
    }
    EXPECT_EQUAL(system.numParticles(), 5);

    /* Move them all. */
    system.moveParticles();
    EXPECT_EQUAL(system.numParticles(), 2); // Three were removed

    /* See where they are. */
    ParticleCatcher catcher;
    system.drawParticles();

    /* Confirm we have particles 1 and 3, in that order. */
    EXPECT_EQUAL(catcher.numDrawn(), 2);
    EXPECT_EQUAL(catcher[0], { 1, 1, colors[1] });
    EXPECT_EQUAL(catcher[1], { 3, 1, colors[3] });

    /* Check the linked list to make sure the wiring is right. */
    ParticleSystem::ParticleCell* curr = system._head;
    ParticleSystem::ParticleCell* prev = nullptr;
    for (int i = 0; i < 2; i++) {
        EXPECT_NOT_EQUAL(curr, nullptr);
        EXPECT_EQUAL(curr->prev, prev);

        prev = curr;
        curr = curr->next;
    }
    EXPECT_EQUAL(curr, nullptr);
}

PROVIDED_TEST("Milestone 4: After all particles expire, can add new particles.") {
    ParticleSystem system;

    /* On entry to this loop, the particle system should be empty.
     * On exit, it should be empty.
     */
    for (int round = 0; round < 10; round++) {
        /* Shouldn't see anything. */
        ParticleCatcher catcher;
        system.drawParticles();
        EXPECT_EQUAL(catcher.numDrawn(), 0);

        /* Create a bunch of short-lived particles. */
        for (int i = 0; i < 5; i++) {
            Particle p;
            p.x = i;
            p.y = i;
            p.lifetime = 2;
            system.add(p);
        }
        EXPECT_EQUAL(system.numParticles(), 5);

        /* Should see five particles. */
        system.drawParticles();
        EXPECT_EQUAL(catcher.numDrawn(), 5);

        /* They should be in the right place. */
        for (int i = 0; i < 5; i++) {
            EXPECT_EQUAL(catcher[i].x, i);
            EXPECT_EQUAL(catcher[i].y, i);
        }

        /* Move the particles a bunch to clear everything out as their
         * lifetimes expire.
         */
        for (int i = 0; i < 10; i++) {
            system.moveParticles();
        }

        /* We should have no particles. */
        EXPECT_EQUAL(system.numParticles(), 0);
        catcher.reset();
        system.drawParticles();
        EXPECT_EQUAL(catcher.numDrawn(), 0);
    }
}

PROVIDED_TEST("Milestone 5: Ballistic particles accelerate downward.") {
    ParticleSystem system;

    Particle ballistic;
    ballistic.type = ParticleType::BALLISTIC;
    ballistic.x = 0;
    ballistic.y = 100;

    /* Fire upward and to the right. */
    ballistic.dx = 1;
    ballistic.dy = -10;

    system.add(ballistic);

    /* Expected positions. */
    Vector<GPoint> positions = {
        {  0, 100 },
        {  1,  90 }, // y += -10
        {  2,  81 }, // y += -9
        {  3,  73 }, // y += -8
        {  4,  66 },
        {  5,  60 },
        {  6,  55 },
        {  7,  51 },
        {  8,  48 },
        {  9,  46 },
        { 10,  45 },
        { 11,  45 }, // y += 0
        { 12,  46 },
        { 13,  48 },
        { 14,  51 },
        { 15,  55 },
        { 16,  60 },
        { 17,  66 },
        { 18,  73 },
        { 19,  81 }, // y += 8
        { 20,  90 }, // y += 9
        { 21, 100 }, // y += 10
    };

    /* Repeatedly check positions and move forward. */
    for (int i = 0; i < positions.size(); i++) {
        ParticleCatcher catcher;
        system.drawParticles();
        EXPECT_EQUAL(catcher.numDrawn(), 1);
        EXPECT_EQUAL(catcher[0].x, positions[i].x);
        EXPECT_EQUAL(catcher[0].y, positions[i].y);

        system.moveParticles();
    }
}

PROVIDED_TEST("Milestone 5: Fireworks accelerate downward.") {
    ParticleSystem system;

    Particle firework;
    firework.type = ParticleType::FIREWORK;
    firework.x = 0;
    firework.y = 100;

    /* Fire upward and to the right. */
    firework.dx = 1;
    firework.dy = -10;

    system.add(firework);

    /* Expected positions. */
    Vector<GPoint> positions = {
        {  0, 100 },
        {  1,  90 }, // y += -10
        {  2,  81 }, // y += -9
        {  3,  73 }, // y += -8
        {  4,  66 },
        {  5,  60 },
        {  6,  55 },
        {  7,  51 },
        {  8,  48 },
        {  9,  46 },
        { 10,  45 },
        { 11,  45 }, // y += 0
        { 12,  46 },
        { 13,  48 },
        { 14,  51 },
        { 15,  55 },
        { 16,  60 },
        { 17,  66 },
        { 18,  73 },
        { 19,  81 }, // y += 8
        { 20,  90 }, // y += 9
        { 21, 100 }, // y += 10
    };

    /* Repeatedly check positions and move forward. */
    for (int i = 0; i < positions.size(); i++) {
        ParticleCatcher catcher;
        system.drawParticles();
        EXPECT_EQUAL(catcher.numDrawn(), 1);
        EXPECT_EQUAL(catcher[0].x, positions[i].x);
        EXPECT_EQUAL(catcher[0].y, positions[i].y);

        system.moveParticles();
    }
}

PROVIDED_TEST("Milestone 5: Fireworks explode when timer expires.") {
    ParticleSystem system;

    /* Create and add a firework. */
    Particle firework;
    firework.type = ParticleType::FIREWORK;
    firework.lifetime = 0; // Explodes as soon as it moves
    firework.x = 100;
    firework.y = 100;
    system.add(firework);

    EXPECT_EQUAL(system.numParticles(), 1);

    /* Move the particles. This should trigger the explosion. */
    system.moveParticles();

    /* There should be 50 new particles, all of which are streamers, and
     * all of which are the same color.
     */
    EXPECT_EQUAL(system.numParticles(), 50);
    EXPECT_NOT_EQUAL(system._head, nullptr);

    /* Color of the first particle. */
    Color color = system._head->particle.color;
    for (auto* curr = system._head; curr != nullptr; curr = curr->next) {
        EXPECT_EQUAL(curr->particle.color, color);
    }
}
