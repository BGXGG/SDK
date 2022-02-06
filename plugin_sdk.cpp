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

std::uint16_t locked_target_selector::_last_target_id = 0;
std::uint32_t locked_target_selector::_last_target_network_id = 0;
damage_type locked_target_selector::_last_damage_type = damage_type::physical;


int __stdcall DllMain( void*, unsigned long, void* ) { return 1; }

PLUGIN_API int get_sdk_version( )
{
	return PLUGIN_SDK_VERSION;
}

std::vector<std::unique_ptr<script_spell>> script_spells;

script_spell* plugin_sdk_core::register_spell( spellslot slot, float range )
{
	if ( slot == spellslot::invalid )
		return nullptr;

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
			if ( physical_shield && this->get_health_percent( ) > 30 )
			{
				auto maw = this->has_item( static_cast< uint32_t >( ItemId::Maw_of_Malmortius ) );

				if ( maw != spellslot::invalid )
				{
					if ( get_spell_state( maw ) == spell_state::Ready )
						result += 300 + this->get_bonus_spell_block( );
				}
				else
				{
					auto hex = this->has_item( static_cast< uint32_t >( ItemId::Hexdrinker ) );

					if ( hex != spellslot::invalid && get_spell_state( hex ) == spell_state::Ready )
						result += 100 + 10 * this->get_level( );
				}
			}

			switch ( this->get_champion( ) )
			{
				case champion_id::Kled:
					result += this->get_hp_bar_stacks( );
					break;
				case  champion_id::Blitzcrank:
					if ( this->has_buff( { buff_hash( "BlitzcrankManaBarrierCD" ),buff_hash( "ManaBarrier" ) } ) == false )
					{
						result += this->get_max_mana( ) * 0.3f;
					}
					break;
				case champion_id::Yasuo:
					if ( this->get_mana( ) == 100 )
					{
						int passive[] = { 100, 105, 110, 115, 120, 130, 140, 150, 165, 180, 200, 225, 255, 330, 380, 440, 510 };
						int lvl = std::max( 17, this->get_level( ) - 1 );
						auto temp = passive[ lvl ];
						result += temp;
					}
					break;
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

	if ( this->is_ai_base( ) )
	{
		vFrom = this->get_path_controller( )->get_position_on_path( );
	}

	if ( to->is_ai_base( ) )
	{
		vTo = to->get_path_controller( )->get_position_on_path( );
	}

	return vFrom.distance( vTo );
}

float game_object::get_distance( const vector& to )
{
	auto vFrom = this->get_position( );

	if ( this->is_ai_base( ) )
	{
		vFrom = this->get_path_controller( )->get_position_on_path( );
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
	auto pc = myhero->get_path_controller( );

	if ( pc )
		return pc->is_moving( );

	return false;
}

bool game_object::is_dashing( )
{
	auto pc = myhero->get_path_controller( );

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

	auto active = this->get_active_spell( );

	if ( !active || ( !active->is_channeling( ) && !active->is_charging( ) ) )
		return 0;

	auto champion = this->get_champion( );

	if ( champion == champion_id::Velkoz && active->get_spellslot( ) == spellslot::r )
		return 2;
	if ( champion == champion_id::Warwick )
	{
		auto spell_data = active->get_spell_data( );
		if ( spell_data && spell_data->get_name_hash( ) == spell_hash( "WarwickRChannel" ) )
		{
			return 2;
		}
	}
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
	if ( champion == champion_id::Jhin && active->get_spellslot( ) == spellslot::r )
		return 2;
	if ( champion == champion_id::TwistedFate && active->get_spellslot( ) == spellslot::r )
		return 2;
	if ( champion == champion_id::Janna && active->get_spellslot( ) == spellslot::r )
		return 2;
	if ( champion == champion_id::TahmKench && active->get_spellslot( ) == spellslot::r )
		return 2;
	if ( champion == champion_id::Shen && active->get_spellslot( ) == spellslot::r )
		return 2;
	//Low Priority
	if ( champion == champion_id::Sion && active->get_spellslot( ) == spellslot::q )
		return 1;
	if ( champion == champion_id::Varus && active->get_spellslot( ) == spellslot::q )
		return 1;
	if ( champion == champion_id::Pantheon && active->get_spellslot( ) == spellslot::e )
		return 1;
	if ( champion == champion_id::TahmKench && active->get_spellslot( ) == spellslot::r )
		return 1;
	if ( champion == champion_id::Xerath && active->get_spellslot( ) == spellslot::q )
		return 1;
	if ( champion == champion_id::Zac && active->get_spellslot( ) == spellslot::e )
		return 1;
	if ( champion == champion_id::MasterYi && active->get_spellslot( ) == spellslot::w )
		return 1;
	if ( champion == champion_id::FiddleSticks && active->get_spellslot( ) == spellslot::w )
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

		if ( unit->is_ai_base( ) )
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
	float detection_delay = 25.f;

	std::map<std::uint32_t, float> dash_map;
	std::vector<void*> p_handlers;

	void on_update( )
	{
		for ( auto&& target : entitylist->get_enemy_heroes( ) )
		{
			if ( dash_map.find( target->get_network_id( ) ) == dash_map.end( ) || !target->is_valid_target( ) )
			{
				continue;
			}

			//wait 25ms for buffs to trigger
			if ( gametime->get_time( ) - dash_map[ target->get_network_id( ) ] < detection_delay / 1000.f )
			{
				continue;
			}

			auto path_controller = target->get_path_controller( );

			if ( path_controller != nullptr && path_controller->is_dashing( ) && path_controller->is_moving( ) )
			{
				auto start = path_controller->get_start_vec( );
				auto end = path_controller->get_end_vec( );

				if ( start.distance( target->get_position( ) ) > target->get_bounding_radius( ) + 30 )
				{
					auto is_grab = bool( ( int ) target->get_action_state( ) & ( 1 << 27 ) );

					for ( auto&& buff : target->get_bufflist( ) )
					{
						if ( buff != nullptr && buff->is_valid( ) && buff->is_alive( ) )
						{
							game_object_script buff_caster = nullptr;

							if ( buff_caster = buff->get_caster( ), buff_caster != nullptr && buff_caster->is_ally( ) )
							{
								switch ( buff->get_hash_name( ) )
								{
									//buff_hash doesnt evaluate to constant in C++ older than 17
									case buff_hash_real( "plantsatchelknockback" ):
										is_grab = false;
										continue;

										//buff_hash doesnt evaluate to constant in C++ older than 17
									case buff_hash_real( "ThreshQ" ):
										is_grab = true;
										break;
								}

								if ( buff->get_type( ) == buff_type::Knockback )
								{
									is_grab = true;
									break;
								}
							}
						}
					}

					for ( auto const& callback : p_handlers )
					{
						if ( callback != nullptr )
						{
							reinterpret_cast< gapcloser_handler >( callback )( target, start, end, path_controller->get_dash_speed( ), is_grab );
						}
					}
				}
			}
		}
	}

	void on_new_path( game_object_script sender, const std::vector<vector>& path, bool is_dash, float dash_speed )
	{
		if ( is_dash && sender->is_ai_hero( ) )
		{
			dash_map[ sender->get_network_id( ) ] = gametime->get_time( );
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
			event_handler<events::on_update>::add_callback( on_update );
			event_handler<events::on_new_path>::add_callback( on_new_path );
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
			event_handler<events::on_update>::remove_handler( on_update );
			event_handler<events::on_new_path>::remove_handler( on_new_path );
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
		return ( fmaxf( 0.f, fminf( 1.f, ( gametime->get_time( ) - buff->get_start( ) + 0.25f - ( buff->get_hash_name( ) == buff_hash( "PykeQ" ) ? 0.4f : 0.f ) ) / this->charge_duration ) ) );
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
		myhero->cast_spell( this->slot, position );

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
