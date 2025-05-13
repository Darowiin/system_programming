#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <shadow.h>
#include <fcntl.h>

struct UserInfo {
    std::string name;
    uid_t uid;
    std::string home;
    std::string shell;
    std::string password_hash;
    std::set<std::string> groups;
    std::set<std::string> admin_groups;
};

std::map<std::string, std::string> parse_shadow() {
    std::map<std::string, std::string> shadow_map;

    int fd = open("/etc/shadow", O_RDONLY);
    setuid(getuid());
    if (fd < 0) return shadow_map;
    close(fd);

    std::ifstream shadow("/etc/shadow");
    std::string line;
    while (std::getline(shadow, line)) {
        std::istringstream iss(line);
        std::string user, pass;
        if (std::getline(iss, user, ':') && std::getline(iss, pass, ':')) {
            shadow_map[user] = pass;
        }
    }
    return shadow_map;
}

std::map<std::string, std::set<std::string>> parse_groups(std::map<std::string, std::set<std::string>>& admin_map) {
    std::map<std::string, std::set<std::string>> user_groups;

    std::ifstream group_file("/etc/group");
    std::string line;
    while (std::getline(group_file, line)) {
        std::istringstream iss(line);
        std::string groupname, x, gid, users;
        std::getline(iss, groupname, ':');
        std::getline(iss, x, ':');
        std::getline(iss, gid, ':');
        std::getline(iss, users, ':');

        std::istringstream user_stream(users);
        std::string user;
        while (std::getline(user_stream, user, ',')) {
            if (!user.empty()) {
                user_groups[user].insert(groupname);
            }
        }
    }

    std::ifstream gshadow_file("/etc/gshadow");
    while (std::getline(gshadow_file, line)) {
        std::istringstream iss(line);
        std::string groupname, password, admins, users;
        std::getline(iss, groupname, ':');
        std::getline(iss, password, ':');
        std::getline(iss, admins, ':');
        std::getline(iss, users, ':');

        std::istringstream admin_stream(admins);
        std::string admin;
        while (std::getline(admin_stream, admin, ',')) {
            if (!admin.empty()) {
                user_groups[admin].insert(groupname);
                admin_map[admin].insert(groupname);
            }
        }
    }

    return user_groups;
}

int main() {
    std::ifstream passwd("/etc/passwd");
    std::map<std::string, std::string> shadow_map = parse_shadow();
    std::map<std::string, std::set<std::string>> admin_map;
    std::map<std::string, std::set<std::string>> user_groups = parse_groups(admin_map);

    std::string line;
    while (std::getline(passwd, line)) {
        std::istringstream iss(line);
        std::string username, x, uid_str, gid_str, info, home, shell;
        std::getline(iss, username, ':');
        std::getline(iss, x, ':');
        std::getline(iss, uid_str, ':');
        std::getline(iss, gid_str, ':');
        std::getline(iss, info, ':');
        std::getline(iss, home, ':');
        std::getline(iss, shell, ':');

        uid_t uid = std::stoi(uid_str);

        UserInfo user;
        user.name = username;
        user.uid = uid;
        user.home = home;
        user.shell = shell;
        user.password_hash = shadow_map.count(username) ? shadow_map[username] : "<unavailable>";
        if (user_groups.count(username))
            user.groups = user_groups[username];
        if (admin_map.count(username))
            user.admin_groups = admin_map[username];

        std::cout << "\n=== User: " << user.name << " ===\n";
        std::cout << "UID: " << user.uid << "\n";
        std::cout << "Home: " << user.home << "\n";
        std::cout << "Shell: " << user.shell << "\n";
        std::cout << "Password Hash: " << user.password_hash << "\n";
        std::cout << "Groups: ";
        for (const auto& g : user.groups) {
            std::cout << g;
            if (user.admin_groups.count(g)) std::cout << " (admin)";
            std::cout << " ";
        }
        std::cout << "\n";
    }
    return 0;
}