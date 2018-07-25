#ifndef _DATABASE_H_
#define _DATABASE_H_

#include <string>

class Database {
public:
    void add_user_to_group( const std::string &gname, 
                            const std::string &uname );
    std::string create_user( const std::string &uname, 
                             const std::string &key );
    void create_group( const std::string &gname, 
                       const std::string &uid );
    void remove_user_from_group( const std::string &gname, 
                                 const std::string &uname );
};

#endif

