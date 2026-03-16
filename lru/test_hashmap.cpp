#include <iostream>
#include <string>
#include "lru.hpp" // 只要引入这一个，它里面已经包含了 exception, utility 等

// 为了测试简单，我们可以先不写 Hash 和 Equal，直接用默认的 std::hash
int main() {
    // 1. 实例化你的 hashmap (Key=int, Value=std::string)
    sjtu::hashmap<int, std::string> map;

    // 2. 测试插入
    sjtu::pair<const int, std::string> p1(1, "hello");
    map.insert(p1);
    map.insert(sjtu::pair<const int, std::string>(2, "world"));

    // 3. 测试查找
    auto it = map.find(1);
    if (it != map.end()) {
        std::cout << "Found key 1: " << it->second << std::endl;
    }

    // 4. 测试更新和扩容等...
    std::cout << "Test Finished!" << std::endl;
    return 0;
}