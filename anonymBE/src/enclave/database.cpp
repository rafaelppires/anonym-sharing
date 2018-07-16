#include <database.h>
#include <sgx_trts.h>
#define KEY_SIZE 32

//------------------------------------------------------------------------------
void Database::add_user2group( const std::string &gname, 
                               const std::string &new_uid ) {
    if( groups_.find(gname) == groups_.end() ) {
        throw std::invalid_argument("attempt to add user '" + new_uid + "' to"
                                    "non existing group: '" + gname + "'");
    }

    if( users_.find( new_uid ) == users_.end() ) {
        throw std::invalid_argument("attempt to add unknown user '" + new_uid +
                                    "' to group '" + gname + "'");
    }

    groups_[gname].insert( new_uid );
}

//------------------------------------------------------------------------------
std::string Database::create_user( const std::string &uid ) {
    if( users_.find( uid ) == users_.end() ) {
        unsigned char rnd[ KEY_SIZE ];
        sgx_read_rand( rnd, sizeof(rnd) );
        users_[uid] = std::string( (const char*)rnd, sizeof(rnd) );
        return users_[uid];
    }
    throw std::invalid_argument("attempt to create an existing user: '" + 
                                uid + "'" );
}

//------------------------------------------------------------------------------
void Database::create_group( const std::string &gname, const std::string &uid ){
    if( groups_.find(gname) == groups_.end() ) {
        if( users_.find(uid) == users_.end() ) {
            throw std::invalid_argument("tried to create group '" + gname + 
                                        "' with an unknown user:'" + uid + "'");
        }
        groups_[gname].insert( uid );
    } else {
        throw std::invalid_argument( "attempt to create an existing group: '" 
                                     + gname + "'");
    }
}

//------------------------------------------------------------------------------

