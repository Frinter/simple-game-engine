#pragma once

#include <vector>

class Condition
{
public:
    virtual ~Condition() {}
    virtual bool Check() = 0;
};

class MultiConditionChecker : public Condition
{
public:
    bool Check()
    {
        for (int i = 0; i < _conditions.size(); ++i)
        {
            if (_conditions[i]->Check() == false)
                return false;
        }

        return true;
    }

    void AddCondition(Condition *condition)
    {
        _conditions.push_back(condition);
    }

private:
    std::vector<Condition*> _conditions;
};
