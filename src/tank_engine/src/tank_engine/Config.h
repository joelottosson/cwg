#include <iostream>
#include <stdlib.h>
#include <confuse.h>

class Config
{
public:



	int m_coin_value;
	int m_gas_penalty;
	int m_survival_points;
	int m_hit_points;
	int m_dude_penalty;
	int m_draw_points;
	int m_walkover_penalty;
	int m_smoke_timer;

	Config(const char *file_path);

	void printConfig();






};
