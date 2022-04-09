#include "gtest/gtest.h"
#include <unordered_map>
#pragma once

class HashTest : public ::testing::Test
{
public:
    virtual void SetUp(void) { }
    virtual void TearDown(void) { }
    static void SetUpTestSuite() { }
    static void TearDownTestSuite() { }
};