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

/*!
 * \brief A logger to the standard output. 
 *
 * Should not be used directly. Should only be used via log::emit.
 */
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
    /*!
     * \struct Error
     * \brief Error Level for the logger
     */
    struct Error {
        constexpr static const char* label = "ERROR";       //!< The label of the level
        static const int level = 0;                         //!< The level of the level
    };
    
    /*!
     * \struct Warning
     * \brief Warning Level for the logger
     */
    struct Warning {
        constexpr static const char* label = "WARNING";     //!< The label of the level
        static const int level = 1;                         //!< The level of the level
    };
    
    /*!
     * \struct Debug
     * \brief Debug Level for the logger
     */
    struct Debug {
        constexpr static const char* label = "DEBUG";       //!< The label of the level
        static const int level = 2;                         //!< The level of the level
    };

#ifdef LOGGING
    static void set_level(int level){
        current_level = level;
    }
#else
    static void set_level(int){}
#endif

    /*!
     * \brief Return a logger for the given Level.
     * \tparam Level The logging level. Determine if the output is enabled or not at the current level. 
     * \return A logger configured with the given Level.
     */
    template<typename Level>
    static inline logger<Level> emit(){
        return logger<Level>();
    }

    static const char tab = '\t'; //!< Simple placeholder to generate a tab to the logging output
    static const char endl= '\n'; //!< Simple placeholder to generate a new line to the logging output
};

#endif
