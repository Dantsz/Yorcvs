#pragma once
#include "../../common/ecs.h"
#include "../components.h"

/**
 * @brief Handles combat
 *
 */
class CombatSystem {
public:
    explicit CombatSystem(yorcvs::ECS* parent)
        : world(parent)
    {
        world->register_system(*this);
        world->add_criteria_for_iteration<CombatSystem, healthComponent, offensiveStatsComponent, defensiveStatsComponent>();
    }
    void attack(size_t source, size_t target) const
    {
        // TODO: implement attacking
        float damage = world->get_component<offensiveStatsComponent>(source).strength;
        world->get_component<healthComponent>(target).HP -= damage;
    }
    /**
     * @brief Calculates the percentage of the damage reduced by the armor stat
     *
     * @param armor
     * @return constexpr float
     */
    static constexpr float calculate_armor_damage_reduction(float armor)
    {
        constexpr float maximum_reduction = 95.0f; // the limits to infinity
        constexpr float slope = 2000.0f; // not the actual slope, but this affects the slope calculation
        if (armor < 0.0f) {
            armor *= -1.0f;
        }
        return (maximum_reduction * armor) / (armor + slope);
    }
    /**
     * @brief Calculates the block value from the block stat
     *
     * @param block block stat
     * @return constexpr float the block values, a number between 0.0 and 1.0 representing the chance the character block the attack
     */
    static constexpr float calculate_block_chance(float block)
    {
        constexpr float slope = 1500.0f;
        if (block < 0.0f) {
            block *= -1.0f;
        }
        return block / (block + slope);
    }
    /**
     * @brief Calculates the dodge value from the dodge stat
     *
     * @param dodge
     * @return constexpr float
     */
    static constexpr float calculate_dodge_chance(float dodge)
    {
        constexpr float slope = 2000.0f;
        if (dodge < 0.0f) {
            dodge *= -1.0f;
        }
        return dodge / (dodge + slope);
    }
    /**
     * @brief Calculates the bonus damage from the strength stat
     *
     * @param strength
     * @return constexpr float
     */
    static constexpr float calculate_strength_bonus(float strength)
    {
        return strength;
    }
    /**
     * @brief Calculates the added chance for an attack to do double damage
     *
     * @param agility
     * @return constexpr float
     */
    static constexpr float calculate_agility_bonus(float agility)
    {
        constexpr float slope = 2500.0f;
        if (agility < 0.0f) {
            agility *= -1.0f;
        }
        return agility / (agility + slope);
    }
    std::shared_ptr<yorcvs::EntitySystemList> entityList = nullptr;
    yorcvs::ECS* world = nullptr;
};
