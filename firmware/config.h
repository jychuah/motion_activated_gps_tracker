#ifndef __CONFIG_H__
#define __CONFIG_H__

class Config {
    public:
    	Config(void);
    	
        void begin(void);

    private:
        void parseValue(const char *event, const char *data);
        
};

#endif
