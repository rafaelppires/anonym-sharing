#include <memory_database.h>
#include <string.h>

//------------------------------------------------------------------------------
void MemDatabase::add_user_to_group(const std::string &gname,
                                    const std::string &new_uid) {
    std::lock_guard<std::mutex> lock(gmutex_);
    if (groups_.find(gname) == groups_.end()) {
        throw std::invalid_argument("attempt to add user '" + new_uid +
                                    "' to"
                                    "non existing group: '" +
                                    gname + "'");
    }

    {
        std::lock_guard<std::mutex> lock(umutex_);
        if (users_.find(new_uid) == users_.end()) {
            throw std::invalid_argument("attempt to add unknown user '" +
                                        new_uid + "' to group '" + gname + "'");
        }
    }

    auto rep = groups_[gname].insert(new_uid);
    if (!rep.second)  // existing user
        throw std::logic_error("'" + new_uid +
                               "' was already a member of "
                               "group '" +
                               gname + "'");
}

//------------------------------------------------------------------------------
void MemDatabase::create_user(const std::string &uid, const std::string &key) {
    std::lock_guard<std::mutex> lock(umutex_);
    if (users_.find(uid) == users_.end()) {
        users_[uid] = key;
    } else {
        throw std::invalid_argument("attempt to create an existing user: '" +
                                    uid + "'");
    }
}

//------------------------------------------------------------------------------
void MemDatabase::create_group(const std::string &gname,
                               const std::string &uid) {
    std::lock_guard<std::mutex> lock(gmutex_);
    if (groups_.find(gname) == groups_.end()) {
        {
            std::lock_guard<std::mutex> lock(umutex_);
            if (users_.find(uid) == users_.end()) {
                throw std::invalid_argument("tried to create group '" + gname +
                                            "' with an unknown user:'" + uid +
                                            "'");
            }
        }
        groups_[gname].insert(uid);
    } else {
        throw std::invalid_argument("attempt to create an existing group: '" +
                                    gname + "'");
    }
}

//------------------------------------------------------------------------------
void MemDatabase::delete_user(const std::string &uid) {
    std::lock_guard<std::mutex> ulock(umutex_), glock(gmutex_);
    for(auto &kv : groups_)
        kv.second.erase(uid);
    users_.erase(uid);
}

//------------------------------------------------------------------------------
void MemDatabase::delete_all_data() {
    std::lock_guard<std::mutex> ulock(umutex_), glock(gmutex_);
    users_.clear();
    groups_.clear();
}

//------------------------------------------------------------------------------
bool MemDatabase::is_user_part_of_group(const std::string &uname,
                                        const std::string &gname) {
    if (groups_.find(gname) != groups_.end()) {
        return groups_[gname].find(uname) != groups_[gname].end();
    } else {
        throw std::invalid_argument("unknown group '" + gname + "'");
    }
}
//------------------------------------------------------------------------------
void MemDatabase::remove_user_from_group(const std::string &gname,
                                         const std::string &uid) {
    std::lock_guard<std::mutex> lock(gmutex_);
    if (groups_.find(gname) != groups_.end()) {
        {
            std::lock_guard<std::mutex> lock(umutex_);
            if (users_.find(uid) == users_.end()) {
                throw std::invalid_argument(
                    "tried to remove an unknown user '" + uid +
                    "' from group '" + gname + "'");
            }
        }

        if (groups_[gname].find(uid) == groups_[gname].end()) {
            throw std::invalid_argument("attempt to remove user '" + uid +
                                        "' from group '" + gname +
                                        "' to which he or she does not belong");
        }

        if (groups_[gname].erase(uid) == 0)  // nothing was removed
            throw std::invalid_argument(
                "run to the hills: "
                "this should not ever happen");
    } else {
        throw std::invalid_argument("attempt to remove user '" + uid +
                                    "' from"
                                    " inexistent group: '" +
                                    gname + "'");
    }
}

//------------------------------------------------------------------------------
KeyArray MemDatabase::get_keys_of_group(const std::string &gname) {
    std::lock_guard<std::mutex> lock(gmutex_);
    KeyArray ret;
    if (groups_.find(gname) != groups_.end()) {
        if (groups_[gname].empty()) {
            throw std::invalid_argument("group '" + gname + "' is empty");
        }

        KeyArray::value_type key;
        for (const auto &uid : groups_[gname]) {
            std::lock_guard<std::mutex> lock(umutex_);
            if (users_.find(uid) != users_.end()) {
                key.fill(0);
                memcpy(key.data(), users_[uid].c_str(),
                       std::min(users_[uid].size(), size_t(KEY_SIZE)));
                ret.push_back(key);
            }
        }
    } else {
        throw std::invalid_argument("unknown group '" + gname + "'");
    }
    return ret;
}

//------------------------------------------------------------------------------
