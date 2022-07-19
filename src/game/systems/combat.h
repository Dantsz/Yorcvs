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
    /**
     * @brief attack
     * @param source
     * @param target
     * @return the amount of damaged dealt
     */
    [[nodiscard]] float attack(size_t source, size_t target) const
    {
        const auto& source_stats = world->get_component<offensiveStatsComponent>(source);
        const auto& target_stats = world->get_component<defensiveStatsComponent>(target);

        // add strength bonus
        float damage = calculate_strength_bonus(source_stats.strength);
        // check if critical
        const float agility_roll = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        if (agility_roll < calculate_agility_bonus(source_stats.agility)) {
            damage *= crititcal_multiplier;
        }
        // check if dodged
        const float dodge_roll = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        if (dodge_roll < calculate_dodge_chance(target_stats.dodge)) {
            return 0.0f;
        }
        // check if blocked
        const float block_roll = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        if (block_roll < calculate_block_chance(target_stats.block)) {
            damage /= block_multiplier;
        }

        world->get_component<healthComponent>(target).HP -= damage;
        return damage;
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
     * @return constexpr float the block values, a number between 0.0 and 1.0 representing the chance the character block the attack, blocked attacks deal a quarter damage
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
    static constexpr float crititcal_multiplier = 2.0f;
    static constexpr float block_multiplier = 4.0f;
};
