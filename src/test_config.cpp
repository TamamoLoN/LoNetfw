#include "config/config.h"
#include "yaml-cpp/yaml.h"

int main(int argc, char const *argv[])
{
    auto config = std::make_shared<lon::config::Config>();
    lon::config::ConfigData<int>::Ptr data1 =
        std::make_shared<lon::config::ConfigData<int>>("test.port", 123, "testtest");

    lon::config::ConfigData<float>::Ptr data2 =
        std::make_shared<lon::config::ConfigData<float>>("test.value", 3.1415f, "testtest");

    auto data3 =
        lon::config::Config::setData("test.vec", std::vector<int>({1, 2, 3, 4}), "test.vec");

    config->setData(data1);
    config->setData("test.value", 3.1415f, "testtest");
    config->setData(data3);

    // auto vec = lon::config::Config::getData<std::vector<int>>("test.vec")->getData();
    // for (auto &v : vec)
    // {
    //     LON_INFO(LON_LOG_ROOT) << v;
    // }

    auto res = config->getData<int>("test");
    if (res != nullptr)
    {
        LON_INFO(LON_LOG_ROOT) << res->toString();
    }

    YAML::Node root = YAML::LoadFile("/home/idriver/yqh/git/LoNetfw/.config/log.yaml");
    auto log_node   = root["logs"];
    // for (const auto &node : log_node)
    // {
    //     LON_DEBUG(LON_LOG_ROOT) << node["name"].as<std::string>();
    // }
    // LON_FATAL(LON_LOG_ROOT) << root;
    // lon::util::printYamlString(root, 0);

    LON_INFO(LON_LOG_ROOT) << "before: " << data1->getData();
    LON_INFO(LON_LOG_ROOT) << "before: " << data2->toString();
    LON_INFO(LON_LOG_ROOT) << "before: " << data3->toString();
    // config->parseFromYaml("/home/idriver/yqh/git/LoNetfw/.config/log.yaml");
    lon::config::Config::parseFromYaml(root);

    LON_INFO(LON_LOG_ROOT) << "after: " << data1->getData();
    LON_INFO(LON_LOG_ROOT) << "after: " << data2->toString();
    LON_INFO(LON_LOG_ROOT) << "after: " << data3->toString();

    return 0;
}
