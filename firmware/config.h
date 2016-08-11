#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "application.h"

class Config {
    public:
    	Config(void);
    	
        void begin(void);

    private:
        void parse_config_get(const char *event, const char *data);
        void parse_child_changed(const char *event, const char *data);
        void parse_pair(String pair);
        
};

#endif
