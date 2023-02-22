//
// Created by fengyuwei on 2022/8/14.
//

#include "test.h"
#include <vector>
#include <glog/logging.h>

int main() {
    std::vector<int> aaa;
    aaa.push_back(1);
    aaa.push_back(1);
    aaa.push_back(1);
    aaa.push_back(1);
    aaa.push_back(1);

    aaa.erase(aaa.begin(), aaa.begin()+3);

    LOG(INFO) << std::string(aaa.begin(), aaa.end());
}