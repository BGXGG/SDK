#include "plugin_sdk.hpp"

plugin_sdk_core* plugin_sdk = nullptr;

game_state* state = nullptr;
r3d_renderer* renderer = nullptr;
hud_manager* hud = nullptr;
menu_gui* gui = nullptr;
game_time* gametime = nullptr;
game_ping* ping = nullptr;
input* game_input = nullptr;
game_event_manager* event_manager = nullptr;
nav_mesh* navmesh = nullptr;
game_keyboard_state* keyboard_state = nullptr;
locale_manager* locale = nullptr;
mission_info* missioninfo = nullptr;
game_object_script myhero = nullptr;
target_selector_manager* target_selector = nullptr;
prediction_manager* prediction = nullptr;
entity_list* entitylist = nullptr;
tree_menu* menu = nullptr;
health_prediction_manager* health_prediction = nullptr;
orbwalker_manager* orbwalker = nullptr;
damagelib_manager* damagelib = nullptr;
drawning_manager* draw_manager = nullptr;
scheduler_manager* scheduler = nullptr;
console_manager* console = nullptr;
glow_manager* glow = nullptr;
sound_manager* sound = nullptr;
evade_manager* evade = nullptr;
neutral_camp_manager* camp_manager = nullptr;
translation_manager* translation = nullptr;

std::uint16_t locked_target_selector::_last_target_id = 0;
std::uint32_t locked_target_selector::_last_target_network_id = 0;
damage_type locked_target_selector::_last_damage_type = damage_type::physical;


int __stdcall DllMain( void*, unsigned long, void* ) { return 1; }

PLUGIN_API int get_sdk_version( )
{
	return PLUGIN_SDK_VERSION;
}

PLUGIN_API void on_plugin_reconnect( )
{
	myhero = plugin_sdk->get_myhero( );
}

std::vector<std::unique_ptr<script_spell>> script_spells;
std::map< spellslot, bool > enabled_spellslots = { { spellslot::q, true }, { spellslot::w, true }, { spellslot::e, true }, { spellslot::r, true } };

bool is_hooked = false;
float next_spell_cast_t = 0.f;

void on_cast_spell( spellslot spell_slot, game_object_script target, vector& pos, vector& pos2, bool is_charge, bool* process )
{
	if ( !target && spell_slot >= spellslot::q && spell_slot <= spellslot::r && myhero->get_spell_state( spell_slot ) == spell_state::Ready )
	{
		if ( gametime->get_time( ) < next_spell_cast_t && enabled_spellslots[ spell_slot ] ) *process = false;
		else next_spell_cast_t = gametime->get_time( ) + ping->get_ping( ) / 2000.f + 0.033f;
	}
}

script_spell* plugin_sdk_core::register_spell( spellslot slot, float range )
{
	if ( slot == spellslot::invalid ) return nullptr;
	if ( !is_hooked ) event_handler<events::on_cast_spell>::add_callback( on_cast_spell );
	
	script_spells.push_back( std::make_unique<script_spell>( slot, range ) );

	return script_spells.back( ).get( );
}

bool plugin_sdk_core::remove_spell( script_spell* spell )
{
	auto const& it = std::find_if( script_spells.begin( ), script_spells.end( ), [&]( std::unique_ptr<script_spell>& x ) { return x.get( ) == spell; } );

	if ( it == script_spells.end( ) )
		return false;

	script_spells.erase( it );
	return true;
}

void drawning_manager::draw_circle_on_minimap( vector const& center, float radius, unsigned long color, float thickness, int quality )
{
	geometry::polygon result;

	result.add( vector( 0, 0, 0 ) );
	result.add( vector( 0, 14800, 0 ) );
	result.add( vector( 14800, 14800, 0 ) );
	result.add( vector( 14800, 0, 0 ) );

	vector cc = center;

	auto points = geometry::geometry::circle_points( cc, radius, quality );

	for ( size_t i = 0; i < points.size( ); ++i )
	{
		auto start = points[ i ];
		auto end = points[ points.size( ) - 1 == i ? 0 : i + 1 ];

		if ( !result.is_inside( start ) && !result.is_inside( end ) )
		{
			continue;
		}

		gui->get_tactical_map( )->to_map_coord( start, start );
		gui->get_tactical_map( )->to_map_coord( end, end );

		draw_manager->add_line_on_screen( start, end, color, thickness );
	}
}

game_object_script target_selector_manager::get_target_min_hitchance( script_spell* spell, hit_chance min_hitchance )
{
	return this->get_target_min_hitchance( spell, min_hitchance, spell->get_damage_type( ) );
}

game_object_script target_selector_manager::get_target_min_hitchance( script_spell* spell, hit_chance min_hitchance, damage_type damage_type )
{
	if ( spell != nullptr )
	{
		std::vector < game_object_script > enemies;

		for ( auto&& enemy : entitylist->get_enemy_heroes( ) )
			if ( spell->can_cast( enemy ) && spell->get_prediction( enemy ).hitchance >= min_hitchance )
				enemies.push_back( enemy );

		return target_selector->get_target( enemies, damage_type );
	}

	return nullptr;
}

game_object_script target_selector_manager::get_target( script_spell* spell, damage_type damage_type )
{
	if ( spell != nullptr )
	{
		auto is_missile = spell->speed > 0 && spell->speed != FLT_MAX;
		auto range = spell->range( );

		/*if ( spell->type == skillshot_type::skillshot_circle )
			range += spell->radius;*/

		return target_selector->get_target( range, damage_type, is_missile, true );
	}
	return nullptr;
}

bool target_selector_manager::is_invulnerable( game_object_script target )
{
	if ( target == nullptr || !target->is_valid( ) )
	{
		return false;
	}

	for ( auto&& buff : target->get_bufflist( ) )
	{
		if ( buff == nullptr || !buff->is_valid( ) || !buff->is_alive( ) )
		{
			continue;
		}

		if ( buff->get_type( ) == buff_type::Invulnerability )
			return true;

		switch ( buff->get_hash_name( ) )
		{
			case buff_hash_real( "KindredRNoDeathBuff" ):
			case buff_hash_real( "UndyingRage" ):
			case buff_hash_real( "ChronoRevive" ):
			case buff_hash_real( "ChronoShift" ):
				if ( target->get_health_percent( ) <= 10 )
				{
					return true;
				}
				break;
			case buff_hash_real( "KayleR" ):
			case buff_hash_real( "VladimirSanguinePool" ):
			case buff_hash_real( "lissandrarself" ):
			case buff_hash_real( "fioraw" ):
			{
				return true;
			}
		}
	}

	return false;
}

bool target_selector_manager::has_spellshield( game_object_script target )
{
	if ( target == nullptr || !target->is_valid( ) )
	{
		return false;
	}

	return target->get_buff_by_type( { buff_type::SpellShield, buff_type::SpellImmunity, buff_type::Invulnerability } ) != nullptr;
}

bool game_object::is_valid( bool force ) { return reinterpret_cast< bool( __thiscall* )( game_object*, bool ) >( plugin_sdk->get_is_valid_function( ) )( this, force ); }

float game_object::get_buff_time_left( uint32_t hash )
{
	auto buff = this->get_buff( hash );

	return buff ? buff->get_remaining_time( ) : 0.f;
}

float game_object::get_exp_percent( )
{
	if ( !this->is_ai_hero( ) )
		return 0;
	auto exp = this->get_exp( );
	auto level = 1;
	auto required_exp = 180 + level * 100;
	while ( level++ != this->get_level( ) )
		required_exp += 180 + level * 100;
	auto missing_exp = required_exp - exp;
	exp = 180 + this->get_level( ) * 100 - missing_exp;
	auto percentage = fmaxf( 0, fminf( 100, exp / ( 180 + this->get_level( ) * 100 ) ) );

	if ( this->get_level( ) == 18 )
		percentage = 1;

	return percentage;
}

float game_object::get_real_health( bool physical_shield, bool magical_shield )
{	
	if ( this->is_ai_base( ) )
	{
		auto result = get_health( );
		result += this->get_all_shield( );
		result += this->get_hp_regen_rate( );
		if ( physical_shield )
			result += this->get_physical_shield( );

		if ( magical_shield )
			result += this->get_magical_shield( );

		if ( this->is_ai_hero( ) )
		{
			if ( this->get_health_percent( ) > 30 )
			{
				bool has_hexdrinker = false, has_maw = false;

				for ( spellslot slot = spellslot::item_1; slot <= spellslot::item_6; slot = static_cast< spellslot >( static_cast< int >( slot ) + 1 ) )
				{
					auto item = get_item( slot );

					if ( item )
					{
						ItemId item_id = static_cast< ItemId >( item->get_item_id( ) );

						if ( item_id == ItemId::Maw_of_Malmortius )
						{
							has_hexdrinker = false;
							has_maw = true;
						}

						if ( item_id == ItemId::Hexdrinker )
							has_hexdrinker = !has_maw;

						if ( get_spell_state( slot ) != spell_state::Ready )
							continue;

						if ( item_id == ItemId::Steraks_Gage )
						{
							auto bonus_health = get_max_health( ) - get_base_hp( ) + get_stat_for_level( per_level_stat_type::health, get_level( ) );
							result += 0.75f * bonus_health;
						}

						if ( item_id == ItemId::Immortal_Shieldbow )
							result += 275 + ( get_level( ) >= 10 ? ( ( get_level( ) - 9 ) * 47.22f ) : 0 );

						if ( item_id == ItemId::Maw_of_Malmortius && magical_shield )
							result += 150 + ( this->is_melee( ) ? 50 : 0 ) + this->get_additional_attack_damage( ) * ( this->is_melee( ) ? 2.25f : 1.6875f );
					}
				}

				if ( magical_shield && has_hexdrinker)
					result += ( this->is_melee( ) ? ( 100.f + 10.f * this->get_level( ) ) : ( 75.f + 7.5f * this->get_level( ) ) );
			}

			switch ( this->get_champion( ) )
			{
				case champion_id::Kled:
				{	
					result += this->get_hp_bar_stacks( );
					break;
				}
				case  champion_id::Blitzcrank:
				{	
					if ( this->has_buff( { buff_hash( "manabarriercooldown" ), buff_hash( "manabarrier" ) } ) == false )
						result += this->get_max_mana( ) * 0.3f;

					break;
				}
				case champion_id::Yasuo:
				{
					if ( this->get_mana( ) == 100 )
					{
						int passive[] = { 115, 120, 125, 130, 135, 145, 155, 165, 180, 195, 210, 240, 270, 305, 345, 395, 455, 525 };
						int lvl = std::max( 17, this->get_level( ) - 1 );
						auto temp = passive[ lvl ];
						result += temp;
					}
					break;
				}
				default:
					break;
			}
		}
		return result;
	}
	return 0;
}

float game_object::get_distance( game_object_script to )
{
	auto vFrom = this->get_position( );
	auto vTo = to->get_position( );

	if ( this->is_ai_hero( ) )
	{
		if ( auto path_controller = this->get_path_controller( ) )
		{
			vFrom = path_controller->get_position_on_path( );
		}
	}

	if ( to->is_ai_hero( ) )
	{
		if ( auto path_controller = to->get_path_controller( ) )
		{
			vTo = path_controller->get_position_on_path( );
		}
	}

	return vFrom.distance( vTo );
}

float game_object::get_distance( const vector& to )
{
	auto vFrom = this->get_position( );

	if ( this->is_ai_hero( ) )
	{
		if ( auto path_controller = this->get_path_controller( ) )
		{
			vFrom = path_controller->get_position_on_path( );
		}
	}

	return vFrom.distance( to );
}

int32_t game_object::count_allies_in_range( float range )
{
	auto count = 0;

	for ( auto hero : ( this->is_ally( ) ? entitylist->get_ally_heroes( ) : entitylist->get_enemy_heroes( ) ) )
	{
		if ( this->get_handle( ) != hero->get_handle( ) && !hero->is_dead( ) && hero->is_visible( ) && hero->is_targetable( ) && this->get_distance( hero ) < range )
			count++;
	}

	return count;
}

int32_t game_object::count_enemies_in_range( float range )
{
	auto count = 0;

	for ( auto hero : ( this->is_ally( ) ? entitylist->get_enemy_heroes( ) : entitylist->get_ally_heroes( ) ) )
	{
		if ( hero->is_visible( ) && !hero->is_dead( ) && hero->is_targetable( ) && this->get_distance( hero ) < range )
			count++;
	}

	return count;
}

float game_object::get_immovibility_time( )
{
	float t_max = 0;

	for ( auto&& buff : this->get_bufflist( ) )
	{
		if ( !buff->is_valid( ) || !buff->is_alive( ) )
			continue;

		switch ( buff->get_type( ) )
		{
			case buff_type::Charm:
			case buff_type::Stun:
			case buff_type::Flee:
			case buff_type::Fear:
			case buff_type::Snare:
			case buff_type::Knockup:
			case buff_type::Suppression:
			case buff_type::Taunt:
			case buff_type::Asleep:
				t_max = fmaxf( t_max, buff->get_remaining_time( ) );
				break;
			default:
				break;
		}
	}

	return t_max;
}

bool game_object::is_moving( )
{
	auto pc = this->get_path_controller( );

	if ( pc )
		return pc->is_moving( );

	return false;
}

bool game_object::is_dashing( )
{
	auto pc = this->get_path_controller( );

	if ( pc )
		return pc->is_dashing( );

	return false;
}

bool game_object::has_buff_type( buff_type type )
{
	auto buff = this->get_buff_by_type( type );

	if ( buff && buff->is_valid( ) && buff->is_alive( ) )
	{
		return true;
	}

	return false;
}

bool game_object::has_buff_type( const std::vector<buff_type> & type )
{
	auto buff = this->get_buff_by_type( type );

	if ( buff && buff->is_valid( ) && buff->is_alive( ) )
	{
		return true;
	}

	return false;
}

bool game_object::is_item_ready( int32_t itemid )
{
	auto slot = this->has_item( itemid );

	if ( slot != spellslot::invalid )
	{
		return this->get_spell_state( slot ) == spell_state::Ready;
	}

	return false;
}

bool game_object::is_item_ready( ItemId itemid )
{
	return this->is_item_ready( ( int32_t ) itemid );
}

spellslot game_object::has_item( ItemId itemid )
{
	return this->has_item( ( int32_t ) itemid );
}

std::vector<vector> game_object::get_real_path( )
{
	if ( this->is_ai_base( ) && !this->is_ai_turret( ) && !this->is_nexus( ) && !this->is_inhibitor( ) )
	{
		auto const path_controller = this->get_path_controller( );
		if ( !path_controller || ( this->is_winding_up( ) && !this->get_path_controller( )->is_dashing( ) ) )
			return { this->get_position( ) };

		if ( path_controller )
		{
			if ( !path_controller->is_moving( ) )
			{
				return { this->get_position( ) };
			}

			size_t current_path_node = path_controller->get_current_path_node( );

			if ( current_path_node == 0 )
				current_path_node = 1;

			auto path = path_controller->get_path( );

			std::vector<vector> r{ this->get_position( ) };

			for ( auto i = current_path_node; i < path.size( ); ++i )
				r.push_back( path[ i ] );

			return r;
		}
	}

	return { this->get_position( ) };
}

bool game_object::is_in_auto_attack_range( game_object_script to, float additional )
{
#if INTERNAL_CORE
	return is_in_auto_attack_range_native( to.get( ), additional );
#else
	return is_in_auto_attack_range_native( to, additional );
#endif
}

bool game_object::is_in_auto_attack_range_native( game_object* to, float additional )
{
	if ( to == nullptr )
		return false;

	if ( this->get_champion( ) == champion_id::Azir )
	{
		if ( to->is_ai_hero( ) || to->is_ai_minion( ) )
		{
			bool is_allowed_object = true;
			if ( to->is_ai_minion( ) )
			{
				if ( !to->is_ward( ) )
				{
					auto hash = buff_hash_real( to->get_base_skin_name( ).c_str( ) );

					if ( hash == buff_hash( "TeemoMushroom" )
						|| hash == buff_hash( "JhinTrap" )
						|| hash == buff_hash( "NidaleeSpear" ) )
						is_allowed_object = false;
				}
				else
					is_allowed_object = false;
			}

			if ( is_allowed_object )
			{
				for ( auto& it : entitylist->get_other_minion_objects( ) )
				{
					if ( it->is_dead( ) )
						continue;

					if ( it->get_object_owner( ) != this->get_base( ) )
						continue;

					if ( it->get_name( ) != "AzirSoldier" )
						continue;

					if ( it->get_position( ).distance( this->get_position( ) ) > 730.f )
						continue;

					if ( it->get_attack_range( ) == 0.f )
						continue;

					if ( it->is_in_auto_attack_range_native( to, it->get_bounding_radius( ) * -2.f - 15.f ) )
						return true;
				}
			}
		}
	}

	auto attack_range = this->get_bounding_radius( ) + to->get_bounding_radius( ) + additional - 5;

	if ( this->is_ai_turret( ) )
		attack_range += 775.f;
	else if ( this->is_ai_hero( ) && this->get_champion( ) == champion_id::Zeri )
		attack_range += 500.f;
	else 
		attack_range += this->get_attack_range( );

	vector to_position = to->get_position( );
	vector from_position = this->get_position( );

	if ( to->is_ai_hero( ) )
		to_position = to->get_path_controller( )->get_position_on_path( );

	if ( this->is_ai_hero( ) )
		from_position = this->get_path_controller( )->get_position_on_path( );

	if ( this->is_ai_hero( ) && to->is_ai_hero( ) )
	{
		if ( this->get_path_controller( )->is_moving( ) )
			attack_range -= 5;

		if ( to->get_path_controller( )->is_moving( ) && to->get_pathing_direction( ).angle_between( from_position - to_position ) > 89 )
			attack_range -= 8;
	}

	if ( this->get_champion( ) == champion_id::Caitlyn && to->has_buff( buff_hash( "caitlynyordletrapinternal" ) ) )
		attack_range = 1300.f;

	if ( this->get_champion( ) == champion_id::Aphelios && to->has_buff( buff_hash( "aphelioscalibrumbonusrangedebuff" ) ) )
		attack_range = 1800.f;
	
	if ( this->get_champion( ) == champion_id::Samira
		 && !to->has_buff( buff_hash( "samirapcooldown" ) ) 
		 && to->has_buff_type( { buff_type::Stun, buff_type::Snare, buff_type::Knockup, buff_type::Charm, buff_type::Flee, buff_type::Taunt, buff_type::Asleep, buff_type::Suppression } ) )
	{
		attack_range += 300.f;
	}

	return from_position.distance_squared( to_position ) < attack_range * attack_range;
}

bool game_object::is_under_ally_turret( )
{
	for ( auto x : ( this->is_ally( ) ? entitylist->get_ally_turrets( ) : entitylist->get_enemy_turrets( ) ) )
	{
		if ( x->is_in_auto_attack_range_native( this ) )
			return true;
	}

	return false;
}

bool game_object::is_under_enemy_turret( )
{
	for ( auto x : ( this->is_ally( ) ? entitylist->get_enemy_turrets( ) : entitylist->get_ally_turrets( ) ) )
	{
		if ( x->is_in_auto_attack_range_native( this ) )
			return true;
	}

	return false;
}

bool game_object::is_recalling( )
{
	auto active = this->get_active_spell( );

	if ( !active )
		return false;

	auto name = active->get_spell_data( )->get_name_hash( );

	return name == spell_hash( "recall" ) || name == spell_hash( "SuperRecall" );
}

bool game_object::is_valid_target( float range, vector const& from, bool ignore_invulnerability )
{
	if ( !is_valid( ) )
		return false;

	if ( from.length( ) == 0 && ( this->get_position( ).distance( myhero->get_position( ) ) > range ) )
		return false;
	else if ( from.length( ) != 0 && this->get_position( ).distance( from ) > range )
		return false;

	if ( !is_attack_allowed_on_target( ignore_invulnerability ) )
		return false;

	if ( !is_visible( ) )
		return false;

	return true;
}

int32_t game_object::is_casting_interruptible_spell( )
{
	if ( !is_ai_hero( ) )
		return 0;

	auto champion = this->get_champion( );

	if ( champion == champion_id::Nunu && this->has_buff( buff_hash( "NunuW" ) ) )
		return 2;

	auto active = this->get_active_spell( );

	if ( !active )
		return 0;

	//High priority
	if ( champion == champion_id::Rammus && active->get_spellslot( ) == spellslot::q )
		return 2;
	if ( champion == champion_id::Warwick && active->get_spellslot( ) == (spellslot)48 )
		return 2;
	if ( champion == champion_id::Velkoz && active->get_spellslot( ) == spellslot::r )
		return 2;
	if ( champion == champion_id::Akshan && active->get_spellslot( ) == spellslot::r )
		return 2;
	if ( champion == champion_id::Xerath && active->get_spellslot( ) == spellslot::r )
		return 2;
	if ( champion == champion_id::Caitlyn && active->get_spellslot( ) == spellslot::r )
		return 2;
	if ( champion == champion_id::FiddleSticks && active->get_spellslot( ) == spellslot::r )
		return 2;
	if ( champion == champion_id::Karthus && active->get_spellslot( ) == spellslot::r )
		return 2;
	if ( champion == champion_id::Katarina && active->get_spellslot( ) == spellslot::r )
		return 2;
	if ( champion == champion_id::Lucian && active->get_spellslot( ) == spellslot::r )
		return 2;
	if ( champion == champion_id::Malzahar && active->get_spellslot( ) == spellslot::r )
		return 2;
	if ( champion == champion_id::MissFortune && active->get_spellslot( ) == spellslot::r )
		return 2;
	if ( champion == champion_id::Nunu && active->get_spellslot( ) == spellslot::r )
		return 2;
	if ( champion == champion_id::Nunu && active->get_spellslot( ) == spellslot::w )
		return 2;
	if ( champion == champion_id::Jhin && active->get_spellslot( ) == spellslot::r )
		return 2;
	if ( champion == champion_id::TwistedFate && active->get_spellslot( ) == spellslot::r )
		return 2;
	if ( champion == champion_id::Janna && active->get_spellslot( ) == spellslot::r )
		return 2;
	if ( champion == champion_id::Shen && active->get_spellslot( ) == spellslot::r )
		return 2;
	if ( champion == champion_id::Yuumi && active->get_spellslot( ) == spellslot::r )
		return 2;
	if ( champion == champion_id::Galio && active->get_spellslot( ) == spellslot::r )
		return 2;
	if ( champion == champion_id::Pantheon && active->get_spellslot( ) == spellslot::r )
		return 2;
	if ( champion == champion_id::TahmKench && active->get_spellslot( ) == spellslot::w )
		return 2;
	if ( champion == champion_id::Poppy && active->get_spellslot( ) == spellslot::r )
		return 2;
	if ( champion == champion_id::Pyke && active->get_spellslot( ) == spellslot::q )
		return 2;
	if ( champion == champion_id::Viego && active->get_spellslot( ) == spellslot::w )
		return 2;

	//Low Priority
	if ( champion == champion_id::Sion && active->get_spellslot( ) == spellslot::q )
		return 1;
	if ( champion == champion_id::Galio && active->get_spellslot( ) == spellslot::w )
		return 1;
	if ( champion == champion_id::Varus && active->get_spellslot( ) == spellslot::q )
		return 1;
	if ( champion == champion_id::Xerath && active->get_spellslot( ) == spellslot::q )
		return 1;
	if ( champion == champion_id::Zac && active->get_spellslot( ) == spellslot::e )
		return 1;
	if ( champion == champion_id::Vi && active->get_spellslot( ) == spellslot::q )
		return 1;
	if ( champion == champion_id::MasterYi && active->get_spellslot( ) == spellslot::w )
		return 1;
	if ( champion == champion_id::FiddleSticks && active->get_spellslot( ) == spellslot::w )
		return 1;
	if ( champion == champion_id::Quinn && active->get_spellslot( ) == spellslot::r )
		return 1;
	if ( champion == champion_id::Vladimir && active->get_spellslot( ) == spellslot::e )
		return 1;
	if ( champion == champion_id::Pantheon && active->get_spellslot( ) == spellslot::q )
		return 1;

	return 0;
}

vector vector::zero = vector( 0, 0, 0 );

point2::point2( int32_t x, int32_t y )
{
	this->x = x;
	this->y = y;
}

point2::point2( )
{
	this->x = 0;
	this->y = 0;
}

bool point2::operator==( const point2& vOther ) const
{
	return this->x == vOther.x && this->y == vOther.y;
}

bool point2::operator!=( const point2& vOther ) const
{
	return this->x != vOther.x || this->y != vOther.y;
}

point2 point2::operator+( const point2& v ) const
{
	return point2( this->x + v.x, this->y + v.y );
}

point2 point2::operator-( const point2& v ) const
{
	return point2( this->x - v.x, this->y - v.y );
}

point2 point2::operator*( const point2& v ) const
{
	return point2( this->x * v.x, this->y * v.y );
}

point2 point2::operator/( const point2& v ) const
{
	return point2( this->x / v.x, this->y / v.y );
}

vector::vector( )
{
	x = 0;
	y = 0;
	z = 0;
}

vector::vector( float x, float y )
{
	this->x = x;
	this->y = y;
	this->z = 0;
}

vector::vector( float x, float y, float z )
{
	this->x = x;
	this->y = y;
	this->z = z;
}

vector::vector( point2 p2 )
{
	this->x = ( float ) p2.x;
	this->y = ( float ) p2.y;
	this->z = 0;
}

bool vector::is_valid( ) const
{
	if ( this->x != this->x || this->y != this->y || this->z != this->z )
		return false;

	return this->x != 0 && this->y != 0;
}

bool vector::operator==( const vector& v_other ) const
{
	return math::NearEqual( x, v_other.x )
		&& math::NearEqual( y, v_other.y )
		&& math::NearEqual( z, v_other.z );
}

bool vector::operator!=( const vector& v_other ) const
{
	return !math::NearEqual( x, v_other.x )
		|| !math::NearEqual( y, v_other.y )
		|| !math::NearEqual( z, v_other.z );
}

float vector::length( ) const
{
	return sqrtf( this->x * this->x + this->y * this->y );
}

float vector::length_sqr( void ) const
{
	return this->x * this->x + this->y * this->y;
}

float vector::distance( const vector& vOther ) const
{
	vector delta;

	delta.x = this->x - vOther.x;
	delta.y = this->y - vOther.y;

	return delta.length( );
}

float vector::distance( game_object_script unit ) const
{
	if ( unit && unit->is_valid( ) )
	{
		vector vPosition = unit->get_position( );

		if ( unit->is_ai_hero( ) )
		{
			vPosition = unit->get_path_controller( )->get_position_on_path( );
		}

		vector delta;

		delta.x = this->x - vPosition.x;
		delta.y = this->y - vPosition.y;

		return delta.length( );
	}

	return 0.0f;
}

float vector::distance( const vector& segment_start, const vector& segment_end, bool only_if_on_segment, bool squared ) const
{
	auto const projection_info = this->project_on( segment_start, segment_end );

	if ( projection_info.is_on_segment || !only_if_on_segment )
	{
		return squared
			? this->distance_squared( projection_info.segment_point )
			: this->distance( projection_info.segment_point );
	}
	return FLT_MAX;
}

float vector::distance_squared( const vector& vOther ) const
{
	vector delta;

	delta.x = this->x - vOther.x;
	delta.y = this->y - vOther.y;

	return delta.length_sqr( );
}

float vector::dot_product( const vector& other ) const
{
	return this->x * other.x + this->y * other.y;
}

float vector::cross_product( const vector& other ) const
{
	return other.y * this->x - other.x * this->y;
}

float vector::polar( ) const
{
	if ( abs( x - 0 ) <= 1e-9 )
	{
		if ( y > 0.f )
		{
			return 90.f;
		}
		return y < 0.f ? 270.f : 0.f;
	}

	auto theta = atan( y / x ) * ( 180.0 / M_PI );

	if ( x < 0.f )
	{
		theta = theta + 180.;
	}
	if ( theta < 0. )
	{
		theta = theta + 360.;
	}
	return ( float ) theta;
}

float vector::angle_between( const vector& other ) const
{
	auto theta = polar( ) - other.polar( );
	if ( theta < 0.f )
	{
		theta = theta + 360.f;
	}
	if ( theta > 180.f )
	{
		theta = 360.f - theta;
	}
	return theta;
}

vector vector::extend( const vector& to, float range ) const
{
	const auto from = *this;
	return from + ( ( to - from ).normalized( ) * range );
}

vector vector::normalized( ) const
{
	float len = length( );

	if ( !math::IsZero( len ) )
	{
		return *this / len;
	}

	return *this;
}

vector vector::rotated( float angle ) const
{
	float c = cos( angle );
	float s = sin( angle );

	return vector( x * c - y * s, y * c + x * s );
}

vector vector::set_z( float value ) const
{
	if ( value == -1.f )
		return vector( x, y, plugin_sdk->get_nav_mesh( )->get_height_for_position( x, y ) );
	else
		return vector( x, y, value );
}

vector vector::perpendicular( ) const
{
	return vector( -y, x );
}

projection_info vector::project_on( const vector& segment_start, const vector& segment_end ) const
{
	float rs;
	auto const cx = x;
	auto const cy = y;
	auto const ax = segment_start.x;
	auto const ay = segment_start.y;
	auto const bx = segment_end.x;
	auto const by = segment_end.y;

	const auto rl = ( ( cx - ax ) * ( bx - ax ) + ( cy - ay ) * ( by - ay ) ) / ( powf( bx - ax, 2 ) + powf( by - ay, 2 ) );
	const auto point_line = vector( ax + rl * ( bx - ax ), ay + rl * ( by - ay ), 0 );

	if ( rl < 0 )
	{
		rs = 0;
	}
	else if ( rl > 1 )
	{
		rs = 1;
	}
	else
	{
		rs = rl;
	}

	auto const is_on_segment = math::NearEqual( rs, rl ); //rs == rl;
	auto const point_segment = is_on_segment ? point_line : vector( ax + rs * ( bx - ax ), ay + rs * ( by - ay ), 0 );

	return projection_info( is_on_segment, point_segment, point_line );
}

intersection_result vector::intersection( const vector& line_segment_end, const vector& line_segment2_start, const vector& line_segment2_end ) const
{
	//Real-Time Collision Detection, Christer Ericson
	//5.1 Closest-point Computations
	auto Signed2DTriArea = []( const vector& a, const vector& b, const vector& c )
	{
		return ( a.x - c.x ) * ( b.y - c.y ) - ( a.y - c.y ) * ( b.x - c.x );
	};

	intersection_result result;

	auto a = *this;
	// Sign of areas correspond to which side of ab points c and d are
	float a1 = Signed2DTriArea( a, line_segment_end, line_segment2_end ); // Compute winding of abd (+ or -)
	float a2 = Signed2DTriArea( a, line_segment_end, line_segment2_start ); // To intersect, must have sign opposite of a1

	// If c and d are on different sides of ab, areas have different signs
	if ( a1 != 0.0f && a2 != 0.0f && a1 * a2 < 0.0f )
	{
		// Compute signs for a and b with respect to segment cd
		float a3 = Signed2DTriArea( line_segment2_start, line_segment2_end, a ); // Compute winding of cda (+ or -)
		// Since area is constant a1 - a2 = a3 - a4, or a4 = a3 + a2 - a1
		// float a4 = Signed2DTriArea(c, d, b); // Must have opposite sign of a3
		float a4 = a3 + a2 - a1;
		// Points a and b on different sides of cd if areas have different signs
		if ( a3 * a4 < 0.0f )
		{
			// Segments intersect. Find intersection point along L(t) = a + t * (b - a).
			// Given height h1 of an over cd and height h2 of b over cd,
			// t = h1 / (h1 - h2) = (b*h1/2) / (b*h1/2 - b*h2/2) = a3 / (a3 - a4),
			// where b (the base of the triangles cda and cdb, i.e., the length
			// of cd) cancels out.
			auto t = a3 / ( a3 - a4 );

			result.intersects = true;
			result.point = ( *this ) + ( line_segment_end - *this ) * t;
		}
	}

	// Segments not intersecting (or collinear)

	auto isCollinear = []( vector const& p, vector const& q, vector const& r )
	{
		const float collinearRadius = 1.0f; //assume line must be equal to 1px.

		float val = ( q.y - p.y ) * ( r.x - q.x ) -
			( q.x - p.x ) * ( r.y - q.y );

		return fabs( val ) <= collinearRadius;// colinear
	};

	auto isOnSegment = []( vector const& p, vector const& q, vector const& r )
	{
		return q.x <= std::max( p.x, r.x ) && q.x >= std::min( p.x, r.x ) &&
			q.y <= std::max( p.y, r.y ) && q.y >= std::min( p.y, r.y );
	};

	if ( isCollinear( *this, line_segment_end, line_segment2_start ) && isOnSegment( *this, line_segment2_start, line_segment_end ) )
	{
		//line1_start, line2_start and line2_end are colinear and line2_start lies on segment of line1

		result.intersects = true;
		result.is_collinear = true;
	}

	if ( isCollinear( *this, line_segment_end, line_segment2_end ) && isOnSegment( *this, line_segment2_end, line_segment_end ) )
	{
		//line1_start, line1_end and line2_end are colinear and line2_end lies on segment of line1
		result.intersects = true;
		result.is_collinear = true;
	}

	if ( isCollinear( line_segment2_start, line_segment2_end, *this ) && isOnSegment( line_segment2_start, *this, line_segment2_end ) )
	{
		//line2_start, line2_end and line1_start are colinear and line1_start lies on segment of line2

		result.intersects = true;
		result.is_collinear = true;
	}

	if ( isCollinear( line_segment2_start, line_segment2_end, line_segment_end ) && isOnSegment( line_segment2_start, line_segment_end, line_segment2_end ) )
	{
		//line2_start, line2_end and line1_end are colinear and line1_end lies on segment of line2

		result.intersects = true;
		result.is_collinear = true;
	}

	return result;
}

vector& vector::operator=( const vector& vOther )
{
	x = vOther.x; y = vOther.y; z = vOther.z;
	return *this;
}

vector vector::operator-( ) const
{
	return vector( -x, -y, -z );
}

vector vector::operator+( const vector& v ) const
{
	return vector( this->x + v.x, this->y + v.y, this->z );
}

vector vector::operator-( const vector& v ) const
{
	return vector( this->x - v.x, this->y - v.y, this->z );
}

vector vector::operator*( const vector& v ) const
{
	return vector( this->x * v.x, this->y * v.y, this->z );
}

vector vector::operator/( const vector& v ) const
{
	return vector( this->x / v.x, this->y / v.y, this->z );
}

vector vector::operator*( float fl ) const
{
	return vector( this->x * fl, this->y * fl, this->z );
}

vector vector::operator/( float fl ) const
{
	return vector( this->x / fl, this->y / fl, this->z );
}

bool vector::is_in_fow( ) const
{
	return navmesh->is_in_fow( *this );;
}

bool vector::is_wall( ) const
{
	return static_cast< int >( navmesh->get_collision_flag( *this ) & nav_collision_flags::wall ) != 0;
}

bool vector::is_wall_of_grass( ) const
{
	return static_cast< int >( navmesh->get_collision_flag( *this ) & nav_collision_flags::grass ) != 0;
}

bool vector::is_building( ) const
{
	return static_cast< int >( navmesh->get_collision_flag( *this ) & nav_collision_flags::building ) != 0;
}

bool vector::is_on_screen( ) const
{
	return this->x <= renderer->screen_width( ) && this->x >= 0 && this->y <= renderer->screen_height( ) && this->y >= 0;
}

bool vector::is_under_ally_turret( ) const
{
	for ( auto&& t : entitylist->get_ally_turrets( ) )
	{
		if ( t->is_valid( ) && !t->is_dead( ) && this->distance_squared( t->get_position( ) ) < ( 775.f + t->get_bounding_radius( ) ) * ( 775.f + t->get_bounding_radius( ) ) )
			return true;
	}
	return false;
}

bool vector::is_under_enemy_turret( ) const
{
	for ( auto&& t : entitylist->get_enemy_turrets( ) )
	{
		if ( t->is_valid( ) && !t->is_dead( ) && this->distance_squared( t->get_position( ) ) < ( 775.f + t->get_bounding_radius( ) ) * ( 775.f + t->get_bounding_radius( ) ) )
			return true;
	}
	return false;
}

int vector::count_enemies_in_range( float range ) const
{
	int count = 0;
	for ( auto&& t : entitylist->get_enemy_heroes( ) )
	{
		if ( t->is_valid_target( range, *this ) )
			count++;
	}
	return count;
}

int vector::count_allys_in_range( float range, game_object_script original_unit ) const
{
	int count = 0;
	std::uint32_t network_id = ( ( original_unit != nullptr && original_unit->is_valid( ) ) ? original_unit->get_network_id( ) : 0 );

	for ( auto&& t : entitylist->get_ally_heroes( ) )
	{
		if ( t->is_valid( ) && !t->is_dead( ) && t->get_network_id( ) != network_id && this->distance( t->get_position( ) ) < range )
			count++;
	}
	return count;
}

projection_info::
projection_info( const bool is_on_segment, vector const& segment_point, vector const& line_point ):
	is_on_segment( is_on_segment ), line_point( line_point ), segment_point( segment_point )
{
}

intersection_result::
intersection_result( const bool intersects, vector const& point ) :
	intersects( intersects ), point( point )
{
}

prediction_input::prediction_input( )
{
	unit = myhero;
}

vector prediction_input::get_from( )
{
	return _from.is_valid( ) ? _from : myhero->get_position( );
}

vector prediction_input::get_range_check_from( )
{
	return _range_check_from.is_valid( )
		? _range_check_from
		: ( _from.is_valid( ) ? _from : myhero->get_position( ) );
}

float prediction_input::get_real_radius( )
{
	return use_bounding_radius ? radius + unit->get_bounding_radius( ) : radius;
}

vector prediction_output::get_cast_position( )
{
	return _cast_position.is_valid( ) ? _cast_position.set_z( ) : ( input.unit ? input.unit->get_position( ) : vector( 0, 0, 0 ) );
}

vector prediction_output::get_unit_position( )
{
	return _unit_position.is_valid( ) ? _unit_position.set_z( ) : ( input.unit ? input.unit->get_position( ) : vector( 0, 0, 0 ) );
}

int prediction_output::aoe_targets_hit_count( )
{
	return std::max( _aoe_targets_hit_count, ( int32_t ) aoe_targets_hit.size( ) );
}

namespace antigapcloser
{
	#define ADD_DASH_DATA_VAR(TYPE, NAME) TYPE NAME = {}; TrackedDashData& set_##NAME( const TYPE& NAME ) { this->NAME = NAME; return *this; }

	struct TrackedDashData
	{
		ADD_DASH_DATA_VAR( std::string, name )
		ADD_DASH_DATA_VAR( std::string, spell_name )

		ADD_DASH_DATA_VAR( std::uint32_t, required_buffhash )
		ADD_DASH_DATA_VAR( std::uint32_t, spell_name_hash )

		ADD_DASH_DATA_VAR( bool, wait_for_new_path )
		ADD_DASH_DATA_VAR( bool, is_dangerous )
		ADD_DASH_DATA_VAR( bool, is_fixed_range )
		ADD_DASH_DATA_VAR( bool, is_targeted )
		ADD_DASH_DATA_VAR( bool, is_inverted )
		ADD_DASH_DATA_VAR( bool, find_target_by_buffhash )
		ADD_DASH_DATA_VAR( bool, wait_for_targetable )
		ADD_DASH_DATA_VAR( bool, is_cc )
		ADD_DASH_DATA_VAR( bool, is_unstoppable )

		ADD_DASH_DATA_VAR( float, delay )
		ADD_DASH_DATA_VAR( float, speed )
		ADD_DASH_DATA_VAR( float, range )
		ADD_DASH_DATA_VAR( float, min_range )
		ADD_DASH_DATA_VAR( float, extra_range )
		ADD_DASH_DATA_VAR( float, add_ms_ratio )
		ADD_DASH_DATA_VAR( float, always_fixed_delay )

		TrackedDashData( )
		{
			this->speed = FLT_MAX;
			this->spell_name_hash = spell_hash_real( spell_name.c_str( ) );
		}
	};

	struct TrackedDash
	{
		game_object_script sender;
		game_object_script target;

		const TrackedDashData* dash_data;

		float start_time;
		float end_time;
		float speed;

		vector start_position;
		vector end_position;

		bool is_finished_detecting;

		TrackedDash( )
		{
			this->sender = nullptr;
			this->target = nullptr;

			this->dash_data = nullptr;

			this->start_time = 0;
			this->end_time = 0;
			this->speed = 0;

			this->is_finished_detecting = false;
		}
	};
	
	std::vector< TrackedDash > detected_dashes;
	std::vector< TrackedDashData > dashes_data;
	
	std::vector<void*> p_handlers;

	TrackedDashData& add_dash( const std::string& spell_name, float range, float speed )
	{
		TrackedDashData data;
		data.spell_name = spell_name;
		data.range = range;
		data.speed = speed;

		dashes_data.push_back( data );

		return dashes_data[ dashes_data.size( ) - 1 ];
	}

	void OnProcessSpellCast( game_object_script sender, spell_instance_script spell )
	{
		if ( sender->is_enemy( ) )
		{
			auto name = spell->get_spell_data( )->get_name( );
			auto it = std::find_if( dashes_data.begin( ), dashes_data.end( ), [&name]( const TrackedDashData& x ) { return x.spell_name == name; } );

			if ( it != dashes_data.end( ) )
			{
				game_object_script target = spell->get_last_target_id( ) != 0 && it->is_targeted
					? entitylist->get_object( spell->get_last_target_id( ) ) 
					: nullptr;

				if ( it->find_target_by_buffhash )
				{
					for ( auto&& t : sender->is_ally( ) ? entitylist->get_enemy_heroes( ) : entitylist->get_ally_heroes( ) )
					{
						if ( t->is_valid_target( ) && t->has_buff( it->required_buffhash ) )
						{
							target = t;
							break;
						}
					}
				}

				if ( it->is_targeted && target == nullptr )
				{
					return;
				}

				if ( target && it->required_buffhash && !target->has_buff( it->required_buffhash ) )
				{
					return;
				}

				auto start = spell->get_start_position( );
				auto end = spell->get_end_position( );

				if ( it->min_range > 0 && start.distance_squared( end ) < std::powf( it->min_range, 2 ) )
				{
					end = start.extend( end, it->min_range );
				}

				if ( it->is_fixed_range || start.distance_squared( end ) > std::powf( it->range, 2 ) )
				{
					end = start.extend( end, it->range );
				}

				if ( it->is_inverted )
				{
					end = start - ( end - start );
				}

				if ( target && !it->is_fixed_range )
				{
					end = target->get_position( );
				}

				if ( it->extra_range > 0 )
				{
					end = end.extend( start, -it->extra_range );
				}

				TrackedDash new_dash;
				new_dash.sender = sender;
				new_dash.target = target;
				new_dash.dash_data = it._Ptr;
				new_dash.start_position = start;
				new_dash.end_position = end;
				new_dash.speed = it->speed + sender->get_move_speed( ) * it->add_ms_ratio;

				if ( it->always_fixed_delay > 0 )
					new_dash.speed = new_dash.start_position.distance( new_dash.end_position ) / it->always_fixed_delay;
		
				new_dash.start_time = gametime->get_time( );
				new_dash.end_time = new_dash.start_time + it->delay + start.distance( end ) / new_dash.speed;

				if ( it->wait_for_targetable )
					new_dash.end_time = new_dash.start_time + 2.5f;

				new_dash.is_finished_detecting = !it->wait_for_new_path && !it->wait_for_targetable;

				detected_dashes.push_back( new_dash );
			}
		}
	}

	void OnNewPath( game_object_script sender, const std::vector<vector>& path, bool is_dash, float dash_speed )
	{
		if ( is_dash )
		{
			float length = path.size( ) > 1 ? geometry::geometry::path_length( path ) : 0;

			for ( TrackedDash& dash : detected_dashes )
			{
				if ( dash.is_finished_detecting || !dash.dash_data->wait_for_new_path || sender != dash.sender ) continue;

				dash.start_time = gametime->get_time( ) - dash.dash_data->delay;
				dash.end_time = gametime->get_time( ) + length / dash_speed;
				dash.start_position = path.front( );
				dash.end_position = path.back( );
				dash.speed = dash_speed;
				dash.is_finished_detecting = true;
			}
		}
	}
	
	void OnUpdate( )
	{
		detected_dashes.erase( std::remove_if( detected_dashes.begin( ), detected_dashes.end( ), []( const TrackedDash& dash )
		{
			return gametime->get_time( ) >= dash.end_time;
		} ), detected_dashes.end( ) );

		for ( TrackedDash& dash : detected_dashes )
		{
			if ( dash.is_finished_detecting || !dash.dash_data->wait_for_targetable ) continue;
			if ( gametime->get_time( ) - dash.start_time < 0.15f ) continue;

			if ( dash.sender->is_targetable( ) )
			{
				if ( dash.sender->get_distance( myhero ) > 150 )
				{
					dash.end_time = gametime->get_time( ) - 0.1f; //delete Elise E dash, it was not casted on me
					continue;
				}

				dash.end_position = dash.sender->get_position( );

				if ( dash.dash_data->always_fixed_delay > 0 )
					dash.speed = dash.start_position.distance( dash.end_position ) / dash.dash_data->always_fixed_delay;

				dash.start_time = gametime->get_time( );
				dash.end_time = dash.start_time + dash.dash_data->always_fixed_delay;
				dash.is_finished_detecting = true;
			}
		}
		
		for ( const TrackedDash& dash : detected_dashes )
		{
			if ( !dash.is_finished_detecting || !dash.sender->is_valid_target( ) ) continue;
			if ( ( dash.target == nullptr || !dash.target->is_me( ) ) && dash.sender->get_distance( myhero ) > 500 ) continue;

			antigapcloser_args args;
			args.type = gapcloser_type::skillshot;
			args.target = dash.target;
			args.start_time = dash.start_time;
			args.end_time = dash.end_time;
			args.speed = dash.speed;
			args.start_position = dash.start_position;
			args.end_position = dash.end_position;
			args.is_unstoppable = dash.dash_data->is_unstoppable;
			args.is_cc = dash.dash_data->is_cc;
						
			if ( !dash.dash_data->name.empty( ) )
				args.type = gapcloser_type::item;
			
			if ( dash.target != nullptr && dash.target->is_me( ) )
				args.type = gapcloser_type::targeted;
			
			for ( auto const& callback : p_handlers )
			{
				if ( callback != nullptr )
				{
					reinterpret_cast< gapcloser_handler >( callback )( dash.sender, &args );
				}
			}
		}
	}

	void add_event_handler( gapcloser_handler p_handler )
	{
		auto it = std::find( p_handlers.begin( ), p_handlers.end( ), ( void* ) p_handler );

		if ( it == p_handlers.end( ) )
		{
			p_handlers.push_back( ( void* ) p_handler );
		}

		if ( p_handlers.size( ) == 1 )
		{
			add_dash( "3152Active", 300.f, 1150.f ).set_name( "Hextech Rocketbelt" ).set_is_fixed_range( true );
			add_dash( "6671Cast", 425.f, 1350.f ).set_name( "Prowler's Claw" ).set_min_range( 200.f );
			add_dash( "6693Active", 500.f, 2000.f ).set_name( "Galeforce" ).set_is_targeted( true ).set_delay( 0.2f );

			for ( auto& hero : entitylist->get_enemy_heroes( ) )
			{
				switch ( hero->get_champion( ) )
				{
					case champion_id::Aatrox:
						add_dash( "AatroxE", 300.f, 800.f ).set_wait_for_new_path( true );
						break;
					case champion_id::Ahri:
						add_dash( "AhriTumble", 500.f, 1200.f ).set_is_fixed_range( true ).set_add_ms_ratio( 1.f );
						break;
					case champion_id::Akali:
						add_dash( "AkaliE", 350.f, 1400.f ).set_is_fixed_range( true ).set_is_inverted( true ).set_delay( 0.2f );
						add_dash( "AkaliEb", FLT_MAX, 1700.f ).set_is_targeted( true ).set_required_buffhash( buff_hash("AkaliEMis") ).set_find_target_by_buffhash( true );
						add_dash( "AkaliR", 750.f, 1500.f ).set_is_fixed_range( true );
						add_dash( "AkaliRb", 800.f, 3000.f ).set_is_fixed_range( true );
						break;
					case champion_id::Alistar:
						add_dash( "Headbutt", 650.f, 1500.f ).set_is_targeted( true ).set_is_cc( true );
						break;
					case champion_id::Caitlyn:
						add_dash( "CaitlynE", 390.f, 1000.f ).set_is_fixed_range( true ).set_is_inverted( true ).set_delay( 0.15f );
						break;
					case champion_id::Camille:
						add_dash( "CamilleEDash2", 800.f, 1050.f ).set_is_cc( true ).set_add_ms_ratio( 1.f );
						break;
					case champion_id::Corki:
						add_dash( "CarpetBomb", 600.f, 650.f ).set_min_range( 300.f ).set_add_ms_ratio( 1.f );
						break;
					case champion_id::Diana:
						add_dash( "DianaTeleport", 825.f, 2500.f ).set_is_targeted( true );
						break;
					case champion_id::Ekko:
						add_dash( "EkkoEAttack", 600.f, 2000.f ).set_is_targeted( true ).set_delay( 0.1f );
						break;
					case champion_id::Elise:
						add_dash( "EliseSpiderQCast", 475.f, 1200.f ).set_is_targeted( true );
						add_dash( "EliseSpiderE", 700.f, 1000.f ).set_is_targeted( true ).set_wait_for_targetable( true ).set_always_fixed_delay( 0.5f );
						break;
					case champion_id::Evelynn:
						add_dash( "EvelynnE2", 400.f, 1900.f ).set_is_targeted( true );
						break;
					case champion_id::Fiora:
						add_dash( "FioraQ", 450.f, 500.f ).set_add_ms_ratio( 2.f );
						break;
					case champion_id::Fizz:
						add_dash( "FizzQ", 550.f, 1400.f ).set_is_fixed_range( true ).set_is_targeted( true );
						break;
					case champion_id::Galio:
						add_dash( "GalioE", 650.f, 2300.f ).set_is_cc( true ).set_delay( 0.4f );
						break;
					case champion_id::Gnar:
						add_dash( "GnarE", 475.f, 900.f );
						add_dash( "GnarBigE", 675.f, 1165.f );
						break;
					case champion_id::Gragas:
						add_dash( "GragasE", 600.f, 900.f ).set_is_cc( true ).set_is_fixed_range( true );
						break;
					case champion_id::Graves:
						add_dash( "GravesMove", 375.f, 1150.f ).set_wait_for_new_path( true );
						break;
					case champion_id::Gwen:
						add_dash( "GwenE", 350.f, 1050.f ).set_add_ms_ratio( 1.f );
						break;
					case champion_id::Hecarim:
						add_dash( "HecarimRampAttack", 900.f, 1200.f ).set_is_cc( true ).set_is_targeted( true );
						add_dash( "HecarimUlt", 1000.f, 1100.f ).set_is_cc( true ).set_is_unstoppable( true ).set_min_range( 300.f );
						break;
					case champion_id::Illaoi:
						add_dash( "IllaoiWAttack", 300.f, 800.f ).set_is_targeted( true );
						break;
					case champion_id::Irelia:
						add_dash( "IreliaQ", 600.f, 1400.f ).set_is_targeted( true ).set_add_ms_ratio( 1.f );
						break;
					case champion_id::JarvanIV:
						add_dash( "JarvanIVDragonStrike", 850.f, 2000.f ).set_is_cc( true ).set_wait_for_new_path( true ).set_delay( 0.4f );
						break;
					case champion_id::Jax:
						add_dash( "JaxLeapStrike", 700.f, 1600.f ).set_is_targeted( true );
						break;
					case champion_id::Jayce:
						add_dash( "JayceToTheSkies", 600.f, 1000.f ).set_is_targeted( true );
						break;
					case champion_id::Kaisa:
						add_dash( "KaisaR", 3000.f, 3700.f );
						break;
					case champion_id::Kayn:
						add_dash( "KaynQ", 350.f, 1150.f );
						add_dash( "KaynRJumpOut", 500.f, 1200.f ).set_wait_for_new_path( true ).set_delay( 3.f );
						break;
					case champion_id::Khazix:
						add_dash( "KhazixE", 700.f, 1250.f );
						add_dash( "KhazixELong", 850.f, 1250.f );
						break;
					case champion_id::Kindred:
						add_dash( "KindredQ", 300.f, 500.f ).set_add_ms_ratio( 1.f );
						break;
					case champion_id::Kled:
						add_dash( "KledRiderQ", 300.f, 1000.f ).set_is_inverted( true ).set_delay( 0.25f );
						add_dash( "KledEDash", 700.f, 600.f ).set_add_ms_ratio( 1.f );
						break;
					case champion_id::Leblanc:
						add_dash( "LeblancW", 600.f, 1450.f );
						add_dash( "LeblancRW", 600.f, 1450.f );
						break;
					case champion_id::LeeSin:
						add_dash( "BlindMonkQTwo", 2000.f, 2000.f ).set_is_targeted( true ).set_required_buffhash( buff_hash( "BlindMonkQOne" ) ).set_find_target_by_buffhash( true );
						add_dash( "BlindMonkWOne", 700.f, 1350.f ).set_is_targeted( true ).set_add_ms_ratio( 1.f );
						break;
					case champion_id::Leona:
						add_dash( "LeonaZenithBlade", 900.f, 1300.f ).set_is_cc( true ).set_always_fixed_delay( 0.75f );
						break;
					case champion_id::Lillia:
						add_dash( "LilliaW", 500.f, 1000.f ).set_always_fixed_delay( 0.8f );
						break;
					case champion_id::Lucian:
						add_dash( "LucianE", 475.f, 1350.f ).set_wait_for_new_path( true );
						break;
					case champion_id::Malphite:
						add_dash( "UFSlash", 1000.f, 1500.f ).set_is_cc( true ).set_is_unstoppable( true ).set_add_ms_ratio( 1.f );
						break;
					case champion_id::Maokai:
						add_dash( "MaokaiW", 525.f, 1300.f ).set_is_cc( true ).set_is_unstoppable( true ).set_is_targeted( true );
						break;
					case champion_id::MonkeyKing:
						add_dash( "MonkeyKingNimbus", 625.f, 1050.f ).set_is_targeted( true ).set_add_ms_ratio( 1.f );
						break;
					case champion_id::Nidalee:
						add_dash( "Pounce", 750.f, 950.f ).set_wait_for_new_path( true ).set_min_range( 350.f );
						break;
					case champion_id::Ornn:
						add_dash( "OrnnE", 650.f, 1600.f ).set_is_cc( true ).set_is_fixed_range( true ).set_delay( 0.35f );
						break;
					case champion_id::Pantheon:
						add_dash( "PantheonW", 600.f, 1100.f ).set_is_cc( true ).set_is_targeted( true );
						break;
					case champion_id::Poppy:
						add_dash( "PoppyE", 475.f, 1800.f ).set_is_cc( true ).set_is_targeted( true );
						break;
					case champion_id::Pyke:
						add_dash( "PykeE", 550.f, 2000.f ).set_is_cc( true ).set_is_fixed_range( true );
						break;
					case champion_id::Qiyana:
						add_dash( "QiyanaE", 550.f, 1100.f ).set_is_fixed_range( true ).set_is_targeted( true ).set_add_ms_ratio( 1.f );
						break;
					case champion_id::Rakan:
						add_dash( "RakanW", 650.f, 1700.f ).set_is_cc( true ).set_always_fixed_delay( 0.85f );
						break;
					case champion_id::Rammus:
						add_dash( "Tremors2", 1500.f, 1000.f ).set_is_unstoppable( true ).set_always_fixed_delay( 0.85f );
						break;
					case champion_id::RekSai:
						add_dash( "RekSaiEBurrowed", 800.f, 800.f ).set_is_fixed_range( true );
						break;
					case champion_id::Rell:
						add_dash( "RellW_Dismount", 500.f, 1000.f ).set_is_cc( true ).set_always_fixed_delay( 0.85f );
						break;
					case champion_id::Renekton:
						add_dash( "RenektonDice", 450.f, 760.f ).set_is_fixed_range( true ).set_add_ms_ratio( 1.f );
						add_dash( "RenektonSliceAndDice", 450.f, 760.f ).set_is_fixed_range( true ).set_add_ms_ratio( 1.f );
						break;
					case champion_id::Riven:
						add_dash( "RivenFeint", 250.f, 1200.f ).set_is_fixed_range( true );
						break;
					case champion_id::Samira:
						add_dash( "SamiraE", 650.f, 1600.f ).set_is_fixed_range( true ).set_is_targeted( true );
						break;
					case champion_id::Sejuani:
						add_dash( "SejuaniQ", 650.f, 1000.f ).set_is_cc( true );
						break;
					case champion_id::Shen:
						add_dash( "ShenE", 600.f, 800.f ).set_is_cc( true ).set_min_range( 300.f ).set_add_ms_ratio( 1.f );
						break;
					case champion_id::Shyvana:
						add_dash( "ShyvanaTransformLeap", 950.f, 1100.f ).set_is_unstoppable( true );
						break;
					case champion_id::Sylas:
						add_dash( "SylasW", 400.f, 1450.f ).set_is_targeted( true );
						add_dash( "SylasE2", 800.f, 1950.f ).set_is_cc( true );
						add_dash( "SylasE", 400.f, 1450.f );
						break;
					case champion_id::Talon:
						add_dash( "TalonQ", 575.f, 1600.f ).set_is_targeted( true );
						break;
					case champion_id::Tristana:
						add_dash( "TristanaW", 900.f, 1100.f ).set_delay( 0.25f );
						break;
					case champion_id::Tryndamere:
						add_dash( "TryndamereE", 660.f, 900.f );
						break;
					case champion_id::Urgot:
						add_dash( "UrgotE", 450.f, 1200.f ).set_is_cc( true ).set_is_fixed_range( true ).set_delay( 0.45f ).set_add_ms_ratio( 1.f );
						break;
					case champion_id::Vayne:
						add_dash( "VayneTumble", 300.f, 500.f ).set_is_fixed_range( true ).set_add_ms_ratio( 1.f );
						break;
					case champion_id::Vi:
						add_dash( "ViQ", 725.f, 1400.f ).set_is_cc( true ).set_wait_for_new_path( true );
						break;
					case champion_id::Viego:
						add_dash( "ViegoR", 500.f, 1000.f ).set_is_unstoppable( true ).set_always_fixed_delay( 0.6f );
						add_dash( "ViegoW", 300.f, 1000.f ).set_is_cc( true ).set_is_fixed_range( true );
						break;
					case champion_id::Volibear:
						add_dash( "VolibearR", 700.f, 1000.f ).set_is_unstoppable( true ).set_always_fixed_delay( 1.f );
						break;
					case champion_id::XinZhao:
						add_dash( "XinZhaoEDash", 1100.f, 2500.f ).set_is_targeted( true );
						break;
					case champion_id::Yasuo:
						add_dash( "YasuoEDash", 475.f, 750.f ).set_is_fixed_range( true ).set_is_targeted( true ).set_add_ms_ratio( 0.875f );
						break;
					case champion_id::Yone:
						add_dash( "YoneE", 300.f, 1200.f ).set_is_fixed_range( true );
						break;
					case champion_id::Zac:
						add_dash( "ZacE", 1800.f, 1000.f ).set_wait_for_new_path( true );
						break;
					case champion_id::Zed:
						add_dash( "ZedR", 625.f, 1000.f ).set_is_targeted( true ).set_always_fixed_delay( 1.6f );
						break;
					case champion_id::Zeri:
						add_dash( "ZeriE", 2000.f, 600.f ).set_wait_for_new_path( true ).set_add_ms_ratio( 1.f );
						break;
				}
			}
			
			event_handler< events::on_new_path >::add_callback( OnNewPath );
			event_handler< events::on_process_spell_cast >::add_callback( OnProcessSpellCast );
			event_handler< events::on_update >::add_callback( OnUpdate );
		}
	}

	void remove_event_handler( gapcloser_handler p_handler )
	{
		auto it = std::find( p_handlers.begin( ), p_handlers.end( ), p_handler );

		if ( it != p_handlers.end( ) )
		{
			p_handlers.erase( it );
		}

		if ( p_handlers.empty( ) )
		{
			event_handler< events::on_new_path >::remove_handler( OnNewPath );
			event_handler< events::on_process_spell_cast >::remove_handler( OnProcessSpellCast );
			event_handler< events::on_update >::remove_handler( OnUpdate );
		}
	}
}

void locked_target_selector::unlock_target( )
{
	_last_target_id = 0;
	_last_target_network_id = 0;
}

game_object_script locked_target_selector::get_target( script_spell* spell, damage_type damage_type )
{
	auto last_target = get_last_target( );

	if ( last_target == nullptr || !last_target->is_valid_target( ) || _last_damage_type != damage_type )
	{
		auto newTarget = target_selector->get_target( spell->range( ), damage_type );

		if ( newTarget != nullptr )
		{
			_last_target_id = newTarget->get_id( );
			_last_target_network_id = newTarget->get_network_id( );
			_last_damage_type = damage_type;
		}

		return newTarget;
	}

	if ( last_target->is_valid_target( spell->range( ) ) && damage_type == _last_damage_type )
	{
		return last_target;
	}

	auto newTarget2 = target_selector->get_target( spell->range( ), damage_type );

	if ( newTarget2 != nullptr )
	{
		_last_target_id = newTarget2->get_id( );
		_last_target_network_id = newTarget2->get_network_id( );
		_last_damage_type = damage_type;
	}

	return newTarget2;
}

game_object_script locked_target_selector::get_target( float range, damage_type damage_type )
{
	auto last_target = get_last_target( );

	if ( last_target == nullptr || !last_target->is_valid_target( ) || _last_damage_type != damage_type )
	{
		auto newTarget = target_selector->get_target( range, damage_type );

		if ( newTarget != nullptr )
		{
			_last_target_id = newTarget->get_id( );
			_last_target_network_id = newTarget->get_network_id( );
			_last_damage_type = damage_type;
		}

		return newTarget;
	}

	if ( last_target->is_valid_target( range ) && damage_type == _last_damage_type )
	{
		return last_target;
	}

	auto newTarget2 = target_selector->get_target( range, damage_type );

	if ( newTarget2 != nullptr )
	{
		_last_target_id = newTarget2->get_id( );
		_last_target_network_id = newTarget2->get_network_id( );
		_last_damage_type = damage_type;
	}

	return newTarget2;
}

game_object_script locked_target_selector::get_last_target( )
{
	if ( _last_target_id != 0 )
	{
		auto unit = entitylist->get_object( _last_target_id );

		if ( unit != nullptr && unit->is_valid( ) && unit->get_network_id( ) == _last_target_network_id )
		{
			return unit;
		}
	}

	return nullptr;
}

script_spell::script_spell( ): _range( FLT_MAX ), slot( spellslot::invalid )
{

}

script_spell::script_spell( spellslot slot ) : _range( FLT_MAX )
{
	this->slot = slot;
}

script_spell::script_spell( spellslot slot, float range )
{
	this->slot = slot;
	this->_range = range;
}
script_spell::script_spell( spellslot slot, float range, skillshot_type type, float delay, float speed, float radius, bool collision )
{
	this->slot = slot;
	this->_range = range;
	this->type = type;
	this->delay = delay;
	this->speed = speed;
	this->radius = radius;
	this->collision = collision;
	this->from = vector( );
}

float script_spell::get_damage( game_object_script target, int stage )
{
	return damagelib->get_spell_damage( myhero, target, this->slot, false );
}

bool script_spell::is_ready( float time )
{
	auto spellInfo = myhero->get_spell( this->slot );

	if ( spellInfo && spellInfo->is_learned( ) )
	{
		auto spell_state = myhero->get_spell_state( this->slot );

		if ( spell_state == 64 )
		{
			if ( is_charged_spell )
			{
				if ( gametime->get_time( ) - this->charging_started_time < 0.3f )
				{
					return true;
				}

				auto buff = get_charge_buff( );

				return buff != nullptr && buff->is_valid( ) && buff->is_alive( );
			}
		}

		if ( spell_state == 2 )
		{
			return true;
		}

		if ( spell_state & spell_state::Cooldown )
		{
			if ( time > 0.f )
			{
				//cooldown expires in seconds
				return spellInfo->cooldown_start( ) - gametime->get_time( ) <= time;
			}
		}
	}

	return false;
}

int8_t script_spell::icon_index( )
{
	auto spellInfo = myhero->get_spell( this->slot );

	if ( !spellInfo || !spellInfo->is_learned( ) )
		return 0;

	return spellInfo->get_icon_index( );
}

int script_spell::toogle_state( )
{
	auto spellInfo = myhero->get_spell( this->slot );

	if ( !spellInfo || !spellInfo->is_learned( ) )
		return 0;

	return spellInfo->toogle_state( );
}

void script_spell::update_chargeable_spell_handle( spellslot slot, bool release_cast )
{
}

spell_data_inst_script script_spell::handle( )
{
	return myhero->get_spell( this->slot );
}

std::string script_spell::name( )
{
	return myhero->get_spell( this->slot )->get_name( );
}

uint32_t script_spell::name_hash( )
{
	return myhero->get_spell( this->slot )->get_name_hash( );
}

float script_spell::range( )
{
	if ( !this->is_charged_spell )
	{
		return this->_range;
	}

	if ( this->is_charging( ) )
	{
		return this->charged_min_range
			+ fminf( ( float ) ( this->charged_max_range - this->charged_min_range ), ( float ) ( this->charged_max_range - this->charged_min_range ) * this->charged_percentage( ) );
	}

	return ( float ) this->charged_max_range;
}

float script_spell::mana_cost( )
{
	return myhero->get_mana_for_spell( this->slot );
}

float script_spell::charged_percentage( )
{
    auto buff = this->get_charge_buff( );

    if ( buff != nullptr && buff->is_valid( ) && buff->is_alive( ) )
    {
        return ( fmaxf( 0.f, fminf( 1.f, ( gametime->get_time( ) - buff->get_start( ) + 0.25f - ( buff->get_hash_name( ) == buff_hash( "PykeQ" ) ? 0.4f : 0.f ) - ( buff->get_hash_name( ) == buff_hash( "PoppyR" ) ? 0.5f : 0.f ) - (buff->get_hash_name() == buff_hash("SionQ") ? 0.25f : 0.f)) / this->charge_duration ) ) );
    }

    return fmaxf( 0.f, fminf( 1.f, this->is_charging( ) ? ( gametime->get_time( ) - this->charging_started_time ) / this->charge_duration  : 0.f ) );
}

bool script_spell::cast( )
{
	if ( gametime->get_time( ) < last_cast_spell + sciprt_spell_wait )
		return false;

	myhero->cast_spell( this->slot, true, is_charged_spell );

	last_cast_spell = gametime->get_time( );
	return true;
}

int script_spell::level( )
{
	return myhero->get_spell( this->slot )->level( );
}

int script_spell::ammo( )
{
	return myhero->get_spell( this->slot )->ammo( );
}

float script_spell::cooldown_time( )
{
	return myhero->get_spell( this->slot )->cooldown( );
}

void script_spell::set_spell_lock( bool value )
{
	if ( slot >= spellslot::q && slot <= spellslot::r ) 
		enabled_spellslots[ slot ] = value;
	
	is_spell_lock_enable = value;
}

bool script_spell::is_spell_locked( )
{
	if ( !is_spell_lock_enable )
		return false;

	auto active_spell = myhero->get_active_spell( );
	return active_spell != nullptr && !active_spell->is_auto_attack( ) && active_spell->is_winding_up( );
}

vector script_spell::get_cast_on_best_farm_position( int minMinions, bool is_jugnle_mobs )
{
	std::vector<vector> minionPositions;

	auto from_pos = from.is_valid( ) ? from : myhero->get_position( );

	for ( auto minion : ( is_jugnle_mobs ? entitylist->get_jugnle_mobs_minions( ) : entitylist->get_enemy_minions( ) ) )
	{
		if ( minion->get_position( ).distance( from_pos ) > this->range( ) )
			continue;

		auto input = prediction_input( );
		input.unit = minion;
		input.delay = this->delay;
		input._from = from_pos;
		input._range_check_from = from_pos;
		input.type = this->type;
		input.speed = this->speed;
		input.spell_slot = this->slot;

		minionPositions.push_back( prediction->get_prediction( &input ).get_unit_position( ) );
	}
	if ( minionPositions.size( ) == 1 )
	{
		if ( 1 >= minMinions )
		{
			return minionPositions.front( );
		}
		else
			return vector( 0, 0, 0 );
	}

	if ( this->type == skillshot_type::skillshot_circle )
	{
		auto bestPos = vector( );
		auto count = 0;
		auto startPos = myhero->get_position( );
		auto width = this->radius + 30 + 30;
		auto range = this->range( );

		if ( minionPositions.size( ) > 0 )
		{
			if ( minionPositions.size( ) > 1 )
			{
				for ( auto&& pos : minionPositions )
				{
					auto points = geometry::geometry::circle_points( pos, width / 2, 6 );

					for ( auto&& point : points )
					{
						if ( point.distance( startPos ) <= range )
						{
							auto countPosition = 0;
							for ( auto&& pos2 : minionPositions )
							{
								if ( point.distance( pos2 ) <= width )
								{
									countPosition++;
								}
							}
							if ( countPosition > count )
							{
								bestPos = point;
								count = countPosition;
							}
						}
					}
				}
			}
			else
			{
				bestPos = minionPositions.front( );
				count = 1;
			}
		}

		if ( count >= minMinions )
		{
			return bestPos;
		}
		else
			return vector( 0, 0, 0 );
	}

	if ( this->type == skillshot_type::skillshot_line )
	{
		auto startPos = myhero->get_position( );
		auto range = this->range( );
		auto result = vector( );
		auto minionCount = 0;
		auto width = this->radius + 30 + 30;
		std::vector<vector> posiblePositions;

		posiblePositions.insert( std::end( posiblePositions ), std::begin( minionPositions ), std::end( minionPositions ) );

		auto max = minionPositions.size( );
		for ( auto i = 0u; i < max; i++ )
		{
			for ( auto j = 0u; j < max; j++ )
			{
				if ( minionPositions[ j ] != minionPositions[ i ] )
				{
					posiblePositions.push_back( ( minionPositions[ j ] + minionPositions[ i ] ) / 2 );
				}
			}
		}

		for ( auto&& pos : posiblePositions )
		{
			if ( pos.distance( startPos ) <= range * range )
			{
				auto temp = ( pos - startPos ).normalized( );
				temp = temp * range;
				auto endPos = startPos + temp;
				auto count = 0;

				for ( auto&& pos2 : minionPositions )
				{
					if ( pos2.distance( startPos, endPos, true, true ) <= width * width )
						++count;
				}

				if ( count >= minionCount )
				{
					result = endPos;
					minionCount = count;
				}
			}
		}


		if ( minionCount >= minMinions )
		{
			return result;
		}
		else
			return vector( 0, 0, 0 );
	}

	return vector( 0, 0, 0 );
}

void script_spell::set_damage_type( damage_type type )
{
	this->_damage_type = type;
}

damage_type script_spell::get_damage_type( )
{
	return this->_damage_type;
}

spellslot script_spell::get_slot( )
{
	return slot;
}

float script_spell::get_speed( )
{
	return speed;
}

float script_spell::get_delay( )
{
	return delay;
}

float script_spell::get_radius( )
{
	return radius;
}

bool script_spell::can_cast( game_object_script unit )
{
	if ( unit != nullptr && unit->is_valid_target( this->range( ) ) )
	{
		return this->is_ready( );
	}

	return false;
}

bool script_spell::is_in_range( game_object_script target, float range )
{
	if ( target == nullptr || !target->is_valid( ) )
	{
		return false;
	}

	if ( target->is_ai_hero( ) )
		return this->is_in_range( target->get_path_controller( )->get_position_on_path( ), range );

	return this->is_in_range( target->get_position( ), range );
}

bool script_spell::is_in_range( vector const& point, float range )
{
	auto source = from;

	if ( !source.is_valid( ) )
		source = myhero->get_path_controller( )->get_position_on_path( );

	auto r = this->range( );
	auto range_sqr = r * r;

	return source.distance_squared( point ) < ( range < 0 ? range_sqr : ( range * range ) );
}

bool script_spell::cast_on_best_farm_position( int minMinions, bool is_jugnle_mobs )
{
	if ( gametime->get_time( ) < last_cast_spell + sciprt_spell_wait )
		return false;

	if ( is_spell_locked( ) )
		return false;
	
	auto best_pos = get_cast_on_best_farm_position( minMinions, is_jugnle_mobs );

	if ( best_pos.is_valid( ) )
	{
		if ( !this->is_charged_spell )
		{
			myhero->cast_spell( this->slot, best_pos );

			last_cast_spell = gametime->get_time( );
			return true;
		}

		if ( is_charging( ) && gametime->get_time( ) - charging_started_time > 0.f )
		{
			myhero->update_charged_spell( this->slot, best_pos, true );

			last_cast_spell = gametime->get_time( );
			return true;
		}
	}

	return false;
}

bool script_spell::cast( game_object_script unit, hit_chance minimum, bool aoe, int min_targets )
{
	if ( gametime->get_time( ) < last_cast_spell + sciprt_spell_wait )
		return false;
	
	if ( is_spell_locked( ) )
		return false;
	
	vector cast_position;

	prediction_input x;

	if ( !this->from.is_valid( ) )
		x._from = myhero->get_position( );
	else
		x._from = this->from;

	x.unit = unit;
	x.delay = this->delay;
	x.radius = this->radius;
	x.speed = this->speed;
	x.collision_objects = this->collision_flags;
	x.range = this->range( );
	x.type = this->type;
	x.aoe = aoe;
	x.spell_slot = this->slot;
	x.use_bounding_radius = this->type != skillshot_type::skillshot_circle;

	auto output = prediction->get_prediction( &x );

	if ( output.hitchance >= minimum && output.aoe_targets_hit_count( ) >= min_targets )
	{
		cast_position = output.get_cast_position( );

		if ( !this->is_charged_spell )
		{
			myhero->cast_spell( this->slot, cast_position );

			last_cast_spell = gametime->get_time( );
			return true;
		}

		if ( is_charging( ) && gametime->get_time( ) - charging_started_time > 0.f )
		{
			myhero->update_charged_spell( this->slot, cast_position, true );

			last_cast_spell = gametime->get_time( );
			return true;
		}
	}
	return false;
}

bool script_spell::cast( vector position )
{
	if ( gametime->get_time( ) < last_cast_spell + sciprt_spell_wait )
		return false;
	
	if ( is_spell_locked( ) )
		return false;
	
	if ( !this->is_charged_spell )
	{
		myhero->cast_spell( this->slot, position );

		last_cast_spell = gametime->get_time( );
		return true;
	}
	if ( is_charging( ) && gametime->get_time( ) - charging_started_time > 0.f )
	{
		myhero->update_charged_spell( this->slot, position, true );

		last_cast_spell = gametime->get_time( );
		return true;
	}
	return start_charging( );
}

bool script_spell::cast( game_object_script unit )
{
	if ( gametime->get_time( ) < last_cast_spell + sciprt_spell_wait )
		return false;

	if ( !this->is_charged_spell )
	{
		myhero->cast_spell( this->slot, unit );
		last_cast_spell = gametime->get_time( );
		return true;
	}
	if ( is_charging( ) && gametime->get_time( ) - charging_started_time > 0.f )
	{
		myhero->update_charged_spell( this->slot, unit->get_position( ), true );

		last_cast_spell = gametime->get_time( );
		return true;
	}
	return false;
}

bool script_spell::cast( vector startPosition, vector endPosition )
{
	if ( gametime->get_time( ) < last_cast_spell + sciprt_spell_wait )
		return false;

	myhero->cast_spell( this->slot, startPosition, endPosition );

	last_cast_spell = gametime->get_time( );
	return true;
}

bool script_spell::is_charging( )
{
	if ( !this->is_ready( ) )
		return false;

	if ( gametime->get_time( ) - this->charging_started_time < 0.3f )
	{
		return true;
	}

	auto buff = get_charge_buff( );

	return buff != nullptr && buff->is_valid( ) && buff->is_alive( );
}

bool script_spell::is_fully_charged( )
{
	return this->is_charging( ) && this->charged_percentage( ) > 0.975f;
}

bool script_spell::start_charging( )
{
	if ( gametime->get_time( ) < last_cast_spell + sciprt_spell_wait )
		return false;

	if ( !is_charging( ) && this->is_ready( ) )
	{
		if ( gametime->get_time( ) < last_cast_spell + 1.5f )
			return false;

		myhero->cast_spell( this->slot, true, true );

		last_cast_spell = gametime->get_time( );

		charging_started_time = gametime->get_time( ) - (ping->get_ping( ) / 1000.f);
		return true;
	}
	return this->is_charging( );
}

bool script_spell::start_charging( const vector& position )
{
	if ( gametime->get_time( ) < last_cast_spell + sciprt_spell_wait )
		return false;
	
	if ( is_spell_locked( ) )
		return false;

	if ( !is_charging( ) && this->is_ready( ) )
	{
		myhero->cast_spell( this->slot, position, true, true );

		last_cast_spell = gametime->get_time( );
		charging_started_time = gametime->get_time( ) - ( ping->get_ping( ) / 1000.f);
		return true;
	}
	return this->is_charging( );
}

bool script_spell::fast_cast( vector position )
{
	if ( gametime->get_time( ) < last_cast_spell + sciprt_spell_wait )
		return false;

	if ( is_charging( ) )
		myhero->update_charged_spell( this->slot, position, true );
	else 
	{
		if ( is_spell_locked( ) )
			return false;

		myhero->cast_spell( this->slot, position );
	}

	last_cast_spell = gametime->get_time( );

	return true;
}

std::vector<collisionable_objects> script_spell::get_collision_flags( )
{
	return collision_flags;
}

vector script_spell::range_check_from( )
{
	return from;
}

void script_spell::set_radius( float radius )
{
	this->radius = radius;
}

void script_spell::set_speed( float speed )
{
	this->speed = speed;
}
void script_spell::set_delay( float delay )
{
	this->delay = delay;
}
void script_spell::set_range( float range )
{
	this->_range = range;
}
void script_spell::set_sollision_flags( std::vector <collisionable_objects> flags )
{
	this->collision_flags = flags;
	this->collision = flags.size( ) != 0;
}
void script_spell::set_range_check_from( vector const& position )
{
	this->from = position;
}
void script_spell::set_skillshot( float delay, float radius, float speed, std::vector < collisionable_objects> flags, skillshot_type skillshot_type )
{
	this->type = skillshot_type;
	this->delay = delay;
	this->radius = radius;
	this->speed = speed;
	this->collision_flags = flags;
	this->collision = flags.size( ) != 0;
	
	if ( this->slot >= spellslot::q && this->slot <= spellslot::r )
	{
		is_spell_lock_enable = true;
	}
}
void script_spell::set_charged( float range_min, float range_max, float charge_duration )
{
	this->charged_min_range = range_min;
	this->charged_max_range = range_max;
	this->charge_duration = charge_duration;
	this->_range = range_max;
	this->is_charged_spell = true;
}

prediction_output script_spell::get_prediction( game_object_script target, vector origin, vector range_check_from )
{
	prediction_input x;
	x._from = origin;
	x._range_check_from = range_check_from;
	x.unit = target;
	x.delay = this->delay;
	x.radius = this->radius;
	x.speed = this->speed;
	x.collision_objects = this->collision_flags;
	x.range = this->range( );
	x.type = this->type;
	x.aoe = false;
	x.spell_slot = this->slot;
	x.use_bounding_radius = this->type != skillshot_type::skillshot_circle;

	return prediction->get_prediction( &x );
}

prediction_output script_spell::get_prediction( game_object_script target, bool aoe, float overrideRange, std::vector<collisionable_objects> collisionable )
{
	prediction_input x;

	if ( !this->from.is_valid( ) )
		x._from = myhero->get_position( );
	else
		x._from = this->from;

	x.unit = target;
	x.delay = this->delay;
	x.radius = this->radius;
	x.speed = this->speed;
	x.collision_objects = collisionable.empty( ) ? this->collision_flags : collisionable;
	x.range = overrideRange > 0 ? overrideRange : this->range( );
	x.type = this->type;
	x.aoe = aoe;
	x.spell_slot = this->slot;
	x.use_bounding_radius = this->type != skillshot_type::skillshot_circle;

	return prediction->get_prediction( &x );
}

std::vector<game_object_script> script_spell::get_collision( const vector& from, const std::vector<vector>& to_pos, float speedOverride, float delayOverride, float radiusOverride )
{
	prediction_input x;
	x._from = from;
	x.delay = delayOverride > 0 ? delayOverride : this->delay;
	x.speed = speedOverride > 0 ? speedOverride : this->speed;
	x.radius = radiusOverride > 0 ? radiusOverride : this->radius;
	x.collision_objects = this->collision_flags;
	x.range = this->range( );
	x.type = this->type;
	x.spell_slot = this->slot;

	return prediction->get_collision( to_pos, &x );
}

prediction_output script_spell::get_prediction_no_collision( game_object_script target, bool aoe, float overrideRange )
{
	prediction_input x;

	if ( !this->from.is_valid( ) )
		x._from = myhero->get_position( );
	else
		x._from = this->from;

	x.unit = target;
	x.delay = this->delay;
	x.radius = this->radius;
	x.speed = this->speed;
	x.collision_objects = { };
	x.range = overrideRange > 0 ? overrideRange : this->range( );
	x.type = this->type;
	x.aoe = aoe;
	x.spell_slot = this->slot;
	x.use_bounding_radius = this->type != skillshot_type::skillshot_circle;

	return prediction->get_prediction( &x );
}

float script_spell::get_last_cast_spell_time( )
{
	return last_cast_spell;
}

buff_instance_script script_spell::get_charge_buff( )
{
	if ( this->is_charged_spell )
	{
		switch ( myhero->get_champion( ) )
		{
			case champion_id::Varus:
			{
				return myhero->get_buff( buff_hash( "VarusQ" ) );
			}
			case champion_id::Xerath:
			{
				return myhero->get_buff( buff_hash( "XerathArcanopulseChargeUp" ) );
			}
			case champion_id::Pyke:
			{
				return myhero->get_buff( buff_hash( "PykeQ" ) );
			}
			case champion_id::Pantheon:
			{
				return myhero->get_buff( buff_hash( "PantheonQ" ) );
			}
			case champion_id::Sion:
			{
				return myhero->get_buff( buff_hash( "SionQ" ) );
			}
			case champion_id::Viego:
			{
				return myhero->get_buff( buff_hash( "ViegoW" ) );
			}
			case champion_id::Zac:
			{
				return myhero->get_buff( buff_hash( "ZacE" ) );
			}
			case champion_id::Poppy:
			{
				return myhero->get_buff( buff_hash( "PoppyR" ) );
			}
			case champion_id::Vi:
			{
				return myhero->get_buff( buff_hash( "ViQ" ) );
			}
			default:
				return nullptr;
		}
	}

	return nullptr;
}

game_object_script script_spell::get_target( float extra_range )
{
	return target_selector->get_target( this->range( ) + extra_range, this->get_damage_type( ) );
}

bool math::IsZero( float A )
{
	return fabsf( A ) < 1e-6f;
}

bool math::NearEqual( float A, float B, int maxUlpsDiff )
{
	Float_t uA( A );
	Float_t uB( B );

	// Different signs means they do not match.
	if ( uA.Negative( ) != uB.Negative( ) )
	{
		// Check for equality to make sure +0==-0
		if ( A == B )
			return true;
		return false;
	}

	// Find the difference in ULPs.
	int ulpsDiff = abs( uA.i - uB.i );
	if ( ulpsDiff <= maxUlpsDiff )
		return true;
	return false;
}

prediction_output prediction_manager::get_prediction( game_object_script unit, float delay )
{
	prediction_input input;
	input.unit = unit;
	input.delay = delay;

	return prediction->get_prediction( &input );
}

prediction_output prediction_manager::get_prediction( game_object_script unit, float delay, float radius )
{
	prediction_input input;
	input.unit = unit;
	input.delay = delay;
	input.radius = radius;

	return prediction->get_prediction( &input );
}

prediction_output prediction_manager::get_prediction( game_object_script unit, float delay, float radius, float speed )
{
	prediction_input input;
	input.unit = unit;
	input.delay = delay;
	input.radius = radius;
	input.speed = speed;

	return prediction->get_prediction( &input );
}

prediction_output prediction_manager::get_prediction( game_object_script unit, float delay, float radius, float speed, std::vector<collisionable_objects> collisionable )
{
	prediction_input input;
	input.unit = unit;
	input.delay = delay;
	input.radius = radius;
	input.speed = speed;
	input.collision_objects = collisionable;

	return prediction->get_prediction( &input );
}
