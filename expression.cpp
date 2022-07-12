#include "expression.h"

#include <QDebug>
#include <QRegularExpression>
#include <QStack>
#include <QStringBuilder>
#include <cmath>

FunctionList QExpression::m_functions;
QMap<QString, double> QExpression::m_variables;

bool isNumber(const QString &arg)
{
    static QRegularExpression re("[0-9]+");
    return re.match(arg).hasMatch();
};

QExpression::QExpression() {}

QExpression::QExpression(const QString &expression)
{
    m_expression = expression;
}

bool QExpression::eval()
{
    if (m_expression.isEmpty())
    {
        m_error = NoExpression;
        return false;
    }
    normalize();
    if (!toReversePolishNotation())
    {
        return false;
    }
    QStringList operands = m_reversePolishNotation.split(" ");

    QStack<double> stack;
    for (const auto &operand : operands)
    {
        if (m_functions.hasOperator(operand))
        {
            const auto *op = m_functions.op(operand);
            if (stack.size() < op->numberOfArguments())
            {
                m_error = WrongNumberOfArguments;
                return false;
            }
            switch (op->type())
            {
            case BaseFunction::UnaryOperator:
            {
                const UnaryOperator *un = static_cast<const UnaryOperator *>(op);
                double value = un->function()(stack.pop());
                stack.push(value);
            }
            break;
            case BaseFunction::BinaryOperator:
            {
                const BinaryOperator *bi = static_cast<const BinaryOperator *>(op);
                double value = bi->function()(stack.pop(), stack.pop());
                stack.push(value);
            }
            break;
            case BaseFunction::OtherFunction:
            {
                const Function *fn = static_cast<const Function *>(op);
                QList<double> params;
                for (int i = 0; i < fn->numberOfArguments(); i++)
                {
                    params.append(stack.pop());
                }
                double value = fn->function()(params);
                stack.push(value);
            }
            break;

            default:
                break;
            }
        }
        else
        {
            stack.push(operand.toDouble());
        }
    }
    m_result = stack.pop();
    m_error = NoError;
    return true;
}

bool QExpression::toReversePolishNotation()
{
    QString result;
    result.reserve(m_expression.size());
    QStringList symbols = m_expression.split(QRegularExpression("[ ,]+"));
    QStack<QString> stack;
    for (const auto &symbol : symbols)
    {
        if (symbol.isEmpty())
        {
            continue;
        }
        if (symbol == "(")
        {
            stack.push(symbol);
            continue;
        }
        if (symbol == ")")
        {
            bool otherBracketFound = false;
            while (!stack.isEmpty())
            {
                QString el = stack.pop();
                if (el != "(")
                {
                    result += el + " ";
                }
                else
                {
                    otherBracketFound = true;
                    break;
                }
            }
            if (!otherBracketFound)
            {
                m_error = UnexpectedEnd;
                return false;
            }
            continue;
        }
        if (m_functions.hasOperator(symbol))
        {
            if (stack.isEmpty())
            {
                stack.push(symbol);
                continue;
            }
            else
            {
                int curOPriority = m_functions.priority(symbol);
                while (curOPriority <= m_functions.priority(stack.top()))
                {
                    result += stack.pop() + " ";
                    if (stack.isEmpty())
                    {
                        break;
                    }
                }
                stack.push(symbol);
                continue;
            }
        }
        if (m_variables.contains(symbol))
        {
            result += QString::number(m_variables[symbol]) + " ";
            continue;
        }
        bool ok;
        symbol.toDouble(&ok);
        if (ok)
        {
            result += symbol + " ";
            continue;
        }
        m_error = UnknownIdentifier;
        qWarning() << symbol;
        return false;
    }
    while (!stack.isEmpty())
    {
        QString symbol = stack.pop();
        if (symbol == "(" || symbol == ")")
        {
            m_error = UnexpectedEnd;
            return false;
        }
        result += symbol + " ";
    }
    result.remove(result.length() - 1, 1);
    m_reversePolishNotation = result;
    return true;
}

void QExpression::normalize()
{
    for (int i = 0; i < m_functions.size(); ++i)
    {
        const BaseFunction *func = m_functions.at(i);
        int index = m_expression.indexOf(func->expr(), 0);
        while (index != -1)
        {
            m_expression.insert(index, " ");
            m_expression.insert(index + 1 + func->expr().size(), " ");
            index = m_expression.indexOf(func->expr(), index + 1 + func->expr().size());
        }
    }
}

FunctionList::FunctionList()
{
    // To add custom operator just create it here, and then put it to the list.
    BinaryOperator *sum = new BinaryOperator("+", 1, [](double a, double b)
                                             { return a + b; });
    BinaryOperator *diff = new BinaryOperator("-", 1, [](double a, double b)
                                              { return a - b; });
    BinaryOperator *mul = new BinaryOperator("*", 2, [](double a, double b)
                                             { return a * b; });
    BinaryOperator *div = new BinaryOperator("/", 2, [](double a, double b)
                                             { return a / b; });
    BinaryOperator *exp = new BinaryOperator(
        "^", 3, [](double a, double b)
        { return pow(a, b); });
    UnaryOperator *fact = new UnaryOperator("!", 4, [](double a)
                                            {
        Q_ASSERT(floor(a) == a);
        double result = 1.0;
        for (int i = 2; i <= a; i++) {
            result *= i;
        }
        return result; });
    
    BinaryOperator *openBracket = new BinaryOperator("(", 0, Q_NULLPTR);
    BinaryOperator *closeBracket = new BinaryOperator(")", 0, Q_NULLPTR);

    UnaryOperator *sinFunc = new UnaryOperator("sin", 4, [](double a)
                                               { return sin(a); });
    UnaryOperator *cosFunc = new UnaryOperator("cos", 4, [](double a)
                                               { return cos(a); });
    Function *sqrtFunc = new Function("sqrt", 1, [](const QList<double> &list)
                                      { return sqrt(list.first()); });
    // Can be deleted, used in test scenario.
    Function *maxOfThree = new Function("max_of_three", 3, [](const QList<double> &list)
                                        { return qMax(qMax(list[0], list[1]), list[2]); });

    m_funclist.append(sum);
    m_funclist.append(diff);
    m_funclist.append(mul);
    m_funclist.append(div);
    m_funclist.append(sinFunc);
    m_funclist.append(cosFunc);
    m_funclist.append(exp);
    m_funclist.append(sqrtFunc);
    m_funclist.append(maxOfThree);
    m_funclist.append(fact);
    m_funclist.append(openBracket);
    m_funclist.append(closeBracket);
}

FunctionList::~FunctionList() { qDeleteAll(m_funclist); }

const BaseFunction *FunctionList::at(int i) const
{
    return m_funclist.at(i);
}

const BaseFunction *FunctionList::op(const QString &signature) const
{
    for (const BaseFunction *op : m_funclist)
    {
        if (op->expr() == signature)
            return op;
    }
    return Q_NULLPTR;
}

int FunctionList::size() const { return m_funclist.size(); }

int FunctionList::priority(const QString &signature) const
{
    for (const BaseFunction *op : m_funclist)
    {
        if (op->expr() == signature)
            return op->priority();
    }
    return -1;
}

bool FunctionList::hasOperator(const QString &signature) const
{
    for (const BaseFunction *op : m_funclist)
    {
        if (op->expr() == signature)
            return true;
    }
    return false;
}

BaseFunction::BaseFunction(int priority, int numberOfArguments,
                           BaseFunction::FunctionType type,
                           const QString &name)
    : m_priority(priority), m_args(numberOfArguments),
      m_type(type), m_expression(name)
{
}

BinaryOperator::BinaryOperator(const QString &name, int priority,
                               const BinaryOperatorFunction &function)
    : BaseFunction(priority, 2, FunctionType::BinaryOperator, name),
      m_func(function)
{
}

BinaryOperatorFunction BinaryOperator::function() const { return m_func; }

Function::Function(const QString &name, int numberOfArguments,
                   const CustomFunction &function)
    : BaseFunction(1, numberOfArguments, BaseFunction::OtherFunction, name),
      m_func(function)
{
}

CustomFunction Function::function() const { return m_func; }

UnaryOperator::UnaryOperator(const QString &name, int priority,
                             const UnaryOperatorFunction &function)
    : BaseFunction(priority, 1, FunctionType::UnaryOperator, name),
      m_func(function)
{
}

UnaryOperatorFunction UnaryOperator::function() const { return m_func; }