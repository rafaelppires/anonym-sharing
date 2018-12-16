#ifndef _DATABASE_H_
#define _DATABASE_H_

#include <array>
#include <string>
#include <vector>
#define KEY_SIZE 32

typedef std::vector<std::array<uint8_t, KEY_SIZE>> KeyArray;

class Database {
   public:
    bool init(const std::string &initdata);
    void add_user_to_group(const std::string &gname, const std::string &uname);
    void create_user(const std::string &uname, const std::string &key);
    void delete_user(const std::string &uname);
    void create_group(const std::string &gname, const std::string &uid);
    bool is_user_part_of_group(const std::string &uname, 
                               const std::string &gname);
    void remove_user_from_group(const std::string &gname,
                                const std::string &uname);
    KeyArray get_keys_of_group(const std::string &group_name);
};

#endif
