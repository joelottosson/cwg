#include "Config.h"
#include <confuse.h>
#include <unistd.h>







Config::Config(const char *file_path){

    cfg_opt_t opts[] = {

        CFG_SIMPLE_INT("coin_value", &m_coin_value),
		CFG_SIMPLE_INT("gas_penalty", &m_gas_penalty),
		CFG_SIMPLE_INT("survival_points", &m_survival_points),
		CFG_SIMPLE_INT("hit_points", &m_hit_points),
		CFG_SIMPLE_INT("dude_penalty", &m_dude_penalty),
		CFG_SIMPLE_INT("draw_points", &m_draw_points),
		CFG_SIMPLE_INT("walkover_penalty", &m_walkover_penalty),
		CFG_SIMPLE_INT("smoke_timer", &m_smoke_timer),


        CFG_END()
    };

    //char buff[1024];
    //getcwd(buff, 1024);
    //std::wcout << "Current path is " << buff << std::endl;
    //abort();

    cfg_t *cfg;
    cfg = cfg_init(opts, 0);
    int status = cfg_parse(cfg, file_path);
    if(status == CFG_FILE_ERROR){
    	std::wcout << "File error when reading config file." << std::endl;
    	abort();
    }else if(status == CFG_PARSE_ERROR){
    	std::wcout << "Parse error when reading config file." << std::endl;
    	abort();
    }else if (status == CFG_SUCCESS){
    	std::wcout << "Config file loaded successfully" << std::endl;

    }

    cfg_free(cfg);


}

void Config::printConfig(){
	std::wcout << "coin value = " << m_coin_value << std::endl;
	std::wcout << "gas penalty = " << m_gas_penalty<< std::endl;
	std::wcout << "survival points = " << m_survival_points << std::endl;
	std::wcout << "hit points = " << m_hit_points << std::endl;
	std::wcout << "dude penalty = " << m_dude_penalty << std::endl;
	std::wcout << "draw points = " << m_draw_points << std::endl;
	std::wcout << "walkover penalty = " << m_walkover_penalty << std::endl;
	std::wcout << "m smoke timer = " << m_smoke_timer << std::endl;

}
