// +build ignore

#include <gtest/gtest.h>

class MyTestEnvironment : public ::testing::Environment
{
public:
    virtual ~MyTestEnvironment() {}
    // Override this to define how to set up the environment.
    virtual void SetUp()
    {
        //add your setup code
    }
    // Override this to define how to tear down the environment.
    virtual void TearDown()
    {
        // add your teardown
    }
};
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    // ::testing::AddGlobalTestEnvironment(new MyTestEnvironment);
    return RUN_ALL_TESTS();
}

