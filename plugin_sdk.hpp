#pragma once
#include <cstdint>
#include <string>
#include <functional>
#include <vector>
#include <memory>
#include <algorithm>
#include "clipper.h"
#include <map>

extern "C++" {

	template <size_t S>
	struct _ENUM_FLAG_INTEGER_FOR_SIZE;

	template <>
	struct _ENUM_FLAG_INTEGER_FOR_SIZE<1>
	{
		typedef std::int8_t type;
	};

	template <>
	struct _ENUM_FLAG_INTEGER_FOR_SIZE<2>
	{
		typedef std::int16_t type;
	};

	template <>
	struct _ENUM_FLAG_INTEGER_FOR_SIZE<4>
	{
		typedef std::int32_t type;
	};

	template <>
	struct _ENUM_FLAG_INTEGER_FOR_SIZE<8>
	{
		typedef std::int64_t type;
	};

	template <class T>
	struct _ENUM_FLAG_SIZED_INTEGER
	{
		typedef typename _ENUM_FLAG_INTEGER_FOR_SIZE<sizeof( T )>::type type;
	};

}
#define _ENUM_FLAG_CONSTEXPR constexpr

#define DEFINE_ENUM_FLAG_OPERATORS(ENUMTYPE) \
extern "C++" { \
inline _ENUM_FLAG_CONSTEXPR ENUMTYPE operator | (ENUMTYPE a, ENUMTYPE b) throw() { return ENUMTYPE(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)a) | ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
inline ENUMTYPE &operator |= (ENUMTYPE &a, ENUMTYPE b) throw() { return (ENUMTYPE &)(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type &)a) |= ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
inline _ENUM_FLAG_CONSTEXPR ENUMTYPE operator & (ENUMTYPE a, ENUMTYPE b) throw() { return ENUMTYPE(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)a) & ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
inline ENUMTYPE &operator &= (ENUMTYPE &a, ENUMTYPE b) throw() { return (ENUMTYPE &)(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type &)a) &= ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
inline _ENUM_FLAG_CONSTEXPR ENUMTYPE operator ~ (ENUMTYPE a) throw() { return ENUMTYPE(~((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)a)); } \
inline _ENUM_FLAG_CONSTEXPR ENUMTYPE operator ^ (ENUMTYPE a, ENUMTYPE b) throw() { return ENUMTYPE(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)a) ^ ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
inline ENUMTYPE &operator ^= (ENUMTYPE &a, ENUMTYPE b) throw() { return (ENUMTYPE &)(((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type &)a) ^= ((_ENUM_FLAG_SIZED_INTEGER<ENUMTYPE>::type)b)); } \
}

#ifdef INTERNAL_CORE
const static bool is_internal = true;
#else
const static bool is_internal = false;
#endif

#define PLUGIN_API	extern "C" __declspec(dllexport)
#define PLUGIN_SDK_VERSION 1

/**
 * Sets supported champions by your plugin
 * 
 * You have to use that macro once in your main file
 *
 * Example: SUPPORTED_CHAMPIONS(champion_id::Annie, champion_id::Ashe);
 */
#define SUPPORTED_CHAMPIONS(...) PLUGIN_API \
champion_id supported_champions[ ] = { __VA_ARGS__ , champion_id::Unknown };

/**
 * Sets name of your plugin
 * Max length is 64 including null character
 * 
 * You have to use that macro once in your main file
 * 
 * Example: PLUGIN_NAME("Test Plugin");
 */
#define PLUGIN_NAME(x) PLUGIN_API const char plugin_name[64] = x;

 /**
  * Initialize global variables for your plugin
  * You have to call it otherwise your plugin will crash
  *
  * Call it instantly at beginning of on_sdk_load function with plugin_sdk_core as parameter
  * 
  * Example: DECLARE_GLOBALS(plugin_sdk_good);
  */
#define DECLARE_GLOBALS(PLUGIN_SDK) \
        plugin_sdk         = PLUGIN_SDK; \
		state              = PLUGIN_SDK->get_game_state(); \
		renderer           = PLUGIN_SDK->get_r3d_renderer(); \
		hud                = PLUGIN_SDK->get_hud_manager(); \
		gui                = PLUGIN_SDK->get_menu_gui(); \
		gametime		   = PLUGIN_SDK->get_game_time(); \
		ping			   = PLUGIN_SDK->get_game_ping(); \
		game_input         = PLUGIN_SDK->get_input(); \
		event_manager      = PLUGIN_SDK->get_game_event_manager(); \
		navmesh            = PLUGIN_SDK->get_nav_mesh(); \
		keyboard_state     = PLUGIN_SDK->get_game_keyboard_state(); \
		locale             = PLUGIN_SDK->get_locale_manager(); \
		missioninfo        = PLUGIN_SDK->get_mission_info(); \
		myhero             = PLUGIN_SDK->get_myhero(); \
		target_selector    = PLUGIN_SDK->get_target_selector_manager(); \
		prediction         = PLUGIN_SDK->get_prediction_manager(); \
		menu               = PLUGIN_SDK->get_tree_menu(); \
		health_prediction  = PLUGIN_SDK->get_health_prediction_manager(); \
		orbwalker          = PLUGIN_SDK->get_orbwalker_manager(); \
		damagelib          = PLUGIN_SDK->get_damagelib_manager(); \
		draw_manager       = PLUGIN_SDK->get_drawning_manager(); \
		scheduler		   = PLUGIN_SDK->get_scheduler_manager(); \
		console		       = PLUGIN_SDK->get_console_manager(); \
		glow		       = PLUGIN_SDK->get_glow_manager(); \
		sound		       = PLUGIN_SDK->get_sound_manager(); \
		entitylist		   = PLUGIN_SDK->get_entity_list();


#define D3DCOLOR_ARGB(a,r,g,b) \
    ((unsigned long)((((a)&0xff)<<24)|(((b)&0xff)<<16)|(((g)&0xff)<<8)|((r)&0xff)))


/**
 * Creates color value from RGBA
 * Example: MAKE_COLOR(255,0,255,255);
 */
#define MAKE_COLOR(r,g,b,a) D3DCOLOR_ARGB(a,r,g,b) 

/**
 * Creates buff name hash for given buff name
 * 
 * Use that macro when checking for buff or getting a buff
 * We don't support getting buffs by their name, you have to pass hash
 * 
 * buff_hash is ignorecase hashing function
 * 
 * Example: buff_hash("ZeriR");
 */
#define buff_hash(str) (std::integral_constant<std::uint32_t, buff_hash_real(str)>::value)

/**
 * Creates spell name hash for given spell name
 * 
 * We recommend to use that when comparing spells name it's a lot faster than string compare
 * 
 * spell_data->get_name_hash() == spell_hash("ZeriR")
 * 
 * spell_hash is ignorecase hashing function
 * 
 * Example: spell_hash("ZeriR");
 */
#define spell_hash(str) (std::integral_constant<std::uint32_t, spell_hash_real(str)>::value)


 /**
  * Same as buff_hash but you can use it in runtime
  *
  * Example: buff_hash_real("ZeriR");
  */
constexpr std::uint32_t __forceinline const buff_hash_real( const char* str )
{
	std::uint32_t hash = 0x811C9DC5;
	std::uint32_t len = 0;

	while ( str[ len ] != '\0' )
		len++;

	for ( auto i = 0u; i < len; ++i )
	{
		char current = str[ i ];
		char current_upper = current + 0x20;

		if ( static_cast< uint8_t >( current - 0x41 ) > 0x19u )
			current_upper = current;

		hash = 16777619 * ( hash ^ current_upper );
	}

	return hash;
}

/**
 * Same as spell_hash but you can use it in runtime
 *
 * Example: spell_hash_real("ZeriR");
 */
constexpr std::uint32_t __forceinline const spell_hash_real( const char* str ) /*use for script_spell* name*/
{
	std::uint32_t hash = 0;
	std::uint32_t len = 0;

	while ( str[ len ] != '\0' )
		len++;

	for ( auto i = 0u; i < len; ++i )
	{
		char current = str[ i ];
		char current_upper = current + 0x20;

		if ( static_cast< uint8_t >( current - 0x41 ) > 0x19u )
			current_upper = current;

		hash = current_upper + 0x10 * hash;

		if ( hash & 0xF0000000 )
			hash ^= (hash & 0xF0000000) ^ ( ( hash & 0xF0000000 ) >> 24 );
	}

	return hash;
}

class game_object;
class path_controller;
class buff_instance;
class spell_data;
class spell_data_inst;
class item;
class spell_instance;
class object_type;
class character_data;

#ifdef INTERNAL_CORE
using game_object_script = std::shared_ptr<game_object>;
using path_controller_script = std::shared_ptr<path_controller>;
using buff_instance_script = std::shared_ptr<buff_instance>;
using spell_data_script = std::shared_ptr<spell_data>;
using spell_data_inst_script = std::shared_ptr<spell_data_inst>;
using item_script = std::shared_ptr<item>;
using spell_instance_script = std::shared_ptr<spell_instance>;
using object_type_script = std::shared_ptr<object_type>;
using character_data_script = std::shared_ptr<character_data>;
#else
using game_object_script = game_object*;
using path_controller_script = path_controller*;
using buff_instance_script = buff_instance*;
using spell_data_script = spell_data*;
using spell_data_inst_script = spell_data_inst*;
using item_script = item*;
using spell_instance_script = spell_instance*;
using object_type_script = object_type*;
using character_data_script = character_data*;
#endif

enum class game_state_stage
{
	loading_screen = 0,
	connecting = 1,
	running = 2,
	oaused = 3,
	finished = 4,
	exiting = 5
};

enum class teleport_type: int
{
	Recall,
	SuperRecall,
	Teleport,
	TwistedFate,
	Shen
};

enum class teleport_status: int
{
	Start,
	Finish,
	Abort
};

struct teleport_info
{
	float start;
	float duration;
	teleport_type type;
};

class game_time
{
public:
	virtual float get_time( ) = 0;
	virtual double get_prec_time( ) = 0;
};

class game_state
{
public:
	virtual game_state_stage get_stage( ) = 0;
};

#define M_PI 3.14159265358979323846f

constexpr float degrees_to_radians( float angle )
{
	return angle * M_PI / 180.f;
}

struct projection_info;
struct intersection_result;

class point2
{
public:
	int32_t x;
	int32_t y;

	point2( int32_t x, int32_t y );

	point2( );

	bool operator==( const point2& vOther ) const;
	bool operator!=( const point2& vOther ) const;

	point2 operator+( const point2& v ) const;
	point2 operator-( const point2& v ) const;
	point2 operator*( const point2& v ) const;
	point2 operator/( const point2& v ) const;
};

class game_object;
class vector
{
public:
	static vector zero;

	float x;
	float z;
	float y;

	vector( );
	vector( float x, float y );
	vector( float x, float y, float z );
	vector( point2 p2 );

	float length( ) const;
	float length_sqr( void ) const;
	float distance( const vector& vOther ) const;
	float distance( game_object_script unit ) const;
	float distance( const vector& segment_start, const vector& segment_end, bool only_if_on_segment = false, bool squared = false ) const;
	float distance_squared( const vector& vOther ) const;
	float dot_product( const vector& other ) const;
	float cross_product( const vector& other ) const;
	float polar( ) const;
	float angle_between( const vector& other ) const;
	vector extend( const vector& to, float range ) const;
	vector normalized( ) const;
	vector rotated( float angle ) const;
	vector perpendicular( ) const;
	vector set_z( float value = -1.f ) const;

	projection_info project_on( const vector& segment_start, const vector& segment_end ) const;
	intersection_result intersection( const vector& line_segment_end, const vector& line_segment2_start, const vector& line_segment2_end ) const;

	vector& operator=( const vector& vOther );

	vector operator-( ) const;
	vector operator+( const vector& v ) const;
	vector operator-( const vector& v ) const;
	vector operator*( const vector& v ) const;
	vector operator/( const vector& v ) const;
	vector operator*( float fl ) const;
	vector operator/( float fl ) const;
	
	//Checks whether the object is still valid meaning if its still in the game
	//
	bool is_valid( ) const;

	bool operator==( const vector& v_other ) const;
	bool operator!=( const vector& v_other ) const;

	bool is_wall( ) const;
	bool is_wall_of_grass( ) const;
	bool is_in_fow( ) const;
	bool is_building( ) const;
	bool is_on_screen( ) const;
	bool is_under_ally_turret( ) const;
	bool is_under_enemy_turret( ) const;
	int count_enemies_in_range( float range ) const;
	int count_allys_in_range( float range, game_object_script original_unit = nullptr ) const;
};

struct projection_info
{
	bool is_on_segment;
	vector line_point;
	vector segment_point;

	projection_info( bool is_on_segment, vector const& segment_point, vector const& line_point );
};

struct intersection_result
{
	bool intersects;
	vector point;
	bool is_collinear = false;

	intersection_result( bool intersects = false, vector const& point = vector( ) );
};

struct PKT_S2C_PlayAnimationArgs
{
	const char* animation_name;
};

enum class pkttype_e: std::uint16_t
{
	PKT_S2C_PlayAnimation_s, //PKT_S2C_PlayAnimationArgs
	PKT_S2C_ForceCreateMissile_s //nullptr
};

enum class game_object_team
{
	unknown,
	order = 100,
	chaos = 200,
	neutral = 300
};

enum class ItemId: uint32_t
{
	Boots = 1001,
	Faerie_Charm = 1004,
	Rejuvenation_Bead = 1006,
	Giants_Belt = 1011,
	Cloak_of_Agility = 1018,
	Blasting_Wand = 1026,
	Sapphire_Crystal = 1027,
	Ruby_Crystal = 1028,
	Cloth_Armor = 1029,
	Chain_Vest = 1031,
	Null_Magic_Mantle = 1033,
	Emberknife = 1035,
	Long_Sword = 1036,
	Pickaxe = 1037,
	B_F_Sword = 1038,
	Hailblade = 1039,
	Dagger = 1042,
	Recurve_Bow = 1043,
	Amplifying_Tome = 1052,
	Vampiric_Scepter = 1053,
	Dorans_Shield = 1054,
	Dorans_Blade = 1055,
	Dorans_Ring = 1056,
	Negatron_Cloak = 1057,
	Needlessly_Large_Rod = 1058,
	Dark_Seal = 1082,
	Cull = 1083,
	Health_Potion = 2003,
	Total_Biscuit_of_Everlasting_Will = 2010,
	Kircheis_Shard = 2015,
	Refillable_Potion = 2031,
	Corrupting_Potion = 2033,
	Guardians_Horn = 2051,
	Poro_Snax = 2052,
	Control_Ward = 2055,
	Shurelyas_Battlesong = 2065,
	Elixir_of_Iron = 2138,
	Elixir_of_Sorcery = 2139,
	Elixir_of_Wrath = 2140,
	Minion_Dematerializer = 2403,
	Commencing_Stopwatch = 2419,
	Stopwatch = 2420,
	Broken_Stopwatch = 2421,
	Slightly_Magical_Footware = 2422,
	Perfectly_Timed_Stopwatch = 2423,
	Abyssal_Mask = 3001,
	Archangels_Staff = 3003,
	Manamune = 3004,
	Berserkers_Greaves = 3006,
	Boots_of_Swiftness = 3009,
	Chemtech_Putrifier = 3011,
	Sorcerers_Shoes = 3020,
	Glacial_Buckler = 3024,
	Guardian_Angel = 3026,
	Infinity_Edge = 3031,
	Mortal_Reminder = 3033,
	Last_Whisper = 3035,
	Lord_Dominiks_Regards = 3036,
	Mejais_Soulstealer = 3041,
	Muramana = 3042,
	Phage = 3044,
	Phantom_Dancer = 3046,
	Plated_Steelcaps = 3047,
	Seraphs_Embrace = 3048,
	Zekes_Convergence = 3050,
	Hearthbound_Axe = 3051,
	Steraks_Gage = 3053,
	Sheen = 3057,
	Spirit_Visage = 3065,
	Winged_Moonplate = 3066,
	Kindlegem = 3067,
	Sunfire_Aegis = 3068,
	Tear_of_the_Goddess = 3070,
	Black_Cleaver = 3071,
	Bloodthirster = 3072,
	Ravenous_Hydra = 3074,
	Thornmail = 3075,
	Bramble_Vest = 3076,
	Tiamat = 3077,
	Trinity_Force = 3078,
	Wardens_Mail = 3082,
	Warmogs_Armor = 3083,
	Runaans_Hurricane = 3085,
	Zeal = 3086,
	Rabadons_Deathcap = 3089,
	Wits_End = 3091,
	Rapid_Firecannon = 3094,
	Stormrazor = 3095,
	Lich_Bane = 3100,
	Banshees_Veil = 3102,
	Aegis_of_the_Legion = 3105,
	Redemption = 3107,
	Fiendish_Codex = 3108,
	Knights_Vow = 3109,
	Frozen_Heart = 3110,
	Mercurys_Treads = 3111,
	Guardians_Orb = 3112,
	Aether_Wisp = 3113,
	Forbidden_Idol = 3114,
	Nashors_Tooth = 3115,
	Rylais_Crystal_Scepter = 3116,
	Mobility_Boots = 3117,
	Executioners_Calling = 3123,
	Guinsoos_Rageblade = 3124,
	Caulfields_Warhammer = 3133,
	Serrated_Dirk = 3134,
	Void_Staff = 3135,
	Mercurial_Scimitar = 3139,
	Quicksilver_Sash = 3140,
	Youmuus_Ghostblade = 3142,
	Randuins_Omen = 3143,
	Hextech_Alternator = 3145,
	Hextech_Rocketbelt = 3152,
	Blade_of_the_Ruined_King = 3153,
	Hexdrinker = 3155,
	Maw_of_Malmortius = 3156,
	Zhonyas_Hourglass = 3157,
	Ionian_Boots_of_Lucidity = 3158,
	Morellonomicon = 3165,
	Guardians_Blade = 3177,
	Umbral_Glaive = 3179,
	Sanguine_Blade = 3181,
	Guardians_Hammer = 3184,
	Locket_of_the_Iron_Solari = 3190,
	Seekers_Armguard = 3191,
	Gargoyle_Stoneplate = 3193,
	Spectres_Cowl = 3211,
	Mikaels_Blessing = 3222,
	Scarecrow_Effigy = 3330,
	Stealth_Ward = 3340,
	Farsight_Alteration = 3363,
	Oracle_Lens = 3364,
	Your_Cut = 3400,
	Ardent_Censer = 3504,
	Essence_Reaver = 3508,
	Eye_of_the_Herald = 3513,
	Kalistas_Black_Spear = 3599,
	Dead_Mans_Plate = 3742,
	Titanic_Hydra = 3748,
	Crystalline_Bracer = 3801,
	Lost_Chapter = 3802,
	Edge_of_Night = 3814,
	Spellthiefs_Edge = 3850,
	Frostfang = 3851,
	Shard_of_True_Ice = 3853,
	Steel_Shoulderguards = 3854,
	Runesteel_Spaulders = 3855,
	Pauldrons_of_Whiterock = 3857,
	Relic_Shield = 3858,
	Targons_Buckler = 3859,
	Bulwark_of_the_Mountain = 3860,
	Spectral_Sickle = 3862,
	Harrowing_Crescent = 3863,
	Black_Mist_Scythe = 3864,
	Oblivion_Orb = 3916,
	Imperial_Mandate = 4005,
	Force_of_Nature = 4401,
	The_Golden_Spatula = 4403,
	Horizon_Focus = 4628,
	Cosmic_Drive = 4629,
	Blighting_Jewel = 4630,
	Verdant_Barrier = 4632,
	Riftmaker = 4633,
	Leeching_Leer = 4635,
	Night_Harvester = 4636,
	Demonic_Embrace = 4637,
	Watchful_Wardstone = 4638,
	Stirring_Wardstone = 4641,
	Bandleglass_Mirror = 4642,
	Vigilant_Wardstone = 4643,
	Ironspike_Whip = 6029,
	Silvermere_Dawn = 6035,
	Deaths_Dance = 6333,
	Chempunk_Chainsword = 6609,
	Staff_of_Flowing_Water = 6616,
	Moonstone_Renewer = 6617,
	Goredrinker = 6630,
	Stridebreaker = 6631,
	Divine_Sunderer = 6632,
	Liandrys_Anguish = 6653,
	Ludens_Tempest = 6655,
	Everfrost = 6656,
	Bamis_Cinder = 6660,
	Frostfire_Gauntlet = 6662,
	Turbo_Chemtank = 6664,
	Noonquiver = 6670,
	Galeforce = 6671,
	Kraken_Slayer = 6672,
	Immortal_Shieldbow = 6673,
	Navori_Quickblades = 6675,
	The_Collector = 6676,
	Rageknife = 6677,
	Duskblade_of_Draktharr = 6691,
	Eclipse = 6692,
	Prowlers_Claw = 6693,
	Seryldas_Grudge = 6694,
	Serpents_Fang = 6695
};

enum class action_state
{
	CanAttack = 1,
	CanCast = 2,
	CanMove = 4,
	Immovable = 8,
	Unknown = 16,
	IsStealth = 32,
	Taunted = 64,
	Feared = 128,
	Fleeing = 256,
	Surpressed = 512,
	Asleep = 1024,
	NearSight = 2048,
	Ghosted = 4096,
	HasGhost = 8192,
	Charmed = 16384,
	NoRender = 32768,
	DodgePiercing = 131072,
	DisableAmbientGold = 262144,
	DisableAmbientXP = 524288,
	ForceRenderParticles = 65536,
	IsCombatEnchanced = 1048576,
	IsSelectable = 16777216
};
DEFINE_ENUM_FLAG_OPERATORS( action_state );

enum class buff_type: unsigned char
{
	Internal = 0,
	Aura = 1,
	CombatEnchancer = 2,
	CombatDehancer = 3,
	SpellShield = 4,
	Stun = 5,
	Invisibility = 6,
	Silence = 7,
	Taunt = 8,
	Berserk = 9,
	Polymorph = 10,
	Slow = 11,
	Snare = 12,
	Damage = 13,
	Heal = 14,
	Haste = 15,
	SpellImmunity = 16,
	PhysicalImmunity = 17,
	Invulnerability = 18,
	AttackSpeedSlow = 19,
	NearSight = 20,
	Fear = 22,
	Charm = 23,
	Poison = 24,
	Suppression = 25,
	Blind = 26,
	Counter = 27,
	Currency = 21,
	Shred = 28,
	Flee = 29,
	Knockup = 30,
	Knockback = 31,
	Disarm = 32,
	Grounded = 33,
	Drowsy = 34,
	Asleep = 35,
	Obscured = 36,
	ClickproofToEnemies = 37,
	UnKillable = 38
};

enum class champion_id
{
	Aatrox = 266,
	Ahri = 103,
	Akali = 84,
	Alistar = 12,
	Amumu = 32,
	Anivia = 34,
	Annie = 1,
	Aphelios = 523,
	Ashe = 22,
	AurelionSol = 136,
	Azir = 268,
	Bard = 432,
	Blitzcrank = 53,
	Brand = 63,
	Braum = 201,
	Caitlyn = 51,
	Camille = 164,
	Cassiopeia = 69,
	Chogath = 31,
	Corki = 42,
	Darius = 122,
	Diana = 131,
	DrMundo = 36,
	Draven = 119,
	Ekko = 245,
	Elise = 60,
	Evelynn = 28,
	Ezreal = 81,
	FiddleSticks = 9,
	Fiora = 114,
	Fizz = 105,
	Galio = 3,
	Gangplank = 41,
	Garen = 86,
	Gnar = 150,
	Gragas = 79,
	Graves = 104,
	Hecarim = 120,
	Heimerdinger = 74,
	Illaoi = 420,
	Irelia = 39,
	Ivern = 427,
	Janna = 40,
	JarvanIV = 59,
	Jax = 24,
	Jayce = 126,
	Jhin = 202,
	Jinx = 222,
	Kaisa = 145,
	Kalista = 429,
	Karma = 43,
	Karthus = 30,
	Kassadin = 38,
	Katarina = 55,
	Kayle = 10,
	Kayn = 141,
	Kennen = 85,
	Khazix = 121,
	Kindred = 203,
	Kled = 240,
	KogMaw = 96,
	Leblanc = 7,
	LeeSin = 64,
	Leona = 89,
	Lillia = 876,
	Lissandra = 127,
	Lucian = 236,
	Lulu = 117,
	Lux = 99,
	Malphite = 54,
	Malzahar = 90,
	Maokai = 57,
	MasterYi = 11,
	MissFortune = 21,
	Mordekaiser = 82,
	Morgana = 25,
	Nami = 267,
	Nasus = 75,
	Nautilus = 111,
	Neeko = 518,
	Nidalee = 76,
	Nocturne = 56,
	Nunu = 20,
	Olaf = 2,
	Orianna = 61,
	Ornn = 516,
	Pantheon = 80,
	Poppy = 78,
	Pyke = 555,
	Qiyana = 246,
	Quinn = 133,
	Rakan = 497,
	Rammus = 33,
	RekSai = 421,
	Rell = 526,
	Renekton = 58,
	Rengar = 107,
	Riven = 92,
	Rumble = 68,
	Ryze = 13,
	Samira = 360,
	Sejuani = 113,
	Senna = 235,
	Seraphine = 147,
	Sett = 875,
	Shaco = 35,
	Shen = 98,
	Shyvana = 102,
	Singed = 27,
	Sion = 14,
	Sivir = 15,
	Skarner = 72,
	Sona = 37,
	Soraka = 16,
	Swain = 50,
	Sylas = 517,
	Syndra = 134,
	TahmKench = 223,
	Taliyah = 163,
	Talon = 91,
	Taric = 44,
	Teemo = 17,
	Thresh = 412,
	Tristana = 18,
	Trundle = 48,
	Tryndamere = 23,
	TwistedFate = 4,
	Twitch = 29,
	Udyr = 77,
	Urgot = 6,
	Varus = 110,
	Vayne = 67,
	Veigar = 45,
	Velkoz = 161,
	Vi = 254,
	Viego = 234,
	Viktor = 112,
	Vladimir = 8,
	Volibear = 106,
	Warwick = 19,
	MonkeyKing = 62,
	Xayah = 498,
	Xerath = 101,
	XinZhao = 5,
	Yasuo = 157,
	Yone = 777,
	Yorick = 83,
	Yuumi = 350,
	Zac = 154,
	Zed = 238,
	Ziggs = 115,
	Zilean = 26,
	Zoe = 142,
	Zyra = 143,
	Gwen = 887,
	Akshan = 166,
	Vex = 711,
	Zeri = 221,
	Unknown = 5000,
	TFTChampion,
	SG_Skarner,
	SG_VelKoz,
	SG_RekSai,
	SG_KogMaw,
	SG_KhaZix,
	SG_ChoGath,
};

enum class spellslot: int32_t
{
	invalid = -1,
	q = 0,
	w,
	e,
	r,
	summoner1,
	summoner2,
	item_1,
	item_2,
	item_3,
	item_4,
	item_5,
	item_6,
	trinket,
	recall
};

enum spell_state
{
	Ready = 1 << 1,
	NotLearned = 1 << 2,
	NotAvaliable = 1 << 3,
	Surpressed = 1 << 4,
	Cooldown = 1 << 5,
	NotEnoughMana = 1 << 6,
	UnknownState = 1 << 7
};
DEFINE_ENUM_FLAG_OPERATORS( spell_state )


class perk
{
public:
	virtual int32_t get_id( ) = 0;
	virtual std::string get_name( ) = 0;
};

class object_type
{
public:
	virtual uint32_t get_id( ) = 0;
};

enum class spell_targeting: unsigned char
{
	self = 0,
	target = 1,
	area = 2,
	cone = 3,
	self_aoe = 4,
	target_or_location = 5,
	location = 6,
	direction = 7,
	drag_direction = 8,
	invalid = 9,
	area_clamped = 10,
	location_clamped = 11,
	terrain_location = 12,
	terrain_type = 13
};

class spell_data
{
public:
	virtual std::string get_name( ) = 0;
	virtual uint32_t get_name_hash( ) = 0;
	virtual const char* get_name_cstr( ) = 0;
	virtual bool is_insta( ) = 0;
	virtual bool is_no_cooldown( ) = 0;
	virtual spell_targeting get_targeting_type( ) = 0;

	virtual std::uint32_t flags( ) = 0;
	virtual std::uint32_t mAffectsTypeFlags( ) = 0;
	virtual std::uint32_t mAffectsStatusFlags( ) = 0;
	virtual const char* mAlternateName( ) = 0;
	virtual float mCoefficient( ) = 0;
	virtual float mCoefficient2( ) = 0;
	virtual const char* mAnimationName( ) = 0;
	virtual const char* mAnimationLoopName( ) = 0;
	virtual const char* mAnimationWinddownName( ) = 0;
	virtual const char* mAnimationLeadOutName( ) = 0;
	virtual const char* mMinimapIconName( ) = 0;
	virtual const char* mKeywordWhenAcquired( ) = 0;
	virtual float mCastTime( ) = 0;
	virtual float* mChannelDuration( ) = 0;
	virtual float* CooldownTime( ) = 0;
	virtual float DelayCastOffsetPercent( ) = 0;
	virtual float DelayTotalTimePercent( ) = 0;
	virtual float mPreCastLockoutDeltaTime( ) = 0;
	virtual float mPostCastLockoutDeltaTime( ) = 0;
	virtual bool mIsDelayedByCastLocked( ) = 0;
	virtual float mStartCooldown( ) = 0;
	virtual float* mCastRangeGrowthMax( ) = 0;
	virtual float* mCastRangeGrowthStartTime( ) = 0;
	virtual float* mCastRangeGrowthDuration( ) = 0;
	virtual float mChargeUpdateInterval( ) = 0;
	virtual float mCancelChargeOnRecastTime( ) = 0;
	virtual std::int32_t* mMaxAmmo( ) = 0;
	virtual std::int32_t* mAmmoUsed( ) = 0;
	virtual float* mAmmoRechargeTime( ) = 0;
	virtual bool mAmmoNotAffectedByCDR( ) = 0;
	virtual bool mCooldownNotAffectedByCDR( ) = 0;
	virtual bool mAmmoCountHiddenInUI( ) = 0;
	virtual bool mCostAlwaysShownInUI( ) = 0;
	virtual bool cannotBeSuppressed( ) = 0;
	virtual bool canCastWhileDisabled( ) = 0;
	virtual bool mCanTriggerChargeSpellWhileDisabled( ) = 0;
	virtual bool canCastOrQueueWhileCasting( ) = 0;
	virtual bool canOnlyCastWhileDisabled( ) = 0;
	virtual bool mCantCancelWhileWindingUp( ) = 0;
	virtual bool mCantCancelWhileChanneling( ) = 0;
	virtual bool cantCastWhileRooted( ) = 0;
	virtual bool mChannelIsInterruptedByDisables( ) = 0;
	virtual bool mChannelIsInterruptedByAttacking( ) = 0;
	virtual bool mApplyAttackDamage( ) = 0;
	virtual bool mApplyAttackEffect( ) = 0;
	virtual bool mApplyMaterialOnHitSound( ) = 0;
	virtual bool mDoesntBreakChannels( ) = 0;
	virtual bool mBelongsToAvatar( ) = 0;
	virtual bool mIsDisabledWhileDead( ) = 0;
	virtual bool canOnlyCastWhileDead( ) = 0;
	virtual bool mCursorChangesInGrass( ) = 0;
	virtual bool mCursorChangesInTerrain( ) = 0;
	virtual bool mProjectTargetToCastRange( ) = 0;
	virtual bool mSpellRevealsChampion( ) = 0;
	virtual bool mUseMinimapTargeting( ) = 0;
	virtual bool CastRangeUseBoundingBoxes( ) = 0;
	virtual bool mMinimapIconRotation( ) = 0;
	virtual bool mUseChargeChanneling( ) = 0;
	virtual bool mCanMoveWhileChanneling( ) = 0;
	virtual bool mDisableCastBar( ) = 0;
	virtual bool mShowChannelBar( ) = 0;
	virtual bool AlwaysSnapFacing( ) = 0;
	virtual bool UseAnimatorFramerate( ) = 0;
	virtual bool bHaveHitEffect( ) = 0;
	virtual bool bIsToggleSpell( ) = 0;
	virtual bool mDoNotNeedToFaceTarget( ) = 0;
	virtual bool mNoWinddownIfCancelled( ) = 0;
	virtual bool mIgnoreRangeCheck( ) = 0;
	virtual bool mOrientRadiusTextureFromPlayer( ) = 0;
	virtual bool mIgnoreAnimContinueUntilCastFrame( ) = 0;
	virtual bool mHideRangeIndicatorWhenCasting( ) = 0;
	virtual bool mUpdateRotationWhenCasting( ) = 0;
	virtual bool mPingableWhileDisabled( ) = 0;
	virtual bool mConsideredAsAutoAttack( ) = 0;
	virtual bool mDoesNotConsumeMana( ) = 0;
	virtual bool mDoesNotConsumeCooldown( ) = 0;
	virtual bool mLockedSpellOriginationCastID( ) = 0;
	virtual float* CastRange( ) = 0;
	virtual float* CastRangeDisplayOverride( ) = 0;
	virtual float* CastRadius( ) = 0;
	virtual float* CastRadiusSecondary( ) = 0;
	virtual float CastConeAngle( ) = 0;
	virtual float CastConeDistance( ) = 0;
	virtual float CastTargetAdditionalUnitsRadius( ) = 0;
	virtual float LuaOnMissileUpdateDistanceInterval( ) = 0;
	virtual std::uint32_t mCastType( ) = 0;
	virtual float CastFrame( ) = 0;
	virtual float MissileSpeed( ) = 0;
	virtual const char* mMissileEffectName( ) = 0;
	virtual const char* mMissileEffectPlayerName( ) = 0;
	virtual const char* mMissileEffectEnemyName( ) = 0;
	virtual float mLineWidth( ) = 0;
	virtual float mLineDragLength( ) = 0;
	virtual std::uint32_t mLookAtPolicy( ) = 0;
	virtual std::uint32_t mHitEffectOrientType( ) = 0;
	virtual const char* mHitEffectName( ) = 0;
	virtual const char* mHitEffectPlayerName( ) = 0;
	virtual const char* mAfterEffectName( ) = 0;
	virtual bool bHaveHitBone( ) = 0;
	virtual const char* mHitBoneName( ) = 0;
	virtual std::int32_t* mFloatVarsDecimals( ) = 0;
	virtual float* Mana( ) = 0;
	virtual float* ManaUiOverride( ) = 0;
	virtual std::uint32_t SelectionPriority( ) = 0;
	virtual const char* mVOEventCategory( ) = 0;
	virtual float mSpellCooldownOrSealedQueueThreshold( ) = 0;
};

class spell_data_inst
{
public:
	virtual bool is_learned( ) = 0;
	virtual int32_t level( ) = 0;
	virtual int16_t toogle_state( ) = 0;
	virtual float cast_end_time( ) = 0;
	virtual float cooldown( ) = 0;
	virtual float cooldown_start( ) = 0;
	virtual float cooldown_end( ) = 0;
	virtual int32_t ammo( ) = 0;
	virtual int32_t get_max_ammo_spell( ) = 0;

	virtual std::string get_name( ) = 0;
	virtual uint32_t get_name_hash( ) = 0;

	virtual float get_missile_speed( ) = 0;

	virtual spell_data_script get_spell_data( ) = 0;

	virtual float cooldownex_end( ) = 0;

	virtual uint32_t* get_icon_texture( ) = 0;

	virtual float get_total_cooldown( ) = 0;

	virtual int8_t get_icon_index( ) = 0;
};

class game_object;
class buff_instance
{
public:
	virtual uint32_t get_hash_name( ) = 0;
	virtual int32_t get_count( ) = 0;
	virtual buff_type get_type( ) = 0;
	virtual std::string get_name( ) = 0;
	virtual bool is_valid( ) = 0;
	virtual bool is_alive( ) = 0;
	virtual float get_start( ) = 0;
	virtual float get_end( ) = 0;
	virtual float get_remaining_time( ) = 0;
	virtual game_object_script get_caster( ) = 0;
	virtual const char* get_name_cstr( ) = 0;
};

class item
{
public:
	virtual char get_item_count( ) = 0;
	virtual int32_t get_item_id( ) = 0;
};

class path_controller
{
public:
	virtual bool is_moving( ) = 0;
	virtual bool is_dashing( ) = 0;
	virtual float get_dash_speed( ) = 0;
	virtual uint32_t get_path_count( ) = 0;
	virtual uint32_t get_current_path_node( ) = 0;
	virtual vector get_start_vec( ) = 0;
	virtual vector get_end_vec( ) = 0;

	virtual std::vector<vector> get_path( ) = 0;

	virtual vector get_position_on_path( ) = 0;

	virtual vector get_velocity( ) = 0;
};

class spell_instance
{
public:
	virtual unsigned short get_last_target_id( ) = 0;

	virtual float do_cast_time( ) = 0;
	virtual float cast_start_time( ) = 0;
	virtual float get_attack_delay( ) = 0;
	virtual float get_attack_cast_delay( ) = 0;
	virtual float get_time( ) = 0;

	virtual bool is_winding_up( ) = 0;
	virtual bool is_auto_attack( ) = 0;
	virtual bool spell_has_been_casted( ) = 0;
	virtual bool is_auto_attacking( ) = 0;
	virtual bool is_charging( ) = 0;
	virtual bool is_channeling( ) = 0;
	virtual bool is_stopped( ) = 0;

	virtual vector get_start_position( ) = 0;
	virtual vector get_end_position( ) = 0;

	virtual spell_data_script get_spell_data( ) = 0;

	virtual spellslot get_spellslot( ) = 0;
	virtual std::int32_t get_level( ) = 0;
};

class character_data
{
public:
	virtual std::string get_base_skin_name( ) = 0;
	virtual const char* get_base_skin_name_cstr( ) = 0;
};

enum _issue_order_type
{
	HoldPosition = 1,
	MoveTo = 2,
	AttackUnit = 3,
	AutoAttackPet = 4,
	AutoAttack = 5,
	MovePet = 6,
	AttackTo = 7,
	Stop = 10
};

enum class _player_ping_type
{
	normal = 0,
	on_my_way,
	missing_enemy,
	danger,
	assist_me,
	area_is_warded,
	careful
};

enum class float_hero_stat
{
	MINIONS_KILLED,
	NEUTRAL_MINIONS_KILLED,
	NEUTRAL_MINIONS_KILLED_YOUR_JUNGLE,
	NEUTRAL_MINIONS_KILLED_ENEMY_JUNGLE
};

enum class int_hero_stat
{
	CHAMPIONS_KILLED,
	NUM_DEATHS,
	ASSISTS,
	LARGEST_KILLING_SPREE,
	KILLING_SPREES,
	LARGEST_MULTI_KILL,
	BOUNTY_LEVEL,
	DOUBLE_KILLS,
	TRIPLE_KILLS,
	QUADRA_KILLS,
	PENTA_KILLS,
	UNREAL_KILLS,
	BARRACKS_KILLED,
	BARRACKS_TAKEDOWNS,
	TURRETS_KILLED,
	TURRET_TAKEDOWNS,
	HQ_KILLED,
	HQ_TAKEDOWNS,
	OBJECTIVES_STOLEN,
	OBJECTIVES_STOLEN_ASSISTS,
	FRIENDLY_DAMPEN_LOST,
	FRIENDLY_TURRET_LOST,
	FRIENDLY_HQ_LOST,
	BARON_KILLS,
	DRAGON_KILLS,
	NODE_CAPTURE,
	NODE_CAPTURE_ASSIST,
	NODE_NEUTRALIZE,
	NODE_NEUTRALIZE_ASSIST,
	TEAM_OBJECTIVE
};

class game_object
{
public:
	virtual void update( ) = 0;
	
	//Set's the object skin
	//	- id of the skin
	//	- desired model name
	//
	virtual void set_skin( uint32_t id, const std::string& model ) = 0;

	//Buys an item.
	//	- item id to buy
	//
	virtual void buy_item( ItemId itm ) = 0;

#ifdef INTERNAL_CORE
	//Returns a vector of object perks
	//
	virtual std::vector<std::shared_ptr<perk>> get_perks( ) = 0;
#else
	//Returns a vector of object perks
	//
	virtual std::vector<std::unique_ptr<perk>> const& get_perks( ) = 0;
#endif

	//Returns base model.  For champions it will be the champion's name.
	//
	virtual std::string get_model( ) = 0;
	
	//Returns the base skin name of the object.
	//
	virtual std::string get_base_skin_name( ) = 0;
	
	//Returns name of the object. For champions it will be the Player's name.
	//
	virtual std::string get_name( ) = 0;

	virtual uint32_t get_base( ) = 0;

	//Returns the object team. 
	//
	virtual game_object_team get_team( ) = 0;
	
	//Returns instance of object_type_script class containing information about object type
	//
	virtual object_type_script get_type( ) = 0;
	
	//Returns auto attack spell data of the object.
	//
	virtual spell_data_script get_auto_attack( ) = 0;

	virtual action_state get_action_state( ) = 0;

	//Returns the Id of the object in the EntityList.
	//
	virtual uint16_t get_id( ) = 0;
	virtual uint32_t get_handle( ) = 0;
	virtual uint32_t get_object_owner( ) = 0;
	
	//Returns the owner of the object otherwise nullptr. Calling this function on Tibbers will return Annie (the owner of Tibbers)
	// - Please remember owner is not yet set to an object in on_create event.
	//   Game sets the owner in later update ticks
	//
	virtual game_object_script get_owner( ) = 0;

	virtual int32_t get_type_flags( ) = 0;

#ifdef INTERNAL_CORE
	//Returns a vector of object buffs
	//
	virtual std::vector<std::shared_ptr<buff_instance>> get_bufflist( ) = 0;
#else
	//Returns a vector of object buffs
	//
	virtual std::vector<std::unique_ptr<buff_instance>> const& get_bufflist( ) = 0;
#endif
	virtual spell_data_inst_script get_spell( spellslot spell ) = 0;

	//Returns instance of item_script for a given spellslot otherwise nullptr.
	//	- spellslot of an item
	//
	virtual item_script get_item( spellslot itemslot ) = 0;

	virtual path_controller_script get_path_controller( ) = 0;

	//Returns true if object is a nexus
	//
	virtual bool is_nexus( ) = 0;
	
	//Returns true if object is an inhibitor
	//
	virtual bool is_inhibitor( ) = 0;
	
	//Checks whether the object is winding up. Meaning if the object is doing an action like. casting spell or autoattack.
	//
	virtual bool is_winding_up( ) = 0;
	
	//Returns mana needed to use a spell 
	//	- spellslot of the spell
	virtual float get_mana_for_spell( spellslot slot ) = 0;

	//Returns path starting from object position to it's destination.  
	//
	std::vector<vector> get_real_path( );

	//Returns auto attack damage of the object on the target.
	//	- target
	//	- include passive damage in calculations (eg. on-hit damages)
	//
	virtual float get_auto_attack_damage( game_object_script to, bool respect_passives = true ) = 0;

	//Checks if the object is recalling to base
	//
	bool is_recalling( );
	
	//Checks if the object is the local player.
	//
	virtual bool is_me( ) = 0;
	
	//Checks if the object is visible. Meaning if it's not in FOW
	//
	virtual bool is_visible( ) = 0;
	
	//Checks if the target is in auto attack range of the object.
	//	- target
	//	- additional range 
	//
	bool is_in_auto_attack_range( game_object_script to, float additional = 0.f );
	bool is_in_auto_attack_range_native( game_object* to, float additional = 0.f );
	
	//Checks whether the object is under enemy turret (enemy towards the object)
	//
	bool is_under_enemy_turret( );
	
	//Checks if the object can issue an attack command.
	//Example: if the object is stunned function will return false
	//
	virtual bool can_attack( ) = 0;
	
	//Checks if the object can issue a move command.
	//Example: if the object is stunned function will return false
	//
	virtual bool can_move( ) = 0;
	
	//Checks if the object can cast a spell.
	//Example: if the object is stunned function will return false
	//
	virtual bool can_cast( ) = 0;
	
	//Returns true if object is a monster (mobs in jungle)
	//
	virtual bool is_monster( ) = 0;
	
	//Returns true if object is a minion
	//
	virtual bool is_minion( ) = 0;
	
	//Checks if the object is stealthed.
	//
	virtual bool is_stealthed( ) = 0;
	
	//Checks if the object is immovable.
	//
	virtual bool is_immovable( ) = 0;
	
	//Checks if the object is ghosted (has move speed buff).
	//
	virtual bool is_ghosted( ) = 0;
	
	//Returns true if object is a jungle buff (blue buff, red buff)
	//
	virtual bool is_jungle_buff( ) = 0;
	
	//Returns true if object is an epic monster (baron, herald, dragon)
	//
	virtual bool is_epic_monster( ) = 0;
	
	//Returns true if object is a lane minion
	//
	virtual bool is_lane_minion( ) = 0;
	
	//Checks if the object is dead.
	//
	virtual bool is_dead( ) = 0;
	
	//Checks if the object is melee. 
	//
	virtual bool is_melee( ) = 0;
	
	//Checks if the object is ranged.
	//
	virtual bool is_ranged( ) = 0;
	
	//Checks if the object is an ally towards local player.
	//
	virtual bool is_ally( ) = 0;
	
	//Checks if the object is enemy towards local player.
	//
	virtual bool is_enemy( ) = 0;
	
	//Returns true if object is a plant
	//
	virtual bool is_plant( ) = 0;
	
	//Returns true if object IsValid and also if it is targetable for autoattacks
	//	float range - maximum range from Player to entity_list
	//	vector from - if set to zero range will be checked from Player position otherwise from the vector you pass
	//	bool ignore_invulnerability - skips the check for is_invurnelable (Kayle R, Zhonyas etc)
	//
	bool is_valid_target( float range = FLT_MAX, vector const& from = vector( 0, 0 ), bool ignore_invulnerability = false );
	
	//Checks whether an object has a perk of given id
	//	- id of the perk
	//
	virtual bool has_perk( uint32_t id ) = 0;
	
	//Returns a buff with a given hash name otherwise nullptr.
	//	- hashed buff name
	//
	virtual buff_instance_script get_buff( uint32_t hash ) = 0;
	
	//Returns a buff with a given type otherwise nullptr.
	//	- type of the buff
	//
	virtual buff_instance_script get_buff_by_type( buff_type type ) = 0;
	
	//Check whether the object has buff of the hashed name
	//	- hashed buff name
	//
	virtual bool has_buff( uint32_t hash ) = 0;
	
	//Check whether the object has any buff of the hashed names
	//	- vector of hashed buff names
	//
	virtual bool has_buff( const std::vector<uint32_t>& hashes ) = 0;
	
	//Returns spellslot of an item for a given ItemId otherwise spellslot::invalid.
	//	- ItemId of an item
	//
	virtual spellslot has_item( int32_t itemid ) = 0;
	
	//Returns spellslot of given ItemIds otherwise spellslot::invalid.
	//	- vector of ItemIds
	//
	virtual spellslot has_item( const std::vector<ItemId>& itemid ) = 0;
	virtual int32_t get_minion_type( ) = 0;
	
	//Returns buff count of a buff with a given hash name otherwise 0.
	//	- hashed buff name
	//
	virtual int get_buff_count( uint32_t hash ) = 0;
	
	//Returns true if object is a champion
	//
	virtual bool is_ai_hero( ) = 0;
	
	//Returns true for objects that inherit from AIBase. Ex. AIHero, AIMinion, AITurret
	//
	virtual bool is_ai_base( ) = 0;
	
	//Returns true if object is a minion (any type, jungle monsters, lane minions, wards, etc)
	//
	virtual bool is_ai_minion( ) = 0;
	
	//Returns true if object is a turret
	//
	virtual bool is_ai_turret( ) = 0;

	//Checks if Local Player can issue an attack command on the object
	//	- Skip invulnerability buff check
	//
	virtual bool is_attack_allowed_on_target( bool ignore_invulnerability = false ) = 0;

	//Returns spell_state flags of the spell.
	//	- spellslot of the spell to check
	//
	//	Available states:
	//		spell_state::Ready,
	//		spell_state::NotLearned ,
	//		spell_state::NotAvaliable,
	//		spell_state::Surpressed,
	//		spell_state::Cooldown,
	//		spell_state::NotEnoughMana,
	//		spell_state::UnknownState
	//
	//
	virtual spell_state get_spell_state( spellslot spell ) = 0;

	virtual bool obj_is_attackable( ) = 0;
	
	//Checks if object is targetable to team.
	//	- team to check
	//
	virtual bool is_targetable_to_team( game_object_team team ) = 0;
	
	//Checks if object is targetable.
	//
	virtual bool is_targetable( ) = 0;
	virtual bool can_be_attacked_by( game_object_script obj ) = 0;

	virtual unsigned short missile_get_sender_id( ) = 0;
	virtual unsigned short missile_get_target_id( ) = 0;

	virtual bool missile_is_targeted( ) = 0;

	virtual vector missile_get_start_position( ) = 0;
	virtual vector missile_get_end_position( ) = 0;

	//Returns ChampionId. Object must be of type AIHeroClient.
	//
	virtual champion_id get_champion( ) = 0;

	//Returns 2 dimensional distance from object to object.
	//	- object to check distance
	//
	float get_distance( game_object_script to );

	virtual float get_attack_range( ) = 0;
	virtual float get_percent_physical_damage_mod( ) = 0;
	virtual float get_flat_physical_damage_mod( ) = 0;
	virtual float get_flat_magic_damage_mod( ) = 0;
	virtual float get_base_attack_damage( ) = 0;
	virtual float get_base_ability_power( ) = 0;
	virtual float get_total_attack_damage( ) = 0;
	virtual float get_move_speed( ) = 0;
	virtual float get_armor( ) = 0;
	virtual float get_percent_magic_damage_mod( ) = 0;

	virtual float get_percent_bonus_armor_penetration( ) = 0;
	virtual float get_spell_block( ) = 0;
	virtual float get_bonus_armor( ) = 0;
	virtual float get_crit( ) = 0;
	virtual float get_magic_lathality( ) = 0;
	virtual float get_percent_magic_penetration( ) = 0;
	virtual float get_flat_magic_reduction( ) = 0;
	virtual float get_percent_magic_reduction( ) = 0;
	virtual float get_flat_armor_penetration( ) = 0;
	virtual float get_physical_lathality( ) = 0;
	virtual float get_flat_magic_penetration( ) = 0;
	virtual float get_percent_armor_penetration( ) = 0;

	virtual float get_attack_delay( ) = 0;
	virtual float get_additional_attack_damage( ) = 0;
	virtual float get_flat_damage_reduction_from_barracks_minion_mod( ) = 0;
	virtual float get_percent_damage_to_barracks_minion_mod( ) = 0;
	virtual float get_total_ability_power( ) = 0;
	virtual float get_attack_cast_delay( ) = 0;
	virtual float get_bounding_radius( ) = 0;
	virtual float get_health( ) = 0;
	virtual float get_mana( ) = 0;
	virtual float get_mana_percent( ) = 0;
	virtual float get_max_mana( ) = 0;
	virtual float get_max_health( ) = 0;
	virtual float get_health_percent( ) = 0;
	virtual float get_gold( ) = 0;
	virtual int get_level( ) = 0;

	//Returns the object position in the game.
	//
	virtual vector get_position( ) = 0;

	//Returns the spell_instance_script instance of current casting spell otherwise nullptr.
	//	- old is no longer used by the core. The value of this parameter doesn't matter
	//
	virtual spell_instance_script get_active_spell( bool old = false ) = 0;

	virtual character_data_script get_character_data( ) = 0;

	//Returns the NetworkId of the object.
	//
	virtual uint32_t get_network_id( ) = 0;
	
	//Returns the numer of allies around the object (allies towards the object)
	//	- maximum range from object to allies
	//
	int32_t count_allies_in_range( float range );
	
	//Returns the numer of enemies around the object (enemies towards the object)
	//	- maximum range from object to enemies
	//
	int32_t count_enemies_in_range( float range );

	//Issues an attack command on target
	//	- target of the order
	//	- should event be triggered?
	//	- enable or disable core humanizer
	//
	virtual void issue_order( game_object_script target, bool trigger_event = true, bool use_humanizer = true ) = 0;
	
	//Issues a move command
	//	- position of the order
	//	- should event be triggered?
	//	- set the order to attack_move
	//
	virtual void issue_order( vector game_pos, bool trigger_event = true, bool use_humanizer = true, bool attack_move = false ) = 0;
	
	//Issues an order on the player
	//	- _issue_order_type type of the order
	//	- should event be triggered?
	//	- enable or disable core humanizer
	//
	virtual void issue_order( _issue_order_type type, bool trigger_event = true, bool use_humanizer = true ) = 0;

	//Returns the perpendicular direction of the object.
	//
	virtual vector get_direction_perpendicular( ) = 0;
	
	//Returns the direction of the object.
	//
	virtual vector get_direction( ) = 0;
	
	//Returns the direction on the waypoint of the object.
	//
	virtual vector get_pathing_direction( ) = 0;

	//Checks whether the object is facing object.
	//	- the object to check
	//
	virtual bool is_facing( game_object_script obj ) = 0;
	
	//Checks whether the object is facing a position.
	//	- the position to check
	//
	virtual bool is_facing( const vector& position ) = 0;
	
	//Casts a spell
	//	- slot of the spell to be casted
	//	- should event be triggered?
	//	- is charded spell
	//
	virtual void cast_spell( spellslot slot, bool trigger_event = true, bool is_charging = false ) = 0;
	
	//Casts a spell
	//	- slot of the spell to be casted
	//	- position to where spell needs to be casted
	//	- should event be triggered?
	//	- is charded spell
	//
	virtual void cast_spell( spellslot slot, const vector& pos, bool trigger_event = true, bool is_charging = false ) = 0;
	
	//Casts a spell
	//	- slot of the spell to be casted
	//	- target of the spell
	//	- should event be triggered?
	//	- is charded spell
	//
	virtual void cast_spell( spellslot slot, game_object_script obj, bool trigger_event = true, bool is_charging = false ) = 0;

	//Checks whether the object is immune to physical damage.
	//
	virtual bool is_physical_immune( ) = 0;
	
	//Checks whether the object is immune to magical damage.
	//
	virtual bool is_magic_immune( ) = 0;
	
	//Checks whether the object is immune to lifesteal. 
	//Meaning if dealing damage will return back % health of the damage dealt (if you have lifesteal).
	//
	virtual bool is_lifesteal_immune( ) = 0;
	
	//Checks whether the object is immune to any source of damage. (Kayle R, zhonyas, etc.)
	//
	virtual bool is_invulnerable( ) = 0;

	//Returns the value of importance of the current casting spell
	//Possible return values:
	//	0 - object is not casting an important spell
	//	1 - object is casting important spell of medium danger value (Fiddlesticks W, MasterYi W)
	//	2 - object is casting important spell of high danger value (Katarina R, Caitlyn R)
	int32_t is_casting_interruptible_spell( );

	virtual spell_data_script get_missile_sdata( ) = 0;

	//Returns a buff with any of given types otherwise nullptr.
	//	- vector of buff types
	//
	virtual buff_instance_script get_buff_by_type( std::vector<buff_type> type ) = 0;

	//Checks whether the object is using a teleport.
	//
	virtual bool is_teleporting( ) = 0;
	
	//Returns the teleport state.
	//Possible return values: "Recall", "SuperRecall", "SummonerTeleport", "Gate", "shenrchannelmanagerm", "shenrtargettracker", "YuumiW"
	//
	virtual std::string get_teleport_state( ) = 0;

	//Returns the texture of the object avatar. Return value may be nullptr
	//
	virtual uint32_t* get_square_icon_portrait( ) = 0;

	//Returns the screen position of the object HPBar
	//
	virtual vector get_hpbar_pos( ) = 0;

	//Returns the amount of stacks under the unit HPBar
	//Example: Annie stacks, Zeri passive stacks...
	//
	virtual float get_hp_bar_stacks( ) = 0;

	//Returns the amount of spell training points.
	//
	virtual int get_lvlup_stacks( ) = 0;

	//Checks whether the object is visible on the screen.
	//
	virtual bool is_visible_on_screen( ) = 0;

	virtual float get_hp_regen_rate( ) = 0;

	//Returns the texture of the object passive icon. Return value may be nullptr
	//
	virtual uint32_t* get_passive_icon_texture( ) = 0;

	virtual float get_passive_cooldown_end_time( ) = 0;
	virtual float get_passive_cooldown_total_time( ) = 0;

	//Returns true if object is a ward
	//
	virtual bool is_ward( ) = 0;

	//Checks whether the object is zombie
	//Example: Sion passive, Karthus passive
	//
	virtual bool is_zombie( ) = 0;

	//Returns true if object is a missile
	//
	virtual bool is_missile( ) = 0;

	//Returns true if object is a particle
	//
	virtual bool is_general_particle_emitter( ) = 0;

	virtual float get_physical_shield( ) = 0;
	virtual float get_magical_shield( ) = 0;
	virtual float get_all_shield( ) = 0;

	virtual float msar_max( ) = 0;

	virtual float get_bonus_spell_block( ) = 0;

	//Returns health with shields
	//	- include physical_shield into calculations
	//	- include magical_shield into calculations
	//
	float get_real_health( bool physical_shield = false, bool magical_shield = false );

	virtual float get_exp( ) = 0;
	float get_exp_percent( );

	//Checks whether the object is under ally turret (ally towards the object)
	//
	bool is_under_ally_turret( );
	
	//Returns 2 dimensional distance from object to position.
	//	- position in the game
	//
	float get_distance( vector const& to );
	
	//Returns time left of a buff with a given hash name otherwise 0.
	//	- hashed buff name
	//
	float get_buff_time_left( uint32_t hash );
	
	//Levels a spell on a given spellslot
	//	- spellslot of the spell
	//
	virtual void levelup_spell( spellslot slot ) = 0;

	//Returns the object BoundingBox min
	//
	virtual vector get_bbox_min( ) = 0;
	
	//Returns the object BoundingBox max
	//
	virtual vector get_bbox_max( ) = 0;

	virtual void set_position( const vector& pos ) = 0;

	virtual const char* get_name_cstr( ) = 0;

	virtual const char* get_model_cstr( ) = 0;
	
	//Casts a spell
	//	- slot of the spell to be casted
	//	- start position of the spell
	//	- end position of the spell
	//	- should event be triggered?
	//	- is charded spell
	//
	virtual void cast_spell( spellslot slot, const vector& startPos, const vector& endPos, bool trigger_event = true, bool is_charging = false ) = 0;
	
	//Updates a charged spell
	//	- slot of the spell to be updated
	//	- position of the spell to be updated
	//	- release cast
	//	- should event be triggered?
	//
	virtual void update_charged_spell( spellslot slot, const vector& position, bool releaseCast, bool trigger_event = true ) = 0;
	
	//Checks whether the object has had it's HPBar recently rendered
	//
	virtual bool is_hpbar_recently_rendered( ) = 0;
	virtual float get_percent_cooldown_mod( ) = 0;

	//Returns the particle rotation right vector.
	//
	virtual vector get_particle_rotation_right( ) = 0;
	
	//Returns the particle rotation up vector.
	//
	virtual vector get_particle_rotation_up( ) = 0;
	
	//Returns the particle rotation forward vector.
	//
	virtual vector get_particle_rotation_forward( ) = 0;

	//Casts a ping.
	//	- position of the ping
	//	- object to be pinged (can be nullptr)
	//	- type of the ping to send
	//
	virtual void cast_ping( const vector& position, game_object_script object, _player_ping_type ping_type ) = 0;
		
	//Calculates the path starting from object position to the end
	//	- position where waypoint ends
	//
	virtual std::vector<vector> get_path( const vector& end ) = 0;

	virtual float get_ability_haste_mod( ) = 0;

	//Returns the floating hero stat
	//	- id of the float_hero_stat
	//
	virtual float get_hero_stat( float_hero_stat id ) = 0;
	
	//Returns the int hero stat
	//	- id of the int_hero_stat
	//
	virtual int get_hero_stat( int_hero_stat id ) = 0;

	virtual float mDodge( ) = 0;
	virtual float mPARRegenRate( ) = 0;
	virtual float mAttackSpeedMod( ) = 0;
	virtual float mFlatCastRangeMod( ) = 0;
	virtual float mPercentLifeStealMod( ) = 0;
	virtual float mPercentSpellVampMod( ) = 0;
	virtual float mPercentPhysicalVamp( ) = 0;
	virtual float mPercentCritBonusArmorPenetration( ) = 0;
	virtual float mPercentCritTotalArmorPenetration( ) = 0;
	virtual float mPercentBonusMagicPenetration( ) = 0;
	virtual float mBaseHPRegenRate( ) = 0;
	virtual float mPrimaryARBaseRegenRateRep( ) = 0;
	virtual float mSecondaryARRegenRateRep( ) = 0;
	virtual float mSecondaryARBaseRegenRateRep( ) = 0;
	virtual float mPercentCooldownCapMod( ) = 0;
	virtual float mPercentCCReduction( ) = 0;
	virtual float mPercentEXPBonus( ) = 0;
	virtual float mFlatBaseAttackDamageMod( ) = 0;
	virtual float mPercentBaseAttackDamageMod( ) = 0;
	virtual float mBaseAttackDamageSansPercentScale( ) = 0;
	virtual float mFlatBaseHPPoolMod( ) = 0;
	virtual float mPercentBonusPhysicalDamageMod( ) = 0;
	virtual float mPercentBasePhysicalDamageAsFlatBonusMod( ) = 0;
	virtual float mPercentAttackSpeedMod( ) = 0;
	virtual float mPercentMultiplicativeAttackSpeedMod( ) = 0;
	virtual float mCritDamageMultiplier( ) = 0;
	virtual float mPercentOmnivampMod( ) = 0;
	virtual float mFlatBubbleRadiusMod( ) = 0;
	virtual float mPercentBubbleRadiusMod( ) = 0;
	virtual float mMoveSpeedBaseIncrease( ) = 0;
	virtual float mScaleSkinCoef( ) = 0;

	//Send's a message in the chat.
	//
	virtual void send_chat( const char* format, ... ) = 0;
	virtual game_object_script get_emitter( ) = 0;
	virtual std::uint32_t get_emitter_resources_hash( ) = 0;

	bool is_valid( bool force = false );
	
	//Returns the immovibility time left of the object
	//
	float get_immovibility_time( );

	//Checks whether the object is moving.
	//
	bool is_moving( );
	
	//Checks whether the object is dashing.
	//
	bool is_dashing( );

	//Check whether the object has buff of type
	//	- type of the buff
	//
	bool has_buff_type( buff_type type );
	
	//Check whether the object has any of the buffs of type you pass
	//	- vector of buff types
	//
	bool has_buff_type( std::vector<buff_type> const& type );

	//Checks whether the item of given id is owned by the object and ready to use.
	//	- ItemId of an item
	//
	bool is_item_ready( int32_t itemid );
	
	//Checks whether the item of given id is owned by the object and ready to use.
	//	- ItemId of an item
	//
	bool is_item_ready( ItemId itemid );
	
	//Returns spellslot of given ItemIds otherwise spellslot::invalid.
	//	- vector of ItemIds
	//
	spellslot has_item( ItemId itemid );
};

class r3d_renderer
{
public:
	virtual void* view_matrix( ) = 0;
	virtual void* projection_matrix( ) = 0;
	virtual uint32_t screen_width( ) = 0;
	virtual uint32_t screen_height( ) = 0;
	virtual bool is_on_screen( const vector& screen, int tolerance ) = 0;
	virtual void world_to_screen( const vector& world, vector& screen ) = 0;
};

class hud_select_logic
{
public:
	virtual void set_should_target_champions_only( bool ) = 0;
	virtual bool get_should_target_champions_only( ) = 0;
	virtual game_object_script get_selected_object( ) = 0;
};

class hud_spell_logic
{
public:
	virtual bool has_charged_spell( ) = 0;
};

class hud_input_logic
{
public:
	virtual vector get_game_cursor_position( ) = 0;
};

class hud_manager
{
public:
	virtual hud_select_logic* get_hud_select_logic( ) = 0;
	virtual hud_spell_logic* get_hud_spell_logic( ) = 0;
	virtual hud_input_logic* get_hud_input_logic( ) = 0;
};

class tactical_map
{
public:
	virtual bool to_map_coord( vector& in, vector& out ) = 0;
};

class menu_gui
{
public:
	virtual bool gui_is_open( ) = 0;
	virtual tactical_map* get_tactical_map( ) = 0;
};

class game_ping
{
public:
	virtual float get_ping( ) = 0;
};

class input
{
public:
	virtual point2 get_cursor_pos( ) = 0;
	virtual point2 get_game_cursor_pos( ) = 0;
};

class game_event_manager
{
public:
	virtual uint32_t get_active_handler_index( ) = 0;
	virtual void call_event( uint32_t type, const char* event_name, point2 mouse, vector cast = vector( 0, 0, 0 ) ) = 0;
};

enum class nav_collision_flags
{
	none = 0,
	grass = 1,
	wall = 2,
	building = 64,
	prop = 128,
	global_vision = 256
};
DEFINE_ENUM_FLAG_OPERATORS( nav_collision_flags )

class nav_mesh
{
public:
	virtual nav_collision_flags get_collision_flag( const vector& pos ) = 0;
	virtual bool is_in_fow( const vector& pos ) = 0;
	virtual float get_height_for_position( float x, float y ) = 0;
};

enum class keyboard_game
{
	escape = 1,
	num_1 = 2,
	num_2 = 3,
	num_3 = 4,
	num_4 = 5,
	num_5 = 6,
	num_6 = 7,
	num_7 = 8,
	num_8 = 9,
	num_9 = 10,
	num_0 = 11,
	minus = 12,
	equals = 13,
	backspace = 14,
	tab = 15,
	q = 16,
	w = 17,
	e = 18,
	r = 19,
	t = 20,
	y = 21,
	u = 22,
	i = 23,
	o = 24,
	p = 25,
	lbracket = 26,
	rbracket = 27,
	enter = 28,
	lctrl = 29,
	a = 30,
	s = 31,
	d = 32,
	f = 33,
	g = 34,
	h = 35,
	j = 36,
	k = 37,
	l = 38,
	semicolon = 39,
	apostrophe = 40,
	tilde = 41,
	lshift = 42,
	backslash = 43,
	z = 44,
	x = 45,
	c = 46,
	v = 47,
	b = 48,
	n = 49,
	m = 50,
	comma = 51,
	period = 52,
	slash = 53,
	rshift = 54,
	np_multiply = 55,
	lalt = 56,
	space = 57,
	capslock = 58,
	f1 = 59,
	f2 = 60,
	f3 = 61,
	f4 = 62,
	f5 = 63,
	f6 = 64,
	f7 = 65,
	f8 = 66,
	f9 = 67,
	f10 = 68,
	numlock = 69,
	scrolllock = 70,
	np_7 = 71,
	np_8 = 72,
	np_9 = 73,
	np_subtract = 74,
	np_4 = 75,
	np_5 = 76,
	np_6 = 77,
	np_add = 78,
	np_1 = 79,
	np_2 = 80,
	np_3 = 81,
	np_0 = 82,
	np_period = 83,
	oem_102 = 86,
	f11 = 87,
	f12 = 88,
	f13 = 100,
	f14 = 101,
	f15 = 102,
	colon = 146,
	underline = 147,
	np_enter = 156,
	rctrl = 157,
	np_divide = 181,
	print = 183,
	ralt = 184,
	pause = 197,
	home = 199,
	up = 200,
	pgup = 201,
	left = 203,
	right = 205,
	end = 207,
	down = 208,
	pgdn = 209,
	insert = 210,
	k_delete = 211,
	mouse1 = 386,
	mouse2 = 387,
	mouse3 = 388,
	mouse4 = 389,
	mouse5 = 390,
	mouse6 = 391,
	mouse7 = 392,
	mouse8 = 393,
	mwheel_up = 394,
	mwheel_down = 395,
	maxis_x = 396,
	maxis_y = 397,
	maxis_z = 398,
	Unknown = -1
};

class game_keyboard_state
{
public:
	virtual bool is_pressed( keyboard_game key ) = 0;
};

class locale_manager
{
public:
	virtual std::string get_language( ) = 0;
	virtual std::string translate( std::string name ) = 0;
};

enum game_map_id
{
	CrystalScar = 8,
	TwistedTreeline = 10,
	SummonersRift,
	HowlingAbyss,
	TFT = 22
};

class mission_info
{
public:
	virtual game_map_id get_map_id( ) = 0;
};

enum class damage_type
{
	physical = 0,
	magical = 1,
	true_dmg = 2,
};

enum class hit_chance: int32_t
{
	immobile = 8,
	dashing = 7,
	very_high = 6,
	high = 5,
	medium = 4,
	low = 3,
	impossible = 2,
	out_of_range = 1,
	collision = 0
};

class script_spell;
class target_selector_manager
{
public:
	virtual game_object_script get_target( float range, damage_type damage_type, bool ignore_invulnerability = false, bool is_missile = false ) = 0;

	virtual uintptr_t add_custom_target_selector(
		std::string name, std::function<game_object_script( float range, damage_type damage_type, bool ignore_invulnerability, bool is_missile )> callback,
		std::function<game_object_script( const std::vector<game_object_script>& t, damage_type damage_type )> _callback2,
		std::function<game_object_script( )> _get_selected_target,
		std::function<void( game_object_script target )> _set_selected_target,
		std::function<float( game_object_script target )> _get_priority,
		std::function<void( game_object_script target, int priority )> _set_priority ) = 0;

	virtual uintptr_t get_active_target_selector( ) = 0;
	virtual void remove_custom_target_selector( std::string name ) = 0;
	virtual void remove_custom_target_selector( uintptr_t id ) = 0;
	virtual game_object_script get_target( const std::vector<game_object_script>& t, damage_type damage_type ) = 0;
	virtual game_object_script get_selected_target( ) = 0;
	virtual void set_selected_target( game_object_script target ) = 0;
	virtual float get_priority( game_object_script target ) = 0;
	virtual void set_priority( game_object_script target, int priority ) = 0;

	game_object_script get_target_min_hitchance( script_spell* spell, hit_chance min_hitchance );
	game_object_script get_target_min_hitchance( script_spell* spell, hit_chance min_hitchance, damage_type damage_type );
	game_object_script get_target( script_spell* spell, damage_type damage_type );

	bool is_invulnerable( game_object_script target );
	bool has_spellshield( game_object_script target );
};

//This TS attempts to always lock the same target, useful for people getting targets for each spell, or for champions
//that have to burst 1 target.
class locked_target_selector
{
public:
	static game_object_script get_last_target( );
	static void unlock_target( );
	static game_object_script get_target( script_spell* spell, damage_type damage_type );
	static game_object_script get_target( float range, damage_type damage_type );

private:
	static std::uint16_t _last_target_id;
	static std::uint32_t _last_target_network_id;
	static damage_type _last_damage_type;
};

enum class skillshot_type: int32_t
{
	skillshot_line,
	skillshot_circle,
	skillshot_cone
};

enum class collisionable_objects: int32_t
{
	minions,
	heroes,
	yasuo_wall,
	walls,
	allies
};

class prediction_input
{
public:
	prediction_input( );
	bool aoe = false;
	std::vector<collisionable_objects> collision_objects =
	{
		//collisionable_objects::minions, collisionable_objects::yasuo_wall
	};
	float delay = 0.f;
	float radius = 1.f;
	float range = FLT_MAX;
	float speed = FLT_MAX;
	skillshot_type type = skillshot_type::skillshot_line;
	game_object_script unit = nullptr;
	bool use_bounding_radius = true;

	vector _from;
	vector _range_check_from;

	spellslot spell_slot = spellslot::invalid;

	vector get_from( );
	vector get_range_check_from( );

	float get_real_radius( );
};

class prediction_output
{
public:
	std::vector<game_object_script> aoe_targets_hit;
	std::vector<game_object_script> collision_objects;
	hit_chance hitchance = hit_chance::impossible;
	int _aoe_targets_hit_count = 0;
	prediction_input input;
	vector _cast_position;
	vector _unit_position;
	int aoe_targets_hit_count( );
	vector get_cast_position( );
	vector get_unit_position( );
};

class prediction_manager
{
public:
	virtual prediction_output get_prediction( prediction_input* input ) = 0;
	virtual uintptr_t add_prediction_callback( std::string _name, std::function<prediction_output( prediction_input* input )> _callback ) = 0;
	virtual uintptr_t get_active_prediction_selector( ) = 0;
	virtual void remove_prediction_callback( uintptr_t id ) = 0;
	virtual void remove_prediction_callback( std::string _name ) = 0;

	prediction_output get_prediction( game_object_script unit, float delay );
	prediction_output get_prediction( game_object_script unit, float delay, float radius );
	prediction_output get_prediction( game_object_script unit, float delay, float radius, float speed );
	prediction_output get_prediction( game_object_script unit, float delay, float radius, float speed, std::vector<collisionable_objects> collisionable );
};

class entity_list
{
public:

	// AIHeroClient
	//
	// These lists returns "raw" objects
	// To check visiblity you need to call is_visible
	// To check target you need to call is_valid_target
	//
	virtual const std::vector<game_object_script>& get_all_heroes( ) = 0;
	virtual const std::vector<game_object_script>& get_ally_heroes( ) = 0; // Ally heroes are all always visible
	virtual const std::vector<game_object_script>& get_enemy_heroes( ) = 0;


	// AIMinionClient
	//
	// These lists returns "raw" objects
	// To check visiblity you need to call is_visible
	// To check target you need to call is_valid_target
	// To check minion type you can call is_monster, is_lane_minion etc.
	//
	virtual const std::vector<game_object_script>& get_all_minions( ) = 0;
	virtual const std::vector<game_object_script>& get_ally_minions( ) = 0; // Ally minions are all always visible

	// AIMinionClient
	//
	// These lists returns already prepered objects
	//
	// They are visible and can be attacked  
	//   You don't need to call is_visible and is_valid_target
	//
	virtual const std::vector<game_object_script>& get_enemy_minions( ) = 0;
	virtual const std::vector<game_object_script>& get_plants_minions( ) = 0;
	virtual const std::vector<game_object_script>& get_jugnle_mobs_minions( ) = 0;
	virtual const std::vector<game_object_script>& get_barrels_minions( ) = 0;

	// AIMinionClient
	//
	// Minions which are not in any other lists
	//
	//  They are visible and can't be attacked
	//
	virtual const std::vector<game_object_script>& get_other_minion_objects( ) = 0;

	// SpawnPoint
	// 
	// Returns map spawn points
	//
	virtual const std::vector<game_object_script>& get_all_spawnpoints( ) = 0;

	// HQ
	//
	// Returns map nexuses
	//
	virtual const std::vector<game_object_script>& get_all_nexus( ) = 0;

	// AITurrentClient
	//
	// These lists returns "raw" objects
	// To check visiblity you need to call is_visible
	// To check target you need to call is_valid_target
	//
	virtual const std::vector<game_object_script>& get_all_turrets( ) = 0;
	virtual const std::vector<game_object_script>& get_ally_turrets( ) = 0; // Ally turrets are all always visible
	virtual const std::vector<game_object_script>& get_enemy_turrets( ) = 0;

	// BarracksDampener
	//
	// These lists returns "raw" objects
	// To check visiblity you need to call is_visible
	// To check target you need to call is_valid_target
	//
	virtual const std::vector<game_object_script>& get_all_inhibitors( ) = 0;
	virtual const std::vector<game_object_script>& get_ally_inhibitors( ) = 0; // Ally inhibitors are all always visible
	virtual const std::vector<game_object_script>& get_enemy_inhibitors( ) = 0;

	// Get object by id
	//
	//  If not found returns nullptr
	//
	virtual game_object_script get_object( uint16_t id ) = 0;

	// Returns current maximum capability of object list
	//
	virtual uint32_t get_max_objects( ) = 0;

	// AIMinionClient
	//
	// Returns enemy wards list
	// They are visible and can be attacked
	//   You don't need to call is_visible and is_valid_target
	//
	virtual const std::vector<game_object_script>& get_enemy_wards( ) = 0;

	// Get object by network id
	//
	// If not found returns nullptr
	//
	virtual game_object_script get_object_by_network_id( std::uint32_t network_id ) = 0;
};

namespace TreeHotkeyMode
{
	enum
	{
		Hold,
		Toggle
	};
};

namespace TreeEntryType
{
	enum
	{
		MainTreeTab,
		TreeTab,
		Separator,
		Hotkey,
		Checkbox,
		Slider,
		DisplayInt,
		Combobox,
		ColorPicker,
		ProrityList,
		Image
	};
};

struct ProrityCheckItem
{
	std::uint32_t key;
	std::string display_name;
	bool is_active = false;
	void* assigned_texture = nullptr;
};

class TreeEntry;
typedef void( __cdecl* PropertyChangeCallback )( TreeEntry* );

class TreeEntry
{
public:
	virtual ~TreeEntry( );

	// Returns bool value of element
	//
	// TreeTab*
	//   Returns false if assigned_active is not specify otherwise it returns assigned_active current value
	//
	// TreeHotkey*
	//   Returns current hotkey_state
	//
	// TreeCheckbox*
	//   Returns true if checkbox is checked
	//
	// Other types of menu element returns false
	//
	virtual bool get_bool( );

	// Returns integer value of element
	//
	// TreeSlider*
	//   Returns current slider value
	//
	// TreeDisplayInt*
	//   Returns current display value
	//
	// TreeCombobox*
	//   Returns selected index
	//
	// TreeHotkey*
	//   Returns associated virtual key to hotkey
	//
	// TreeColorPicker*
	//   Returns current color value from picker
	//
	// Other types of menu element returns 0
	//
	virtual std::int32_t get_int( );

	// Returns color value of element
	//
	// TreeColorPicker*
	//   Returns current color value from picker
	//
	// Other types of menu element returns 0
	//
	virtual std::uint32_t get_color( );

	// Returns pair <prority, active> value of element
	//
	// TreeProrityList*
	//   Returns pair <prority, active> of element associated with given key.
	//   If given key wasn't found return value is {-1, false}
	//   If given key was found return value is {i, active}
	//      Where i = index (from 0) on list, top is 0, bottom is elements size - 1
	//            active = element checked, always true if always_active option is true while creating
	//
	// Other types of menu element returns {-1, false}
	//
	virtual std::pair<std::int32_t, bool> get_prority( std::uint32_t key );

	// Sets bool value of element
	//
	//   See get_bool function for supported elements
	//
	virtual void set_bool( bool value );

	// Sets int value of element
	//
	//   See get_int function for supported elements
	//
	virtual void set_int( std::int32_t value );

	// Sets new list for combo element
	//
	//   See creation of TreeCombobox for more details
	//
	//   Use reset_index if you want to sets current element and scrollbar to 0
	//
	virtual void set_combo( const std::vector<std::pair<std::string, void*>>& list, bool reset_index = true );

	// Sets texture to current menu element
	//
	//   Supported types
	//     TreeImage*
	//     TreeCheckbox*
	//     TreeTab*
	//
	virtual void set_texture( void* texture );

	// Determines if element is hidden
	//
	//  This function returns reference so you can change the value itself
	//    element->is_hidden() = true;
	//
	virtual bool& is_hidden( );

	// Determines if element should be saved after new session
	//
	// This function returns reference so you can change the value itself
	// You can also specify this value while creating element
	//
	virtual bool& should_save( );

	// Gets display name of element
	//
	virtual const std::string& display_name( );

	// Gets key of element
	// 
	virtual const std::string& key( );

	// Gets config path of element
	//
	virtual const std::string& config_key( );

	// Gets element type
	//
	virtual std::int32_t element_type( );

	// Add property change callback
	//
	//   See PropertyChangeCallback section to more information
	//
	virtual void add_property_change_callback( PropertyChangeCallback callback );

	// Remove property change callback
	//
	//   See PropertyChangeCallback section to get more information
	//
	virtual void remove_property_change_callback( PropertyChangeCallback callback );

	// Gets tooltip of element
	//
	virtual const std::string& tooltip( );

	// Sets display name of element
	//
	virtual void set_display_name( const std::string& display_name );

	// Sets tooltip of element
	//
	//   See Tooltip section to get more information
	//
	virtual void set_tooltip( const std::string& tooltip );

	//Reserved for core
	//
	virtual void reserved_0( );
	virtual void reserved_1( );
};

class TreeTab: public TreeEntry
{
public:

	// Gets menu element by key, only for this tab
	//
	// If not found returns nullptr
	//
	virtual TreeEntry* get_entry( const std::string& key );

	// Creates or gets (if already exists and type is TreeTab) menu element
	//
	// See TreeTab element to get more information
	//
	virtual TreeTab* add_tab( const std::string& key, const std::string& name );

	// Creates or gets (if already exists and type is TreeSeperator) menu element
	//
	// See TreeSeperator element to get more information
	//
	virtual TreeEntry* add_separator( const std::string& key, const std::string& name );

	// Creates or gets (if already exists and type is TreeHotkey) menu element
	//
	// See TreeHotkey element to get more information
	//
	virtual TreeEntry* add_hotkey( const std::string& key, const std::string& name, const std::int32_t& type, const std::int32_t& vkey, const bool& default_value, const bool& should_save = true );

	// Creates or gets (if already exists and type is TreeCheckbox) menu element
	//
	// See TreeCheckbox element to get more information
	//
	virtual TreeEntry* add_checkbox( const std::string& key, const std::string& name, const bool& default_value, const bool& should_save = true );

	// Creates or gets (if already exists and type is TreeSlider) menu element
	//
	// See TreeSlider element to get more information
	//
	virtual TreeEntry* add_slider( const std::string& key, const std::string& name, const int32_t& default_value, const std::int32_t& min, const std::int32_t& max, const bool& should_save = true );

	// Creates or gets (if already exists and type is TreeDisplayInt) menu element
	//
	// See TreeDisplayInt element to get more information
	//
	virtual TreeEntry* add_display_value( const std::string& key, const std::string& name, std::int32_t* value );

	// Creates or gets (if already exists and type is TreeCombobox) menu element
	//
	// See TreeCombobox element to get more information
	//
	virtual TreeEntry* add_combobox( const std::string& key, const std::string& name, const std::vector<std::pair<std::string, void*>>& combo_elements, const std::int32_t& default_value, const bool& should_save = true );

	// Creates or gets (if already exists and type is TreeColorPicker) menu element
	//
	// See TreeColorPicker element to get more information
	//
	virtual TreeEntry* add_colorpick( const std::string& key, const std::string& name, std::array<float, 4> const& default_color, const bool& should_save = true );

	// Creates or gets (if already exists and type is TreeColorPicker) menu element
	//
	// See TreeColorPicker element to get more information
	//
	virtual TreeEntry* add_colorpick( const std::string& key, const std::string& name, float default_color[ 4 ], const bool& should_save = true );

	// Creates or gets (if already exists and type is TreeProrityList) menu element
	//
	// See TreeProrityList element to get more information
	//
	virtual TreeEntry* add_prority_list( const std::string& key, const std::string& name, const std::vector<ProrityCheckItem>& prority_elements, const bool& always_active = true, const bool& should_save = true );

	// Creates or gets (if already exists and type is TreeImage) menu element
	//
	// See TreeImage element to get more information
	//
	virtual TreeEntry* add_image_item( const std::string& key, void* texture, const std::int32_t& height );

	// Remove all sub elements of this tab
	//
	// After clear menu elements created with this tab will be invalid and you can't use them
	//
	virtual void clear( );

	// Sets if user is able to use revert function (right click) on this tab
	//
	// Default: false
	//
	virtual void set_revert_enabled( bool enabled );

	// Sets assigned texture for this tab
	//
	// Function is same as set_texture from TreeEntry
	//
	virtual void set_assigned_texture( void* texture );

	// Sets assigned active for this tab
	//
	// You can use bool pointer or TreeEntry pointer
	//
	//   TreeEntry must be type of TreeCheckbox
	//
	virtual void set_assigned_active( TreeEntry* active );
	virtual void set_assigned_active( bool* active );

	// Reserved for core
	//
	virtual void reserved_3( );
};

class tree_menu
{
public:
	// Delete tab and associated elements and sub tabs 
	//
	virtual bool delete_tab( TreeTab* tab ) = 0;
	virtual bool delete_tab( std::string _key ) = 0;

	// Create or get (if existing) root tab of menu
	//
	virtual TreeTab* create_tab( std::string key, std::string name ) = 0;

	// Get root tab
	//
	// nullptr - if dosen't exists
	//
	virtual TreeTab* get_tab( std::string key ) = 0;
};

enum class events
{
	on_update,
	on_draw,
	on_process_spell_cast,
	on_do_cast,
	on_stop_cast,
	on_create_object,
	on_delete_object,
	on_object_dead,
	on_object_respawn,
	on_buff_gain,
	on_buff_lose,
	on_new_path,
	on_after_attack_orbwalker = 13,
	on_before_attack_orbwalker,
	on_issue_order,
	on_cast_spell,
	on_teleport,
	on_wndproc,
	on_endscene,
	on_prereset,
	on_postreset,
	on_start,
	on_end,
	on_visibility_enter,
	on_lvlup,
	on_preupdate,
	on_play_animation,
	on_network_packet,
	events_size
};

class event_handler_manager
{
public:
	virtual void add_callback( events event, void* callback ) = 0;
	virtual void remove_callback( events event, void* callback ) = 0;

	virtual void trigger_on_before_attack_orbwalker( game_object_script target, bool* process ) = 0;
	virtual void trigger_on_after_attack_orbwalker( game_object_script target ) = 0;
};

class health_prediction_manager
{
public:
	virtual game_object_script get_aggro_turret( game_object_script minion ) = 0;
	virtual float turret_aggro_start_time( game_object_script minion ) = 0;
	virtual bool has_turret_aggro( game_object_script minion ) = 0;
	virtual bool has_minion_aggro( game_object_script minion ) = 0;
	virtual float get_health_prediction( game_object_script unit, float time, float delay = 0.07f ) = 0;
	virtual float get_lane_clear_health_prediction( game_object_script unit, float time, float delay = 0.07f ) = 0;
	virtual float get_incoming_damage( game_object_script unit, float time, bool include_skillshot ) = 0;
	virtual bool has_agro_on( game_object_script source, game_object_script target ) = 0;
	virtual int minions_aggro_count( game_object_script minion ) = 0;

	virtual uintptr_t add_health_prediction_callback( std::string _name,
		std::function<game_object_script( game_object_script minion )> _get_aggro_turret,
		std::function<float( game_object_script minion )> _turret_aggro_start_time,
		std::function<bool( game_object_script minion )> _has_turret_aggro,
		std::function<bool( game_object_script minion )> _has_minion_aggro,
		std::function<float( game_object_script unit, float time, float delay )> _get_health_prediction,
		std::function<float( game_object_script unit, float time, float delay )> _get_lane_clear_health_prediction,
		std::function<float( game_object_script unit, float time, bool include_skillshot )> _get_incoming_damage,
		std::function<bool( game_object_script source, game_object_script target )> _has_agro_on,
		std::function<int( game_object_script minion )> _minions_aggro_count ) = 0;

	virtual uintptr_t get_active_health_prediction( ) = 0;
	virtual void remove_health_prediction_callback( uintptr_t id ) = 0;
	virtual void remove_health_prediction_callback( std::string _name ) = 0;
};

class orbwalker_manager
{
public:
	virtual bool last_hit_mode( ) = 0;
	virtual bool mixed_mode( ) = 0;
	virtual bool lane_clear_mode( ) = 0;
	virtual bool combo_mode( ) = 0;
	virtual bool flee_mode( ) = 0;
	virtual bool none_mode( ) = 0;
	virtual bool harass( ) = 0;
	virtual void reset_auto_attack_timer( ) = 0;

	virtual game_object_script get_target( ) = 0;
	virtual game_object_script get_last_target( ) = 0;

	virtual float get_last_aa_time( ) = 0;
	virtual float get_last_move_time( ) = 0;

	virtual float get_my_projectile_speed( ) = 0;

	virtual bool can_attack( ) = 0;
	virtual bool can_move( float extra_windup = 0.f ) = 0;
	virtual bool should_wait( ) = 0;

	virtual void move_to( vector& pos ) = 0;
	virtual void orbwalk( game_object_script target, vector& pos ) = 0;
	virtual void set_attack( bool enable ) = 0;
	virtual void set_movement( bool enable ) = 0;

	virtual void set_orbwalking_target( game_object_script target ) = 0;
	virtual void set_orbwalking_point( vector const& pos ) = 0;

	virtual uintptr_t add_orbwalker_callback( std::string _name,
		std::function<bool( )> _last_hit_mode,
		std::function<bool( )> _mixed_mode,
		std::function<bool( )> _lane_clear_mode,
		std::function<bool( )> _combo_mode,
		std::function<bool( )> _flee_mode,
		std::function<bool( )> _none_mode,
		std::function<bool( )> _harass,
		std::function<void( )> _reset_auto_attack_timer,
		std::function<game_object_script( )> _get_target,
		std::function<game_object_script( )> _get_last_target,
		std::function<float( )> _get_last_aa_time,
		std::function<float( )> _get_last_move_time,
		std::function<float( )> _get_my_projectile_speed,
		std::function<bool( )> _can_attack,
		std::function<bool( float extra_windup )> _can_move,
		std::function<bool( )> _should_wait,
		std::function<void( vector& pos )> _move_to,
		std::function<void( game_object_script target, vector& pos )> _orbwalk,
		std::function<void( bool enable )> _set_attack,
		std::function<void( bool enable )> _set_movement,
		std::function<void( game_object_script target )> _set_orbwalking_target,
		std::function<void( vector const& pos )> _set_orbwalking_point ) = 0;

	virtual uintptr_t get_active_orbwalker( ) = 0;
	virtual void remove_orbwalker_callback( uintptr_t id ) = 0;
	virtual void remove_orbwalker_callback( std::string _name ) = 0;
};

struct damage_input
{
	bool is_auto_attack = false;
	bool is_critical_attack = false;
	bool is_ability = true;
	bool dont_include_passives = false;
	bool dont_calculate_item_damage = false;
	bool is_on_hit_damage = false;
	bool is_targeted_ability = false;
	bool applies_on_hit_damage = false;
	bool doesnt_trigger_on_hit_effects = false;

	float raw_physical_damage{};
	float raw_magical_damage{};
	float raw_true_damage{};

	damage_input( ) = default;
};

struct skin_info
{
	std::string name;
	std::string model;
	uint32_t skin_id;
};

class damagelib_manager
{
public:
	virtual float get_auto_attack_damage( game_object_script source, game_object_script target, bool respect_passives = false ) = 0;
	virtual float calculate_damage_on_unit( game_object_script source, game_object_script target, damage_input* input ) = 0;
	virtual float calculate_damage_on_unit( game_object_script source, game_object_script target, damage_type damage_type, float raw_damage ) = 0;
	virtual float get_spell_damage( game_object_script source, game_object_script target, spellslot slot, bool return_raw_damage = false ) = 0;

	virtual uintptr_t add_damagelib_callback( std::string _name,
		std::function<float( game_object_script source, game_object_script target, bool respect_passives )> _get_auto_attack_damage,
		std::function<float( game_object_script source, game_object_script target, damage_input* input )> _calculate_damage_on_unit,
		std::function<float( game_object_script source, game_object_script target, damage_type damage_type, float raw_damage )> _calculate_damage_on_unit2,
		std::function<float( game_object_script source, game_object_script target, spellslot slot, bool return_raw_damage )> _get_spell_damage ) = 0;

	virtual uintptr_t get_active_damagelib_selector( ) = 0;
	virtual void remove_damagelib_callback( uintptr_t id ) = 0;
	virtual void remove_damagelib_callback( std::string _name ) = 0;
};

struct ImVec4
{
	float     x, y, z, w;
	ImVec4( ) { x = y = z = w = 0.0f; }
	ImVec4( float _x, float _y, float _z, float _w ) { x = _x; y = _y; z = _z; w = _w; }
};

struct loaded_texture
{
	std::uint32_t* texture;
	std::uint32_t width;
	std::uint32_t height;
};

class drawning_manager
{
public:
	virtual void add_text( vector const& point, unsigned long color, int font_size, const char* format, ... );
	virtual void add_text_on_screen( vector const& point, unsigned long color, int font_size, const char* format, ... );
	virtual void add_circle( vector const& center, float radius, unsigned long color, float thickness = 1.f, float height = -1.f, int num_segments = 200 );
	virtual void add_circle_on_screen( vector const& center, float radius, unsigned long color, float thickness = 1.0f, int num_segments = 200 );
	virtual void add_line( vector const& a, vector const& b, unsigned long color, float thickness = 1.f );
	virtual void add_line_on_screen( vector const& start, vector const& end, unsigned long color, float thickness = 1.0f );
	virtual void add_rect( const vector& a, const vector& b, unsigned long col, float rounding = 0.0f, int rounding_corners_flags = ~0 );
	virtual void add_filled_rect( const vector& a, const vector& b, unsigned long col, float rounding = 0.0f, int rounding_corners_flags = ~0 );
	virtual void add_image( uint32_t* user_texture_id, const vector& pos, const vector& size, float rounding = 0.0f, const vector& uv0 = vector( 0, 0 ), const vector& uv1 = vector( 1, 1 ), const ImVec4& tint_col = ImVec4( 1, 1, 1, 1 ), const ImVec4& border_col = ImVec4( 0, 0, 0, 0 ) );
	virtual vector calc_text_size( int font_size, const char* format, ... ) = 0;
	virtual loaded_texture* load_texture_from_file( const std::wstring& path ) = 0;
	void draw_circle_on_minimap( vector const& center, float radius, unsigned long color, float thickness = 1.f, int quality = 40 );
};

class sound_manager
{
public:
	virtual const std::wstring& get_current_song( ) = 0;
	virtual bool is_playing( ) = 0;
	virtual void clear_queue( ) = 0;
	virtual void stop_play( ) = 0;
	virtual void play_sound( std::uintptr_t* sound, float volume = 1.f ) = 0;
	virtual std::uintptr_t* load_sound( const std::wstring& name ) = 0;
};

class scheduler_manager
{
public:
	virtual void delay_action( float time, std::function<void( )> function );
};

class console_manager
{
public:
	virtual void print( const char* format, ... ) = 0;
	virtual void print_warning( const char* format, ... ) = 0;
	virtual void print_success( const char* format, ... ) = 0;
	virtual void print_error( const char* format, ... ) = 0;
};

class glow_manager
{
public:
	virtual void remove_glow( game_object_script obj ) = 0;
	virtual bool can_be_glowed( game_object_script obj ) = 0;
	virtual bool is_glowed( game_object_script obj ) = 0;
	virtual bool apply_glow( game_object_script obj, uint32_t color, int thick = 4, int blur = 5 ) = 0;
};

#define sciprt_spell_wait 0.1f
class script_spell
{
public:
	float charged_min_range{};
	float charged_max_range{};
	float charging_started_time{};
	float charge_duration{};

	float radius{};
	float speed{};
	float delay{};
	float _range{};

	bool collision{};
	bool is_charged_spell = false;

	spellslot slot = spellslot::invalid;

	vector from{};
	skillshot_type type{};
	std::vector<collisionable_objects> collision_flags{};
	damage_type _damage_type{};

	script_spell( );
	script_spell( spellslot slot );
	script_spell( spellslot slot, float range );
	script_spell( spellslot slot, float range, skillshot_type type, float delay, float speed, float radius, bool collision );

	void update_chargeable_spell_handle( spellslot slot, bool release_cast );

	virtual ~script_spell( ) = default;
	virtual spell_data_inst_script handle( );
	virtual std::string name( );
	virtual uint32_t name_hash( );

	virtual float cooldown_time( );
	virtual float mana_cost( );
	virtual float range( );
	virtual float charged_percentage( );

	virtual int ammo( );
	virtual int toogle_state( );
	virtual int level( );

	virtual bool is_ready( float time = 0.f );

	virtual bool cast( );
	virtual bool cast( game_object_script unit, hit_chance minimum, bool aoe = false, int min_targets = 0 );
	virtual bool cast( vector position );
	virtual bool cast( game_object_script unit );
	virtual bool cast( vector start_position, vector end_position );
	virtual bool fast_cast( vector position );

	virtual bool cast_on_best_farm_position( int min_minions = 2, bool is_jugnle_mobs = false );

	virtual bool is_charging( );
	virtual bool is_fully_charged( );

	virtual bool start_charging( );
	virtual bool start_charging( const vector& position );

	virtual float get_damage( game_object_script target, int stage = 0 );

	virtual std::vector<collisionable_objects> get_collision_flags( );
	virtual vector range_check_from( );

	virtual void set_radius( float radius );
	virtual void set_speed( float speed );
	virtual void set_delay( float delay );
	virtual void set_range( float range );
	virtual void set_sollision_flags( std::vector <collisionable_objects> flags );
	virtual void set_range_check_from( vector const& position );
	virtual void set_skillshot( float delay, float radius, float speed, std::vector<collisionable_objects> flags, skillshot_type skillshot_type );
	virtual void set_charged( float range_min, float range_max, float charge_duration );

	virtual prediction_output get_prediction( game_object_script target, bool aoe = false, float overrideRange = -1, std::vector<collisionable_objects> collisionable = {} );
	virtual prediction_output get_prediction_no_collision( game_object_script target, bool aoe = false, float overrideRange = -1 );

	virtual float get_last_cast_spell_time( );

	virtual vector get_cast_on_best_farm_position( int min_minions = 2, bool is_jugnle_mobs = false );

	virtual void set_damage_type( damage_type type );
	virtual damage_type get_damage_type( );

	virtual spellslot get_slot( );
	virtual float get_speed( );
	virtual float get_delay( );
	virtual float get_radius( );

	virtual bool can_cast( game_object_script unit );

	virtual bool is_in_range( game_object_script target, float range = -1 );
	virtual bool is_in_range( vector const& point, float range = -1 );

	int8_t icon_index( );
	game_object_script get_target( float extra_range = 0 );
	prediction_output get_prediction( game_object_script target, vector origin, vector range_check_from );

private:
	float last_cast_spell = 0.f;

	buff_instance_script get_charge_buff( );
};

class plugin_sdk_core
{
public:
	virtual game_state* get_game_state( ) = 0;
	virtual r3d_renderer* get_r3d_renderer( ) = 0;
	virtual hud_manager* get_hud_manager( ) = 0;
	virtual menu_gui* get_menu_gui( ) = 0;
	virtual game_time* get_game_time( ) = 0;
	virtual game_ping* get_game_ping( ) = 0;
	virtual input* get_input( ) = 0;
	virtual game_event_manager* get_game_event_manager( ) = 0;
	virtual nav_mesh* get_nav_mesh( ) = 0;
	virtual game_keyboard_state* get_game_keyboard_state( ) = 0;
	virtual locale_manager* get_locale_manager( ) = 0;
	virtual mission_info* get_mission_info( ) = 0;
	virtual game_object_script get_myhero( ) = 0;
	virtual target_selector_manager* get_target_selector_manager( ) = 0;
	virtual prediction_manager* get_prediction_manager( ) = 0;
	virtual entity_list* get_entity_list( ) = 0;
	virtual tree_menu* get_tree_menu( ) = 0;
	virtual event_handler_manager* get_event_handler_manager( ) = 0;
	virtual health_prediction_manager* get_health_prediction_manager( ) = 0;
	virtual orbwalker_manager* get_orbwalker_manager( ) = 0;
	virtual damagelib_manager* get_damagelib_manager( ) = 0;
	virtual drawning_manager* get_drawning_manager( ) = 0;
	virtual scheduler_manager* get_scheduler_manager( ) = 0;
	virtual std::map<champion_id, std::vector<skin_info>>& get_skins_database( ) = 0;
	virtual console_manager* get_console_manager( ) = 0;
	virtual glow_manager* get_glow_manager( ) = 0;
	virtual void* get_is_valid_function( ) = 0;
	virtual sound_manager* get_sound_manager( ) = 0;
	script_spell* register_spell( spellslot slot, float range );
	bool remove_spell( script_spell* spell );
};

extern plugin_sdk_core* plugin_sdk;

template < events event >
struct event_handler
{
	static void add_callback( ) { }
	static void remove_handler( ) { }
};

template < >
struct event_handler<events::on_update>
{
	static void add_callback( void( *callback )( ) ) { plugin_sdk->get_event_handler_manager( )->add_callback( events::on_update, ( void* ) callback ); }
	static void remove_handler( void( *callback )( ) ) { plugin_sdk->get_event_handler_manager( )->remove_callback( events::on_update, ( void* ) callback ); }
};

template < >
struct event_handler<events::on_preupdate>
{
	static void add_callback( void( *callback )( ) ) { plugin_sdk->get_event_handler_manager( )->add_callback( events::on_preupdate, ( void* ) callback ); }
	static void remove_handler( void( *callback )( ) ) { plugin_sdk->get_event_handler_manager( )->remove_callback( events::on_preupdate, ( void* ) callback ); }
};

template < >
struct event_handler<events::on_draw>
{
	static void add_callback( void( *callback )( ) ) { plugin_sdk->get_event_handler_manager( )->add_callback( events::on_draw, ( void* ) callback ); }
	static void remove_handler( void( *callback )( ) ) { plugin_sdk->get_event_handler_manager( )->remove_callback( events::on_draw, ( void* ) callback ); }
};

template < >
struct event_handler<events::on_network_packet>
{
	static void add_callback( void( *callback )( game_object_script sender, std::uint32_t network_id, pkttype_e type, void* args ) ) { plugin_sdk->get_event_handler_manager( )->add_callback( events::on_network_packet, ( void* ) callback ); }
	static void remove_handler( void( *callback )( game_object_script sender, std::uint32_t network_id, pkttype_e type, void* args ) ) { plugin_sdk->get_event_handler_manager( )->remove_callback( events::on_network_packet, ( void* ) callback ); }
};

template < >
struct event_handler<events::on_issue_order>
{
	static void add_callback( void( *callback )( game_object_script& target, vector& pos, _issue_order_type& type, bool* process ) ) { plugin_sdk->get_event_handler_manager( )->add_callback( events::on_issue_order, ( void* ) callback ); }
	static void remove_handler( void( *callback )( game_object_script& target, vector& pos, _issue_order_type& type, bool* process ) ) { plugin_sdk->get_event_handler_manager( )->remove_callback( events::on_issue_order, ( void* ) callback ); }
};

template < >
struct event_handler<events::on_process_spell_cast>
{
	static void add_callback( void( *callback )( game_object_script sender, spell_instance_script spell ) ) { plugin_sdk->get_event_handler_manager( )->add_callback( events::on_process_spell_cast, ( void* ) callback ); }
	static void remove_handler( void( *callback )( game_object_script sender, spell_instance_script spell ) ) { plugin_sdk->get_event_handler_manager( )->remove_callback( events::on_process_spell_cast, ( void* ) callback ); }
};

template < >
struct event_handler<events::on_do_cast>
{
	static void add_callback( void( *callback )( game_object_script sender, spell_instance_script spell ) ) { plugin_sdk->get_event_handler_manager( )->add_callback( events::on_do_cast, ( void* ) callback ); }
	static void remove_handler( void( *callback )( game_object_script sender, spell_instance_script spell ) ) { plugin_sdk->get_event_handler_manager( )->remove_callback( events::on_do_cast, ( void* ) callback ); }
};

template < >
struct event_handler<events::on_stop_cast>
{
	static void add_callback( void( *callback )( game_object_script sender, spell_instance_script spell ) ) { plugin_sdk->get_event_handler_manager( )->add_callback( events::on_stop_cast, ( void* ) callback ); }
	static void remove_handler( void( *callback )( game_object_script sender, spell_instance_script spell ) ) { plugin_sdk->get_event_handler_manager( )->remove_callback( events::on_stop_cast, ( void* ) callback ); }
};

template < >
struct event_handler<events::on_create_object>
{
	static void add_callback( void( *callback )( game_object_script sender ) ) { plugin_sdk->get_event_handler_manager( )->add_callback( events::on_create_object, ( void* ) callback ); }
	static void remove_handler( void( *callback )( game_object_script sender ) ) { plugin_sdk->get_event_handler_manager( )->remove_callback( events::on_create_object, ( void* ) callback ); }
};

template < >
struct event_handler<events::on_delete_object>
{
	static void add_callback( void( *callback )( game_object_script sender ) ) { plugin_sdk->get_event_handler_manager( )->add_callback( events::on_delete_object, ( void* ) callback ); }
	static void remove_handler( void( *callback )( game_object_script sender ) ) { plugin_sdk->get_event_handler_manager( )->remove_callback( events::on_delete_object, ( void* ) callback ); }
};

template < >
struct event_handler<events::on_object_dead>
{
	static void add_callback( void( *callback )( game_object_script sender ) ) { plugin_sdk->get_event_handler_manager( )->add_callback( events::on_object_dead, ( void* ) callback ); }
	static void remove_handler( void( *callback )( game_object_script sender ) ) { plugin_sdk->get_event_handler_manager( )->remove_callback( events::on_object_dead, ( void* ) callback ); }
};

template < >
struct event_handler<events::on_object_respawn>
{
	static void add_callback( void( *callback )( game_object_script sender ) ) { plugin_sdk->get_event_handler_manager( )->add_callback( events::on_object_respawn, ( void* ) callback ); }
	static void remove_handler( void( *callback )( game_object_script sender ) ) { plugin_sdk->get_event_handler_manager( )->remove_callback( events::on_object_respawn, ( void* ) callback ); }
};

template < >
struct event_handler<events::on_teleport>
{
	static void add_callback( void( *callback )( game_object_script sender, teleport_type type, teleport_status status, float duration ) ) { plugin_sdk->get_event_handler_manager( )->add_callback( events::on_teleport, ( void* ) callback ); }
	static void remove_handler( void( *callback )( game_object_script sender, teleport_type type, teleport_status status, float duration ) ) { plugin_sdk->get_event_handler_manager( )->remove_callback( events::on_teleport, ( void* ) callback ); }
};

template < >
struct event_handler<events::on_cast_spell>
{
	static void add_callback( void( *callback )( spellslot spell_slot, game_object_script target, vector& pos, vector& pos2, bool is_charge, bool* process ) ) { plugin_sdk->get_event_handler_manager( )->add_callback( events::on_cast_spell, ( void* ) callback ); }
	static void remove_handler( void( *callback )( spellslot spell_slot, game_object_script target, vector& pos, vector& pos2, bool is_charge, bool* process ) ) { plugin_sdk->get_event_handler_manager( )->remove_callback( events::on_cast_spell, ( void* ) callback ); }
};

template < >
struct event_handler<events::on_buff_gain>
{
	static void add_callback( void( *callback )( game_object_script sender, buff_instance_script buff ) ) { plugin_sdk->get_event_handler_manager( )->add_callback( events::on_buff_gain, ( void* ) callback ); }
	static void remove_handler( void( *callback )( game_object_script sender, buff_instance_script buff ) ) { plugin_sdk->get_event_handler_manager( )->remove_callback( events::on_buff_gain, ( void* ) callback ); }
};

template < >
struct event_handler<events::on_buff_lose>
{
	static void add_callback( void( *callback )( game_object_script sender, buff_instance_script buff ) ) { plugin_sdk->get_event_handler_manager( )->add_callback( events::on_buff_lose, ( void* ) callback ); }
	static void remove_handler( void( *callback )( game_object_script sender, buff_instance_script buff ) ) { plugin_sdk->get_event_handler_manager( )->remove_callback( events::on_buff_lose, ( void* ) callback ); }
};

template < >
struct event_handler<events::on_new_path>
{
	static void add_callback( void( *callback )( game_object_script sender, const std::vector<vector>& path, bool is_dash, float dash_speed ) ) { plugin_sdk->get_event_handler_manager( )->add_callback( events::on_new_path, ( void* ) callback ); }
	static void remove_handler( void( *callback )( game_object_script sender, const std::vector<vector>& path, bool is_dash, float dash_speed ) ) { plugin_sdk->get_event_handler_manager( )->remove_callback( events::on_new_path, ( void* ) callback ); }
};

template < >
struct event_handler<events::on_play_animation>
{
	static void add_callback( void( *callback )( game_object_script sender, const char* name ) ) { plugin_sdk->get_event_handler_manager( )->add_callback( events::on_play_animation, ( void* ) callback ); }
	static void remove_handler( void( *callback )( game_object_script sender, const char* name ) ) { plugin_sdk->get_event_handler_manager( )->remove_callback( events::on_play_animation, ( void* ) callback ); }
};

template < >
struct event_handler<events::on_after_attack_orbwalker>
{
	static void add_callback( void( *callback )( game_object_script target ) ) { plugin_sdk->get_event_handler_manager( )->add_callback( events::on_after_attack_orbwalker, ( void* ) callback ); }
	static void remove_handler( void( *callback )( game_object_script target ) ) { plugin_sdk->get_event_handler_manager( )->remove_callback( events::on_after_attack_orbwalker, ( void* ) callback ); }
	static void invoke( game_object_script target ) { plugin_sdk->get_event_handler_manager( )->trigger_on_after_attack_orbwalker( target ); }
};

template < >
struct event_handler<events::on_before_attack_orbwalker>
{
	static void add_callback( void( *callback )( game_object_script target, bool* process ) ) { plugin_sdk->get_event_handler_manager( )->add_callback( events::on_before_attack_orbwalker, ( void* ) callback ); }
	static void remove_handler( void( *callback )( game_object_script target, bool* process ) ) { plugin_sdk->get_event_handler_manager( )->remove_callback( events::on_before_attack_orbwalker, ( void* ) callback ); }
	static void invoke( game_object_script target, bool* process ) { plugin_sdk->get_event_handler_manager( )->trigger_on_before_attack_orbwalker( target, process ); }
};

extern game_state* state;
extern r3d_renderer* renderer;
extern hud_manager* hud;
extern menu_gui* gui;
extern game_time* gametime;
extern game_ping* ping;
extern input* game_input;
extern game_event_manager* event_manager;
extern nav_mesh* navmesh;
extern game_keyboard_state* keyboard_state;
extern locale_manager* locale;
extern mission_info* missioninfo;
extern game_object_script myhero;
extern target_selector_manager* target_selector;
extern prediction_manager* prediction;
extern entity_list* entitylist;
extern tree_menu* menu;
extern health_prediction_manager* health_prediction;
extern orbwalker_manager* orbwalker;
extern damagelib_manager* damagelib;
extern drawning_manager* draw_manager;
extern scheduler_manager* scheduler;
extern console_manager* console;
extern glow_manager* glow;
extern sound_manager* sound;

namespace geometry
{
#define CIRCLE_LINE_SEGMENTS 16

	class polygon
	{
	public:
		std::vector<vector> points;

		void add( const vector& point )
		{
			points.push_back( point );
		}

		bool is_inside( const vector& point )
		{
			return !is_outside( point );
		}

		bool is_outside( const vector& point )
		{
			const auto p = ClipperLib::IntPoint( ( int ) point.x, ( int ) point.y );
			return ClipperLib::PointInPolygon( p, to_clipper_path( ) ) != 1;
		}

		int point_in_polygon( const vector& point )
		{
			const auto p = ClipperLib::IntPoint( ( int ) point.x, ( int ) point.y );
			return ClipperLib::PointInPolygon( p, to_clipper_path( ) );
		}

		std::vector<ClipperLib::IntPoint> to_clipper_path( )
		{
			std::vector<ClipperLib::IntPoint> result;

			for ( const auto& point : points )
				result.emplace_back( ( int ) point.x, ( int ) point.y );

			return result;
		}
	};

	class circle
	{
	public:
		vector center;
		float radius;

		circle( const vector& _center, float _radius )
		{
			center = _center;
			radius = _radius;
		}

		polygon to_polygon( int offset = 0, float overrideWidth = -1 )
		{
			polygon result = polygon( );
			float outRadius = overrideWidth > 0
				? overrideWidth
				: ( offset + radius ) / ( float ) cos( 2 * M_PI / CIRCLE_LINE_SEGMENTS );

			double step = 2 * M_PI / CIRCLE_LINE_SEGMENTS;
			double angle = radius;
			for ( int i = 0; i <= CIRCLE_LINE_SEGMENTS; i++ )
			{
				angle += step;
				vector point = vector( center.x + outRadius * ( float ) cos( angle ),
					center.y + outRadius * ( float ) sin( angle ), this->center.z );
				result.add( point );
			}

			return result;
		}
	};

	class rectangle
	{
	public:
		vector direction;
		vector perpendicular;
		vector r_end;
		vector r_start;
		float width;


		rectangle( const vector& _start, const vector& _end, float _widthStart )
		{
			r_start = _start;
			r_end = _end;
			width = _widthStart;
			direction = ( _end - _start ).normalized( );
			perpendicular = direction.perpendicular( );
		}

		polygon to_polygon( int offset = 0, float overrideWidth = -1 )
		{
			polygon result = polygon( );

			result.add( r_start +
				perpendicular * ( overrideWidth > 0 ? overrideWidth : width + ( float ) offset ) -
				direction * ( float ) offset );
			result.add( r_start -
				perpendicular * ( overrideWidth > 0 ? overrideWidth : width + ( float ) offset ) -
				direction * ( float ) offset );
			result.add( r_end -
				perpendicular * ( overrideWidth > 0 ? overrideWidth : width + ( float ) offset ) +
				direction * ( float ) offset );
			result.add( r_end +
				perpendicular * ( overrideWidth > 0 ? overrideWidth : width + ( float ) offset ) +
				direction * ( float ) offset );

			return result;
		}
	};

	class ring
	{
	public:
		vector center;
		float radius;
		float ring_radius;

		ring( const vector& _center, float _radius, float _ringRadius )
		{
			center = _center;
			radius = _radius;
			ring_radius = _ringRadius;
		}

		polygon to_polygon( int offset = 0 )
		{
			polygon result;

			float outRadius = ( float ) ( offset + this->radius + this->ring_radius ) / cosf( 2 * M_PI / CIRCLE_LINE_SEGMENTS );
			float innerRadius = this->radius - this->ring_radius - offset;

			for ( int i = 0; i <= CIRCLE_LINE_SEGMENTS; i++ )
			{
				float angle = i * 2 * M_PI / CIRCLE_LINE_SEGMENTS;
				auto point = vector(
					this->center.x - outRadius * cosf( angle ),
					this->center.y - outRadius * sinf( angle ), this->center.z );

				result.add( point );
			}
			for ( int i = 0; i <= CIRCLE_LINE_SEGMENTS; i++ )
			{
				float angle = i * 2 * M_PI / CIRCLE_LINE_SEGMENTS;
				auto point = vector(
					this->center.x - innerRadius * cosf( angle ),
					this->center.y - innerRadius * sinf( angle ), this->center.z );

				result.add( point );
			}
			return result;
		}
	};

	class sector
	{
	public:
		float angle;
		vector center;
		vector direction;
		float radius;

		sector( const vector& _center, const vector& _direction, float _angle, float _radius )
		{
			center = _center;
			direction = _direction;
			angle = _angle;
			radius = _radius;
		}

		polygon to_polygon( int offset = 0 )
		{
			polygon result = polygon( );
			float outRadius = ( radius + offset );
			result.add( center );
			vector side1 = direction.rotated( ( ( float ) M_PI / 180.f ) * ( -angle * 0.5f ) );

			for ( int i = 0; i <= CIRCLE_LINE_SEGMENTS; i++ )
			{
				vector cDirection = side1.rotated( ( ( float ) M_PI / 180.f ) * ( i * angle / CIRCLE_LINE_SEGMENTS ) ).normalized( );
				result.add( vector( center.x + outRadius * cDirection.x, center.y + outRadius * cDirection.y, center.z ) );
			}
			return result;
		}


		polygon UpdatePolygon( int offset = 0 )
		{
			polygon result = polygon( );

			auto outRadius = ( radius + offset ) / ( float ) cosf( 2 * M_PI / 20 );
			result.points.push_back( center );
			auto side1 = direction.rotated( -angle * 0.5f );

			for ( auto i = 0; i <= 20; i++ )
			{
				auto cDirection = side1.rotated( i * angle / 20 ).normalized( );
				result.points.push_back(
					{
						center.x + outRadius * cDirection.x,
						center.y + outRadius * cDirection.y
					} );
			}

			return result;
		}
	};

	class geometry
	{
	public:
		static std::vector<vector> circle_circle_intersection( const vector& center1, const vector& center2, float radius1, float radius2 )
		{
			std::vector<vector> result;

			float d = center1.distance( center2 );

			if ( d > radius1 + radius2 || d <= abs( radius1 - radius2 ) )
			{
				return result;
			}

			float a = ( radius1 * radius1 - radius2 * radius2 + d * d ) / ( 2 * d );
			float h = ( float ) sqrt( radius1 * radius1 - a * a );
			vector direction = ( center2 - center1 ).normalized( );
			vector pa = center1 + direction * a;
			vector s1 = pa + direction.perpendicular( ) * h;
			vector s2 = pa - direction.perpendicular( ) * h;

			result.push_back( s1 );
			result.push_back( s2 );
			return result;
		}

		static std::vector<vector> circle_points( const vector& origin, float range, int quality )
		{
			auto closedList = std::vector<vector>( );
			for ( int i = 1; i <= quality; ++i )
			{
				auto angle = i * 2 * M_PI / quality;
				closedList.push_back( vector( origin.x + range * static_cast< float >( cos( angle ) ), origin.y + range * static_cast< float >( sin( angle ) ) ) );
			}
			return closedList;
		}

		template <typename T>
		static std::vector<std::vector<ClipperLib::IntPoint>> clip_polygons( std::vector<T>& polygons )
		{
			std::vector<std::vector<ClipperLib::IntPoint>> subj( polygons.size( ) );
			std::vector<std::vector<ClipperLib::IntPoint>> clip( polygons.size( ) );

			for ( auto&& polygon : polygons )
			{
				subj.push_back( polygon->to_clipper_path( ) );
				clip.push_back( polygon->to_clipper_path( ) );
			}

			auto solution = std::vector<std::vector<ClipperLib::IntPoint>>( );
			ClipperLib::Clipper c;
			c.AddPaths( subj, ClipperLib::PolyType::ptSubject, true );
			c.AddPaths( clip, ClipperLib::PolyType::ptClip, true );
			c.Execute( ClipperLib::ClipType::ctUnion, solution, ClipperLib::PolyFillType::pftNegative, ClipperLib::PolyFillType::pftNonZero );

			return solution;
		}

		static std::vector<vector> cut_path( std::vector<vector>& path, float distance )
		{
			std::vector<vector> result;
			auto Distance = distance;

			if ( distance < 0 )
			{
				path[ 0 ] = path[ 0 ] + ( path[ 1 ] - path[ 0 ] ).normalized( ) * distance;
				return path;
			}

			for ( auto i = 0u; i < path.size( ) - 1; i++ )
			{
				auto dist = path[ i ].distance( path[ i + 1 ] );
				if ( dist > Distance )
				{
					result.push_back( path[ i ] + ( path[ i + 1 ] - path[ i ] ).normalized( ) * Distance );
					for ( auto j = i + 1; j < path.size( ); j++ )
					{
						result.push_back( path[ j ] );
					}
					break;
				}
				Distance -= dist;
			}
			return result.size( ) > 0 ? result : std::vector<vector>{ path.back( ) };
		}

		static float path_length( std::vector<vector> path )
		{
			float distance = 0.0f;
			for ( auto i = 0u; i < path.size( ) - 1; i++ )
			{
				distance += path[ i ].distance( path[ i + 1 ] );
			}
			return distance;
		}

		static vector vector_movement_collision( const vector& startPoint1, const vector& endPoint1, float v1, const vector& startPoint2, float v2, float& t1, float delay = 0.f )
		{
			auto sP1x = startPoint1.x;
			auto sP1y = startPoint1.y;
			auto eP1x = endPoint1.x;
			auto eP1y = endPoint1.y;
			auto sP2x = startPoint2.x;
			auto sP2y = startPoint2.y;

			float d = eP1x - sP1x, e = eP1y - sP1y;
			float dist = ( float ) sqrt( d * d + e * e );
			t1 = NAN;
			float S = abs( dist ) > FLT_EPSILON ? v1 * d / dist : 0.f,
				K = ( abs( dist ) > FLT_EPSILON ) ? v1 * e / dist : 0.f;

			float r = sP2x - sP1x, j = sP2y - sP1y;
			auto c = r * r + j * j;

			if ( dist > 0.f )
			{
				if ( abs( v1 - FLT_MAX ) < FLT_EPSILON )
				{
					auto t = dist / v1;
					t1 = v2 * t >= 0.f ? t : NAN;
				}
				else if ( abs( v2 - FLT_MAX ) < FLT_EPSILON )
				{
					t1 = 0.f;
				}
				else
				{
					float a = S * S + K * K - v2 * v2, b = -r * S - j * K;

					if ( abs( a ) < FLT_EPSILON )
					{
						if ( abs( b ) < FLT_EPSILON )
						{
							t1 = ( abs( c ) < FLT_EPSILON ) ? 0.f : NAN;
						}
						else
						{
							auto t = -c / ( 2 * b );
							t1 = ( v2 * t >= 0.f ) ? t : NAN;
						}
					}
					else
					{
						auto sqr = b * b - a * c;
						if ( sqr >= 0 )
						{
							auto nom = ( float ) sqrt( sqr );
							auto t = ( -nom - b ) / a;
							t1 = v2 * t >= 0.f ? t : NAN;
							t = ( nom - b ) / a;
							auto t2 = ( v2 * t >= 0.f ) ? t : NAN;

							if ( !isnan( t2 ) && !isnan( t1 ) )
							{
								if ( t1 >= delay && t2 >= delay )
								{
									t1 = fmin( t1, t2 );
								}
								else if ( t2 >= delay )
								{
									t1 = t2;
								}
							}
						}
					}
				}
			}
			else if ( abs( dist ) < FLT_EPSILON )
			{
				t1 = 0.f;
			}

			return ( !isnan( t1 ) ) ? vector( sP1x + S * t1, sP1y + K * t1, 0 ) : vector( 0, 0, 0 );
		}

		static vector position_after( std::vector<vector>& self, float time, float speed, float delay = 0.f )
		{
			if ( self.size( ) < 2 )
			{
				return self.size( ) == 1 ? self.front( ) : vector::zero;
			}

			float distance = std::max( 0.f, time - delay ) * speed;

			for ( auto i = 0u; i <= self.size( ) - 2; i++ )
			{
				vector from = self[ i ];
				vector to = self[ i + 1 ];
				auto d = to.distance( from );
				if ( d > distance )
				{
					return from + ( to - from ).normalized( ) * distance;
				}
				distance -= d;
			}

			return self[ self.size( ) - 1 ];
		}

		static polygon to_polygon( std::vector<ClipperLib::IntPoint>& v )
		{
			polygon _polygon = polygon( );
			for ( auto& point : v )
			{
				_polygon.add( { ( float ) point.X, ( float ) point.Y, 0 } );
			}
			return _polygon;
		}

		static std::vector<polygon> to_polygons( std::vector<std::vector<ClipperLib::IntPoint>>& v )
		{
			std::vector<polygon> result;
			for ( auto& item : v )
				result.push_back( to_polygon( item ) );

			return result;
		}
	};


	class arc
	{
	public:
		vector end;
		int hit_box;
		vector start;
		float distance;

		arc( const vector& _start, const vector& _end, int _hitbox )
		{
			start = _start;
			end = _end;
			hit_box = _hitbox;
			distance = start.distance( end );
		}

		polygon to_polygon( int offset = 0 )
		{
			offset += hit_box;
			polygon result = polygon( );

			float innerRadius = -0.1562f * distance + 687.31f;
			float outerRadius = 0.35256f * distance + 133.0f;

			outerRadius = outerRadius / ( float ) cos( 2 * M_PI / CIRCLE_LINE_SEGMENTS );

			auto innerCenters = geometry::circle_circle_intersection( start, end, innerRadius, innerRadius );
			auto outerCenters = geometry::circle_circle_intersection( start, end, outerRadius, outerRadius );

			vector innerCenter = innerCenters[ 0 ];
			vector outerCenter = outerCenters[ 0 ];

			vector direction = ( end - outerCenter ).normalized( );
			vector end = ( start - outerCenter ).normalized( );
			float maxAngle = ( float ) ( direction.angle_between( end ) * M_PI / 180.0f );

			float step = -maxAngle / CIRCLE_LINE_SEGMENTS;

			for ( int i = 0; i < CIRCLE_LINE_SEGMENTS; i++ )
			{
				float angle = step * i;
				vector point = outerCenter + direction.rotated( angle ) * ( outerRadius + 15 + offset );
				result.add( point );
			}

			direction = ( start - innerCenter ).normalized( );
			end = ( end - innerCenter ).normalized( );
			maxAngle = ( float ) ( direction.angle_between( end ) * M_PI / 180.0f );
			step = maxAngle / CIRCLE_LINE_SEGMENTS;

			for ( int i = 0; i < CIRCLE_LINE_SEGMENTS; i++ )
			{
				float angle = step * i;
				vector point = innerCenter + direction.rotated( angle ) * std::max( 0.f, innerRadius - offset - 20 );
				result.add( point );
			}

			return result;
		}
	};
}

namespace antigapcloser
{
	typedef void( *gapcloser_handler )( game_object_script sender, vector const& dash_start, vector const& dash_end, float dash_speed, bool is_ally_grab );

	//detection delay in ms
	//default 25ms
	extern float detection_delay;

	void add_event_handler( gapcloser_handler p_handler );
	void remove_event_handler( gapcloser_handler p_handler );
}

namespace math
{
	/*  See
		https://randomascii.wordpress.com/2012/01/11/tricks-with-the-floating-point-format/
		for the potential portability problems with the union and bit-fields below.
	*/
	union Float_t
	{
		Float_t( float num = 0.0f ): f( num ) {}
		// Portable extraction of components.
		bool Negative( ) const { return i < 0; }
		int32_t RawMantissa( ) const { return i & ( ( 1 << 23 ) - 1 ); }
		int32_t RawExponent( ) const { return ( i >> 23 ) & 0xFF; }

		int32_t i;
		float f;
#ifdef _DEBUG
		struct
		{   // Bitfields for exploration. Do not use in production code.
			uint32_t mantissa : 23;
			uint32_t exponent : 8;
			uint32_t sign : 1;
		} parts;
#endif
	};

	bool IsZero( float A );
	bool NearEqual( float A, float B, int maxUlpsDiff = 4 );
}