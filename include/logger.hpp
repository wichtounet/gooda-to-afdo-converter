#ifndef GOODA_LOGGER_HPP
#define GOODA_LOGGER_HPP

#include <iostream>

#define LOGGING

#ifdef DISABLE_LOGGING
#undef LOGGING
#endif

#ifdef LOGGING
extern int current_level;
#else
static const int current_level = 0;
#endif

template<typename Level>
struct logger {
    logger(){
        if(Level::level <= current_level){
            std::cout << "[" << Level::label << "] ";
        }
    }

    template<typename T>
    logger& operator<<(T t){
        if(Level::level <= current_level){
            std::cout << t;
        }
        
        return *this;
    }
};

struct log {
    struct Error {
        constexpr static const char* label = "ERROR";
        static const int level = 0;
    };

    struct Warning {
        constexpr static const char* label = "WARNING";
        static const int level = 1;
    };
    
    struct Debug {
        constexpr static const char* label = "DEBUG";
        static const int level = 2;
    };

#ifdef LOGGING
    static void set_level(int level){
        current_level = level;
    }
#else
    static void set_level(int){}
#endif

    template<typename Level>
    static inline logger<Level> emit(){
        return logger<Level>();
    }

    static const char tab = '\t';
    static const char endl= '\n';
};

#endif
