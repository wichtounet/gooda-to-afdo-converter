#ifndef GOODA_LOGGER_HPP
#define GOODA_LOGGER_HPP

#define LOGGING

#ifdef DISABLE_LOGGING
#undef LOGGING
#endif

static int current_level = 0;

template<typename Level>
struct logger {
    logger(){
#ifdef LOGGING
        if(Level::level <= current_level){
            std::cout << "[" << Level::label << "] ";
        }
#endif
    }

    template<typename T>
    logger& operator<<(T t){
#ifdef LOGGING
        if(Level::level <= current_level){
            std::cout << t;
        }
#endif
        return *this;
    }
};

struct log {
    struct Warning {
        constexpr static const char* label = "WARNING";
        static const int level = 1;
    };

    static void set_level(int level){
#ifdef LOGGING
        current_level = level;
#endif
    }

    template<typename Level>
    static inline logger<Level> emit(){
        return logger<Level>();
    }

    static const char tab = '\t';
    static const char endl= '\n';
};

#endif
