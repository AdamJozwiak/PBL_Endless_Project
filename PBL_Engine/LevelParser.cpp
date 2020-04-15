#include "LevelParser.h"

#include <unordered_map>
#include <vector>

#include "yaml-cpp/include/yaml-cpp/yaml.h"

struct GameObject {
    std::string nazwa;
    int rotation_x;
    int rotation_y;
    int rotation_z;
    int rotation_w;
    int position_x;
    int position_y;
    int position_z;
    int scale_x;
    int scale_y;
    int scale_z;
    int root_Order;
    int eulerAngle_x;
    int eulerAngle_y;
    int eulerAngle_z;
};

LevelParser::LevelParser() {}

LevelParser::~LevelParser() {}

void LevelParser::load() {
    std::vector<YAML::Node> nodes = YAML::LoadAllFromFile("1.yaml");

    // Map all possible nodes with corresponding identifiers
    std::unordered_map<std::string, YAML::Node> map;
    for (auto const &document : nodes) {
        for (YAML::const_iterator i = document.begin(); i != document.end();
             ++i) {
            if (!i->second["id"]) {
                continue;
            }
            map.insert({i->second["id"].as<std::string>(), document});
        }
    }

    GameObject level[10];
    int j = 0;
    for (int i = 0; i < nodes.size(); i++) {
        if (nodes[i]["GameObject"]) {
            if (nodes[i]["GameObject"]["m_Name"]) {
                level[j].nazwa =
                    nodes[i]["GameObject"]["m_Name"].as<std::string>();
            }
        }
        if (nodes[i]["Transform"]) {
            if (nodes[i]["Transform"]["m_LocalRotation"]) {
                for (YAML::const_iterator it =
                         nodes[i]["Transform"]["m_LocalRotation"].begin();
                     it != nodes[i]["Transform"]["m_LocalRotation"].end();
                     it++) {
                    if (it->first.as<std::string>() == "x") {
                        level[j].rotation_x = it->second.as<float>();
                    }
                    if (it->first.as<std::string>() == "y") {
                        level[j].rotation_y = it->second.as<float>();
                    }
                    if (it->first.as<std::string>() == "z") {
                        level[j].rotation_z = it->second.as<float>();
                    }
                    if (it->first.as<std::string>() == "w") {
                        level[j].rotation_w = it->second.as<float>();
                    }
                }
            }
            if (nodes[i]["Transform"]["m_LocalPosition"]) {
                for (YAML::const_iterator it =
                         nodes[i]["Transform"]["m_LocalPosition"].begin();
                     it != nodes[i]["Transform"]["m_LocalPosition"].end();
                     it++) {
                    if (it->first.as<std::string>() == "x") {
                        level[j].position_x = it->second.as<float>();
                    }
                    if (it->first.as<std::string>() == "y") {
                        level[j].position_y = it->second.as<float>();
                    }
                    if (it->first.as<std::string>() == "z") {
                        level[j].position_z = it->second.as<float>();
                    }
                }
            }
            if (nodes[i]["Transform"]["m_LocalScale"]) {
                for (YAML::const_iterator it =
                         nodes[i]["Transform"]["m_LocalScale"].begin();
                     it != nodes[i]["Transform"]["m_LocalScale"].end(); it++) {
                    if (it->first.as<std::string>() == "x") {
                        level[j].scale_x = it->second.as<float>();
                    }
                    if (it->first.as<std::string>() == "y") {
                        level[j].scale_y = it->second.as<float>();
                    }
                    if (it->first.as<std::string>() == "z") {
                        level[j].scale_z = it->second.as<float>();
                    }
                }
            }
            if (nodes[i]["Transform"]["m_RootOrder"]) {
                level[j].root_Order =
                    nodes[i]["Transform"]["m_RootOrder"].as<int>();
            }
            if (nodes[i]["Transform"]["m_LocalEulerAnglesHint"]) {
                for (YAML::const_iterator it =
                         nodes[i]["Transform"]["m_LocalEulerAnglesHint"]
                             .begin();
                     it !=
                     nodes[i]["Transform"]["m_LocalEulerAnglesHint"].end();
                     it++) {
                    if (it->first.as<std::string>() == "x") {
                        level[j].eulerAngle_x = it->second.as<float>();
                    }
                    if (it->first.as<std::string>() == "y") {
                        level[j].eulerAngle_y = it->second.as<float>();
                    }
                    if (it->first.as<std::string>() == "z") {
                        level[j].eulerAngle_z = it->second.as<float>();
                    }
                }
            }
        }
        j++;
    }
}
