#ifndef _MEMDATABASE_H_
#define _MEMDATABASE_H_

#include <database.h>
#include <map>
#include <set>
typedef std::map<std::string,std::string> KVString;

class MDatabase : public Database {
public:
    void add_user_to_group( const std::string &gname, const std::string &new_uid );
    void create_user( const std::string &uid, const std::string &key );
    void create_group( const std::string &gname, const std::string &uid );
    void remove_user_from_group( const std::string &gname, const std::string &uid );
private:
    std::map<std::string, std::set<std::string> >  groups_;
    KVString users_; // pairs of username,password
};

#endif

