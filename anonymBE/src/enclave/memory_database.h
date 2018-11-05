#ifndef _MEMDATABASE_H_
#define _MEMDATABASE_H_

#include <database.h>
#include <map>
#include <mutex>
#include <set>
typedef std::map<std::string, std::string> KVString;

class MemDatabase : public Database {
   public:
    bool init() { return true; }
    void add_user_to_group(const std::string &gname,
                           const std::string &new_uid);
    void create_user(const std::string &uid, const std::string &key);
    void create_group(const std::string &gname, const std::string &uid);
    void remove_user_from_group(const std::string &gname,
                                const std::string &uid);
    KeyArray get_keys_of_group(const std::string &group_name);

   private:
    std::map<std::string, std::set<std::string> > groups_;
    KVString users_;  // pairs of username,password
    std::mutex gmutex_, umutex_;
};

#endif
