#ifndef CONFIG_H
#define CONFIG_H


#include <iostream>
#include <stdlib.h>
#include <confuse.h>
#include <Safir/Utilities/Internal/VisibilityHelpers.h>


#ifdef config_EXPORTS
#  define CONFIG_API SAFIR_HELPER_DLL_EXPORT
#else
#  define CONFIG_API SAFIR_HELPER_DLL_IMPORT
#endif
#define CONFIG_LOCAL SAFIR_HELPER_DLL_LOCAL


namespace ConfigSystem{
class CONFIG_API Config{
public:



	int m_coin_value;
	int m_gas_penalty;
	int m_survival_points;
	int m_hit_points;
	int m_dude_penalty;
	int m_draw_points;
	int m_walkover_penalty;
	int m_smoke_timer;

	int  m_frame_rate;
	bool m_audio_enabled;
	int m_master_volume;

	int m_laser_delay;
	int m_smoke_puffs;
	int m_smoke_spread;
	int m_smoke_speed;
	int m_death_explosion_count;
	int m_death_explosion_spread;
	int m_death_eclosion_time;
	int m_redeemer_radius;
	int m_redeemer_explosion_per_square;
	int m_redeemer_explosion_spread;

	int m_laser_sound_volume;
	int m_coin_volume;
	int m_scream_volume;
	int m_fire_volume;
	int m_explosion_volume;
	int m_smoke_volume;



	Config(const char *file_path);

	void printConfig();






};
}
#endif
