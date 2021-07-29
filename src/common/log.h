/**
 * @file log.h
 * @author Dansz
 * @brief Simple logging functions
 * @version 0.1
 * @date 2021-07-29
 *
 * @copyright Copyright (c) 2021
 *
 */
#pragma once
#include <string>
namespace yocvs
{

    void log(const std::string &message, size_t level = 0);

}
