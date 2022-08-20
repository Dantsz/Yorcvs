
#include "Yorcvs.h"
#include "common/utilities.h"

#include <cassert>
#include <iostream>

constexpr size_t number_of_additional_bleeding_ducks = 100;

int main(int argc, char** argv)
{
    yorcvs::Timer timy {};
    yorcvs::ECS world {};
    timy.start();
    yorcvs::Map map { TEST_MAP_FILE, &world };

    std::cout << "Loading map took " << timy.get_ticks() << "ms" << '\n';
    // deleting player

    ;
    // delete playerMovementControlledComponent
    const size_t numberOfEntities = world.get_active_entities_number();

    // no more players
    assert(world.get_entities_with_component<player_movement_controlled_component>() == 0);
    // add ducks(first create valid entity IDs  and test the performance) and then load ducks data into the entities
    // create valid entities
    const size_t first_ad_duck_entity = world.get_active_entities_number();
    timy.start();
    for (size_t i = 0; i < number_of_additional_bleeding_ducks; i++) {
        [[maybe_unused]] const size_t k = world.create_entity_ID();
    }
    std::cout << "Creating valid entities took " << timy.get_ticks() << " ms" << '\n';
    timy.start();

    for (size_t i = 0; i < number_of_additional_bleeding_ducks; i++) {
        map.load_character_from_path(first_ad_duck_entity + i, TEST_TEMP_ENTITY_FILE);
        // remove the collision
        if (world.has_components<hitbox_component>(first_ad_duck_entity + i)) {
            world.remove_component<hitbox_component>(first_ad_duck_entity + i);
        }
    }

    timy.stop();
    std::cout << "Loading the " + std::to_string(number_of_additional_bleeding_ducks) + " ducks took " << timy.get_ticks() << " ms\n";
    // verify they have been added
    assert(world.get_active_entities_number() == numberOfEntities + number_of_additional_bleeding_ducks);

    // update the world so the duck dissapear
    float update_time = 0.0f;
    float samples = 0.0f;
    timy.start();
    while (world.get_active_entities_number() != numberOfEntities - 1) {
        const auto dt = HealthSystem::update_time;
        map.collision_system.update(dt);
        map.velocity_system.update(dt);
        map.animation_system.update(dt);
        map.health_system.update(dt);

        timy.stop();
        update_time += timy.get_ticks();
        samples += 1.0f;
        timy.start();
    }

    timy.stop();
    update_time += timy.get_ticks();
    samples += 1.0f;
    std::cout << "Updating the map " << samples << " times  took " << update_time << " (avg : " << update_time / samples << " )\n ";
    assert(world.get_active_entities_number() == numberOfEntities - 1);
    return 0;
}
