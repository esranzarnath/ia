#ifndef DATA_H
#define DATA_H

#include <vector>
#include <string>

#include "cmn_data.hpp"
#include "ability_values.hpp"
#include "spells.hpp"
#include "feature_data.hpp"
#include "audio.hpp"

enum class Snd_vol;

enum class Item_weight
{
    none        = 0,
    extra_light = 1,  //E.g. ammo
    light       = 10, //E.g. dynamite, daggers
    medium      = 50, //E.g. most firearms
    heavy       = 110 //E.g. heavy armor, heavy weapons
};

enum class Main_att_mode
{
    none,
    melee,
    thrown,
    ranged
};

enum class Item_value
{
    normal,
    minor_treasure,
    major_treasure
};

enum class Item_id
{
    trapezohedron,

    thr_knife,
    rock,
    iron_spike,
    dagger,
    hatchet,
    club,
    hammer,
    machete,
    axe,
    pitch_fork,
    sledge_hammer,

    pharaoh_staff,

    sawed_off,
    pump_shotgun,
    machine_gun,
    incinerator,
    spike_gun,
    shotgun_shell,
    drum_of_bullets,
    incinerator_ammo,
    pistol,
    pistol_clip,
    flare_gun,

    trap_dart,
    trap_dart_poison,
    trap_spear,
    trap_spear_poison,

    dynamite,
    flare,
    molotov,
    smoke_grenade,

    player_kick,
    player_stomp,
    player_punch,
    player_ghoul_claw,

    rat_bite,
    rat_bite_diseased,
    rat_thing_bite,
    brown_jenkin_bite,
    worm_mass_bite,
    green_spider_bite,
    white_spider_bite,
    red_spider_bite,
    shadow_spider_bite,
    leng_spider_bite,
    ghoul_claw,
    shadow_claw,
    byakhee_claw,
    giant_mantis_claw,
    giant_locust_bite,
    fire_hound_breath, frost_hound_breath,
    fire_hound_bite, frost_hound_bite, zuul_bite,
    raven_peck,
    giant_bat_bite,
    zombie_axe,
    zombie_claw,
    zombie_claw_diseased,
    bloated_zombie_punch,
    bloated_zombie_spit,
    crawling_intestines_strangle,
    crawling_hand_strangle, thing_strangle,
    wolf_bite,
    ghost_claw,
    phantasm_sickle,
    wraith_claw,
    mi_go_gun,
    mi_go_gun_ammo,
    polyp_tentacle,
    greater_polyp_tentacle,
    mummy_maul,
    croc_head_mummy_spear,
    deep_one_spear_att,
    deep_one_javelin_att,
    ape_maul,
    ooze_black_spew_pus,
    ooze_putrid_spew_pus,
    ooze_poison_spew_pus,
    ooze_clear_spew_pus,
    color_oo_space_touch,
    chthonian_bite,
    death_fiend_claw,
    hunting_horror_bite,
    dust_vortex_engulf, fire_vortex_engulf, frost_vortex_engulf,
    mold_spores,
    mi_go_sting, mi_go_commander_sting,
    the_high_priest_claw,

    armor_leather_jacket,
    armor_iron_suit,
    armor_flack_jacket,
    armor_asb_suit,
    armor_heavy_coat,
    armor_mi_go,

    gas_mask,
//    hideous_mask,

    scroll_pest,
    scroll_telep,
    scroll_slow_mon,
    scroll_terrify_mon,
    scroll_paral_mon,
    scroll_det_traps,
    scroll_det_items,
    scroll_det_mon,
    scroll_bless,
    scroll_mayhem,
    scroll_darkbolt,
    scroll_aza_wrath,
    scroll_opening,
    scroll_sacr_life,
    scroll_sacr_spi,
    scroll_elem_res,
    scroll_summon_mon,
    scroll_light,

    potion_vitality,
    potion_spirit,
    potion_blindness,
    potion_frenzy,
    potion_fortitude,
    potion_paralyze,
    potion_rElec,
    potion_conf,
    potion_poison,
    potion_insight,
    potion_clairv,
    potion_rFire,
    potion_antidote,
    potion_descent,
    potion_invis,
    potion_seeing,

    device_blaster,
    device_shockwave,
    device_rejuvenator,
    device_translocator,
    device_sentry_drone,

    electric_lantern,

    medical_bag,

    star_amulet,
    skull_amulet,
    spider_amulet,
    eye_amulet,
    moon_amulet,
    bat_amulet,
    scarab_amulet,
    dagger_amulet,

    golden_ring,
    silver_ring,
    carnelian_ring,
    garnet_ring,
    iron_ring,
    jade_ring,
    moonstone_ring,
    obsidian_ring,
    onyx_ring,
    topaz_ring,
    emerald_ring,

    END
};

struct Item_container_spawn_rule
{
    Item_container_spawn_rule() :
        feature_id          (Feature_id::END),
        pct_chance_to_incl  (0) {}

    Item_container_spawn_rule(Feature_id feature_id_, int pct_chance_to_incl_) :
        feature_id          (feature_id_),
        pct_chance_to_incl  (pct_chance_to_incl_) {}

    Feature_id feature_id;
    int pct_chance_to_incl;
};

class Item_data_t
{
public:
    Item_data_t();

    ~Item_data_t() {}

    Item_id                     id;
    Item_type                   type;
    bool                        has_std_activate; //E.g. potions and scrolls
    bool                        is_prio_in_backpack_list; //E.g. Medical Bag
    Item_value                  value;
    Item_weight                 weight;
    bool                        allow_spawn;
    Range                       spawn_std_range;
    int                         max_stack_at_spawn;
    int                         chance_to_incl_in_floor_spawn_list;
    bool                        is_stackable;
    bool                        is_identified;
    bool                        is_tried;
    Item_name                   base_name;
    Item_name                   base_name_un_id;
    std::vector<std::string>    base_descr;
    char                        glyph;
    Clr                         clr;
    Tile_id                     tile;
    Main_att_mode               main_att_mode;
    Spell_id                    spell_cast_from_scroll;
    std::string                 land_on_hard_snd_msg;
    Sfx_id                      land_on_hard_sfx;
    int                         shock_while_in_backpack;
    int                         shock_while_equipped;

    std::vector<Room_type>                  native_rooms;
    std::vector<Item_container_spawn_rule>  container_spawn_rules;

    int ability_mods_while_equipped[int(Ability_id::END)];

    struct Item_melee_data
    {
        Item_melee_data();
        ~Item_melee_data();

        bool                    is_melee_wpn;
        std::pair<int, int>     dmg;
        int                     hit_chance_mod;
        Item_att_msgs           att_msgs;
        Prop*                   prop_applied;
        Dmg_type                dmg_type;
        bool                    knocks_back;
        Sfx_id                  hit_small_sfx;
        Sfx_id                  hit_medium_sfx;
        Sfx_id                  hit_hard_sfx;
        Sfx_id                  miss_sfx;
    } melee;

    struct Item_ranged_data
    {
        Item_ranged_data();
        ~Item_ranged_data();

        bool                    is_ranged_wpn, is_machine_gun, is_shotgun;
        //NOTE: This property should be set on ranged weapons (using ammo) and clips
        int                     max_ammo;
        Dice_param              dmg;
        Dice_param              throw_dmg;
        int                     hit_chance_mod;
        int                     throw_hit_chance_mod;
        int                     effective_range;
        bool                    knocks_back;
        std::string             dmg_info_override;
        Item_id                 ammo_item_id;
        Dmg_type                dmg_type;
        bool                    has_infinite_ammo;
        char                    projectile_glyph;
        Tile_id                 projectile_tile;
        Clr                     projectile_clr;
        bool                    projectile_leaves_trail;
        bool                    projectile_leaves_smoke;
        Item_att_msgs           att_msgs;
        std::string             snd_msg;
        Snd_vol                 snd_vol;
        bool                    makes_ricochet_snd;
        Sfx_id                  att_sfx;
        Sfx_id                  reload_sfx;
        Prop*                   prop_applied;
    } ranged;

    struct Item_armor_data
    {
        Item_armor_data();

        int                     armor_points;
        double                  dmg_to_durability_factor;
    } armor;
};

class Item;

namespace item_data
{

extern Item_data_t data[int(Item_id::END)];

void init();
void cleanup();

void store_to_save_lines(std::vector<std::string>& lines);
void setup_from_save_lines(std::vector<std::string>& lines);

} //Item_data

#endif
