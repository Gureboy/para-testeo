#define MOVE_TO_TARGET 1
#define EGG_LAY 2

// Definición de la criatura "otherthing"
/mob/living/simple_animal/hostile/creature
    name = "creature"
    desc = "A sanity-destroying otherthing."
    icon = 'icons/mob/critter.dmi'
    speak_emote = list("gibbers")
    icon_state = "otherthing"
    icon_living = "otherthing"
    icon_dead = "otherthing-dead"
    health = 500
    maxHealth = 500
    melee_damage_lower = 25
    melee_damage_upper = 50
    attacktext = "chomps"
    attack_sound = 'sound/weapons/bite.ogg'
    faction = "creature"
    speed = 4
    var/has_loot = TRUE
    var/objects_destroyed = 0 // contador de objetos destruidos
    var/busy = FALSE
    var/target_obj // objeto objetivo

// Procedimiento para destruir el objeto y actualizar el contador
/mob/living/simple_animal/hostile/creature/DestroyObject(obj/O)
    if(O)
        src.visible_message(SPAN_NOTICE("\the [src] destroys \the [O]."))
        del O
        objects_destroyed++
        if(objects_destroyed >= 2)
            LayEggs()
            objects_destroyed = 0 // reiniciar contador después de poner huevos

// Procedimiento para buscar y destruir objetos cercanos
/mob/living/simple_animal/hostile/creature/Life(delta_time)
    ..()
    if(!stat)
        var/list/nearby_objects = view(src, 5)
        if(!busy && prob(30)) // 30% de probabilidad de buscar un objeto para destruir
            for(var/obj/O in nearby_objects)
                if(!O.anchored) // solo destruir objetos no anclados
                    busy = MOVE_TO_TARGET
                    target_obj = O
                    walk_to(src, target_obj, 1, speed)
                    spawn(20) // esperar un poco antes de destruir el objeto
                        if(busy == MOVE_TO_TARGET && target_obj && get_dist(src, target_obj) <= 1)
                            DestroyObject(target_obj)
                            busy = FALSE
                    return

// Procedimiento para poner huevos después de destruir 2 objetos
/mob/living/simple_animal/hostile/creature/proc/LayEggs()
    busy = EGG_LAY
    src.visible_message(SPAN_NOTICE("\the [src] lays a clutch of eggs on the ground."))
    spawn(40) // retraso para la animación de poner huevos
        if(busy == EGG_LAY)
            new /obj/effect/eggcluster(src.loc)
            busy = FALSE
