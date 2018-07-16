#ifndef _DATABASE_H_
#define _DATABASE_H_

#include <string>
#include <map>
#include <set>
typedef std::map<std::string,std::string> KVString;

class Database {
public:
    void add_user2group( const std::string &gname, const std::string &new_uid );
    std::string create_user( const std::string &uid );
    void create_group( const std::string &gname, const std::string &uid );
private:
    std::map<std::string, std::set<std::string> >  groups_;
    KVString users_; // pairs of username,password
};

#endif

