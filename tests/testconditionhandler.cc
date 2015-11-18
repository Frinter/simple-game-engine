#include "catch.hh"

#include "conditionhandler.hh"

class AlwaysTrue : public Condition
{
public:
    bool Check()
    {
        return true;
    }
};

class AlwaysFalse : public Condition
{
public:
    bool Check()
    {
        return false;
    }
};

class StubCommand : public Command
{
public:
    StubCommand()
        : wasRun(false)
    {
    }

    void Execute()
    {
        wasRun = true;
    }

    bool wasRun;
};

TEST_CASE("ConditionHandler")
{
    ConditionHandler handler;
    StubCommand command;

    SECTION("does not run command when condition is false")
    {
        AlwaysFalse condition;
        handler.SetHandler(&condition, &command);
        handler.Update();

        REQUIRE(command.wasRun == false);
    }

    SECTION("runs command when condition is true")
    {
        AlwaysTrue condition;
        handler.SetHandler(&condition, &command);
        handler.Update();

        REQUIRE(command.wasRun == true);
    }

    SECTION("does not run command when condition is removed")
    {
        AlwaysTrue condition;
        handler.SetHandler(&condition, &command);
        handler.RemoveHandler(&condition, &command);
        handler.Update();

        REQUIRE(command.wasRun == false);
    }
}
