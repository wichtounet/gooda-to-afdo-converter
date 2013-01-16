//=======================================================================
// Copyright Baptiste Wicht 2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef GOODA_HASH_HPP
#define GOODA_HASH_HPP

/*!
 * \file hash.hpp
 * \brief Contains utility functions for hashing.  
 */

namespace gooda {

/*!
 * \brief Combine a hash with a seed. 
 * \param seed The current hash. 
 * \param v The value to hash and combine the hash with the current value. 
 * \tparam T The type of value to hash. 
 * \return The combined hash.
 */
template <class T>
inline void hash_combine(std::size_t& seed, const T& v){
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

}

namespace std {

/*!
 * \brief Hash a pair
 * \tparam U The first type of the pair. 
 * \tparam V The second type of the pair. 
 */
template<typename U, typename V>
struct hash<std::pair<U, V>> {
    /*!
     * \brief Hash the pair
     * \param key The pair to hash
     * \return The hash value of the pair
     */
    std::size_t operator()(const std::pair<U, V>& key) const {
        std::size_t seed = 0;

        gooda::hash_combine(seed, key.first);
        gooda::hash_combine(seed, key.second);

        return seed;
    }
};

}

#endif
