#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <QList>
#include <QMap>
#include <QString>

class BaseFunction
{
public:
    enum FunctionType
    {
        UnaryOperator,
        BinaryOperator,
        OtherFunction
    };

protected:
    int m_priority;
    int m_args;
    FunctionType m_type;
    QString m_expression;

public:
    BaseFunction(int priority, int args, FunctionType type, const QString &qstr);

    virtual ~BaseFunction() {}

    int priority() const
    {
        return m_priority;
    }
    int numberOfArguments() const
    {
        return m_args;
    }
    FunctionType type() const
    {
        return m_type;
    }
    QString expr() const
    {
        return m_expression;
    }
};

using CustomFunction = double (*)(const QList<double> &);

using BinaryOperatorFunction = double (*)(double, double);

using UnaryOperatorFunction = double (*)(double);

class Function : public BaseFunction
{
private:
    CustomFunction m_func;

public:
    Function(const QString &qstr, int numberOfArguments,
             const CustomFunction &function);
    CustomFunction function() const;
};

class UnaryOperator : public BaseFunction
{
private:
    UnaryOperatorFunction m_func;

public:
    UnaryOperator(const QString &qstr, int priority,
                  const UnaryOperatorFunction &function);
    UnaryOperatorFunction function() const;
};

class BinaryOperator : public BaseFunction
{
private:
    BinaryOperatorFunction m_func;

public:
    BinaryOperator(const QString &qstr, int priority,
                   const BinaryOperatorFunction &function);
    BinaryOperatorFunction function() const;
};

class FunctionList
{
private:
    QList<BaseFunction *> m_funclist;

public:
    FunctionList();
    ~FunctionList();
    const BaseFunction *at(int i) const;
    const BaseFunction *op(const QString &signature) const;
    int size() const;
    int priority(const QString &signature) const;
    bool hasOperator(const QString &signature) const;
};

class QExpression
{
public:
    static FunctionList m_functions;
    static QMap<QString, double> m_variables;

    enum EvaluationError
    {
        NoError = 0,
        NoExpression,
        UnknownIdentifier,
        UnexpectedEnd,
        WrongNumberOfArguments
    };
    QExpression();
    QExpression(const QString &expression);
    bool eval();
    double result()
    {
        return m_result;
    };
    EvaluationError error()
    {
        return m_error;
    };
    void setExpression(const QString &expression)
    {
        m_expression = expression;
    };

private:
    QString m_expression;
    QString m_reversePolishNotation;
    double m_result = 0.0;
    EvaluationError m_error = NoError;
    bool toReversePolishNotation();
    void normalize();
};

#endif