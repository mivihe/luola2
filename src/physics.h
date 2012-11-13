#ifndef LUOLA_PHYSICS
#define LUOLA_PHYSICS

#include <glm/glm.hpp>

/**
 * A physical object.
 */
class Physical {
public:
    /// Physics simulation time step, or delta T.
    static const float TIMESTEP;

    /**
     * Default constructor
     */
    Physical();

    /**
     * Construct a new physical object
     *
     * @param pos initial position
     * @param mass mass
     * @param radius radius
     */
    Physical(const glm::vec2 &pos, float mass, float radius);

    /**
     * Get the position of the object
     *
     * @return position vector
     */
    const glm::vec2 &position() const { return m_pos; }

    /**
     * Set the position of the object
     *
     * @param pos new position
     */
    void setPosition(const glm::vec2 pos) { m_pos = pos; }

    /**
     * Get the velocity of the object
     *
     * @return velocity vector
     */
    const glm::vec2 &velocity() const { return m_vel; }

    /**
     * Set the velocity of the object.
     *
     * Note. Typically you should use the addImpulse function
     * to change the velocity. Use this only for initialization
     * and when you need to break the laws of physics.
     *
     * @param vel new velocity
     */
    void setVelocity(const glm::vec2 &vel) { m_vel = vel; }

    /**
     * Add an impulse to the impulse accumulator.
     *
     * The total impulse will be applied on the next simulation step.
     *
     * @param force impulse force
     */
    void addImpulse(const glm::vec2 &impulse) { m_imp += impulse; }

    /**
     * Get the impulse accumulator
     *
     * The impulse accumulator holds the impulse force
     * that will be applied to the object at the integration step.
     *
     * @return accumulated impulse
     */
    const glm::vec2 impulse() const { return m_imp; }

    /**
     * Get the mass of the object
     *
     * @return mass
     */
    float mass() const { return m_mass; }

    /**
     * Get the mass⁻¹
     *
     * This is just a shortcut for certain calculations.
     *
     * @return inverse mass
     */
    float imass() const { return 1.0f / m_mass; }

    /**
     * Change the mass of the object
     *
     * @param mass new mass
     */
    void setMass(float mass) { m_mass = mass; }

    /**
     * Get the radius of the object.
     *
     * The radius is used for collision detection and air
     * resistance calculation.
     *
     * @return object radius
     */
    float radius() const { return m_radius; }

    /**
     * Set the radius of the object.
     *
     * The radius must be greater than zero.
     *
     * @param radius object radius
     */
    void setRadius(float radius) { assert(radius>0); m_radius = radius; }

    /**
     * Simulate a timestep.
     */
    void step();

    /**
     * Check if this object is currently colliding with the other object.
     *
     * @param object the other object to check
     * @return true if this object is in collision with the other one.
     */
    bool checkCollision(Physical &other);

private:
    // Position
    glm::vec2 m_pos;

    // Velocity
    glm::vec2 m_vel;

    // Impulse accumulator
    glm::vec2 m_imp;

    float m_mass;
    float m_radius;
};

#endif
