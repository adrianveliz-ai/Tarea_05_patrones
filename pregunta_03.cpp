#include <cmath>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
using namespace std;

class Expression;
using ExprPtr = shared_ptr<Expression>;

ExprPtr num(double v);
ExprPtr makeAdd(ExprPtr a, ExprPtr b);
ExprPtr makeSub(ExprPtr a, ExprPtr b);
ExprPtr makeMul(ExprPtr a, ExprPtr b);
ExprPtr makeDiv(ExprPtr a, ExprPtr b);
ExprPtr makePow(ExprPtr a, ExprPtr b);


class Expression {
public:
    virtual ~Expression() = default;
    virtual double evaluate(double x) const = 0;
    virtual ExprPtr derivative() const = 0;
    virtual string toString() const = 0;
    virtual bool isConstant() const = 0;   // no depende de x
};


class Constant : public Expression {
public:
    explicit Constant(double v) : value_(v) {}
    double evaluate(double) const override { return value_; }
    ExprPtr derivative() const override { return num(0); }
    string toString() const override {
        ostringstream os; os << value_; return os.str();
    }
    bool isConstant() const override { return true; }
private:
    double value_;
};

class Variable : public Expression {
public:
    double evaluate(double x) const override { return x; }
    ExprPtr derivative() const override { return num(1); }
    string toString() const override { return "x"; }
    bool isConstant() const override { return false; }
};


class BinaryOperation : public Expression {
public:
    BinaryOperation(ExprPtr l, ExprPtr r, string symbol)
        : left_(move(l)), right_(move(r)), symbol_(move(symbol)) {}

    double evaluate(double x) const override {
        return apply(left_->evaluate(x), right_->evaluate(x));
    }
    string toString() const override {
        return "(" + left_->toString() + " " + symbol_ + " " + right_->toString() + ")";
    }
    bool isConstant() const override { return left_->isConstant() && right_->isConstant(); }

protected:
    virtual double apply(double a, double b) const = 0;
    ExprPtr left_, right_;
    string symbol_;
};


class Add : public BinaryOperation {
public:
    Add(ExprPtr l, ExprPtr r) : BinaryOperation(move(l), move(r), "+") {}
    ExprPtr derivative() const override {                       // (u+v)' = u' + v'
        return makeAdd(left_->derivative(), right_->derivative());
    }
protected:
    double apply(double a, double b) const override { return a + b; }
};

class Subtract : public BinaryOperation {
public:
    Subtract(ExprPtr l, ExprPtr r) : BinaryOperation(move(l), move(r), "-") {}
    ExprPtr derivative() const override {                       // (u-v)' = u' - v'
        return makeSub(left_->derivative(), right_->derivative());
    }
protected:
    double apply(double a, double b) const override { return a - b; }
};


class Multiply : public BinaryOperation {
public:
    Multiply(ExprPtr l, ExprPtr r) : BinaryOperation(move(l), move(r), "*") {}
    ExprPtr derivative() const override {                       // (uv)' = u'v + uv'
        return makeAdd(makeMul(left_->derivative(), right_),
                       makeMul(left_, right_->derivative()));
    }
protected:
    double apply(double a, double b) const override { return a * b; }
};

class Divide : public BinaryOperation {
public:
    Divide(ExprPtr l, ExprPtr r) : BinaryOperation(move(l), move(r), "/") {}
    ExprPtr derivative() const override {                       // (u/v)' = (u'v - uv') / v^2
        return makeDiv(makeSub(makeMul(left_->derivative(), right_),
                               makeMul(left_, right_->derivative())),
                       makePow(right_, num(2)));
    }
protected:
    double apply(double a, double b) const override { return a / b; }
};

class Power : public BinaryOperation {
public:
    Power(ExprPtr l, ExprPtr r) : BinaryOperation(move(l), move(r), "^") {}
    ExprPtr derivative() const override;
protected:
    double apply(double a, double b) const override { return pow(a, b); }
};


class UnaryFunction : public Expression {
public:
    UnaryFunction(ExprPtr operand, string name)
        : operand_(move(operand)), name_(move(name)) {}

    double evaluate(double x) const override { return apply(operand_->evaluate(x)); }

    // Regla de la cadena: (f(u))' = f'(u) * u'
    ExprPtr derivative() const override { return makeMul(outerDerivative(), operand_->derivative()); }
    string toString() const override { return name_ + "(" + operand_->toString() + ")"; }
    bool isConstant() const override { return operand_->isConstant(); }

protected:
    virtual double apply(double u) const = 0;
    virtual ExprPtr outerDerivative() const = 0;
    ExprPtr operand_;
    string name_;
};

class Sin : public UnaryFunction {
public:
    explicit Sin(ExprPtr u) : UnaryFunction(move(u), "sin") {}
protected:
    double apply(double u) const override { return sin(u); }
    ExprPtr outerDerivative() const override;
};

class Cos : public UnaryFunction {
public:
    explicit Cos(ExprPtr u) : UnaryFunction(move(u), "cos") {}
protected:
    double apply(double u) const override { return cos(u); }
    ExprPtr outerDerivative() const override { return makeMul(num(-1), make_shared<Sin>(operand_)); }
};

class Exp : public UnaryFunction {
public:
    explicit Exp(ExprPtr u) : UnaryFunction(move(u), "exp") {}
protected:
    double apply(double u) const override { return exp(u); }
    ExprPtr outerDerivative() const override { return make_shared<Exp>(operand_); }
};

class Ln : public UnaryFunction {
public:
    explicit Ln(ExprPtr u) : UnaryFunction(move(u), "ln") {}
protected:
    double apply(double u) const override { return log(u); }
    ExprPtr outerDerivative() const override { return makeDiv(num(1), operand_); }
};

ExprPtr Sin::outerDerivative() const { return make_shared<Cos>(operand_); }

ExprPtr Power::derivative() const {
    if (right_->isConstant()) {
        double n = right_->evaluate(0);
        return makeMul(makeMul(num(n), makePow(left_, num(n - 1))), left_->derivative());
    }
    return makeMul(make_shared<Power>(left_, right_),
                   makeAdd(makeMul(right_->derivative(), make_shared<Ln>(left_)),
                           makeDiv(makeMul(right_, left_->derivative()), left_)));
}


static bool isNum(const ExprPtr& e, double v) { return e->isConstant() && e->evaluate(0) == v; }

ExprPtr num(double v) { return make_shared<Constant>(v); }

ExprPtr makeAdd(ExprPtr a, ExprPtr b) {
    if (isNum(a, 0)) return b;
    if (isNum(b, 0)) return a;
    if (a->isConstant() && b->isConstant()) return num(a->evaluate(0) + b->evaluate(0));
    return make_shared<Add>(a, b);
}
ExprPtr makeSub(ExprPtr a, ExprPtr b) {
    if (isNum(b, 0)) return a;
    if (a->isConstant() && b->isConstant()) return num(a->evaluate(0) - b->evaluate(0));
    return make_shared<Subtract>(a, b);
}
ExprPtr makeMul(ExprPtr a, ExprPtr b) {
    if (isNum(a, 0) || isNum(b, 0)) return num(0);
    if (isNum(a, 1)) return b;
    if (isNum(b, 1)) return a;
    if (a->isConstant() && b->isConstant()) return num(a->evaluate(0) * b->evaluate(0));
    return make_shared<Multiply>(a, b);
}
ExprPtr makeDiv(ExprPtr a, ExprPtr b) {
    if (isNum(a, 0)) return num(0);
    if (isNum(b, 1)) return a;
    return make_shared<Divide>(a, b);
}
ExprPtr makePow(ExprPtr a, ExprPtr b) {
    if (isNum(b, 0)) return num(1);
    if (isNum(b, 1)) return a;
    return make_shared<Power>(a, b);
}


int main() {
    auto x = make_shared<Variable>();
    auto expr =
        make_shared<Multiply>(
            make_shared<Add>(
                make_shared<Power>(x, make_shared<Constant>(2)),
                make_shared<Multiply>(make_shared<Constant>(3), x)
            ),
            make_shared<Sin>(x)
        );

    auto deriv = expr->derivative();
    cout << "f(x) = " << expr->toString() << endl;
    cout << "f'(x) = " << deriv->toString() << endl;
    cout << "f(2) = " << expr->evaluate(2) << endl;

    auto g = make_shared<Divide>(
        make_shared<Exp>(make_shared<Sin>(make_shared<Power>(x, num(2)))),
        make_shared<Ln>(x));
    auto gd = g->derivative();
    cout << "\ng(x) = " << g->toString() << endl;
    cout << "g'(x) = " << gd->toString() << endl;

    const double x0 = 2.0, h = 1e-6;
    for (const auto& e : {static_pointer_cast<Expression>(expr),
                          static_pointer_cast<Expression>(g)}) {
        double numeric = (e->evaluate(x0 + h) - e->evaluate(x0 - h)) / (2 * h);
        cout << "check: simbolica=" << e->derivative()->evaluate(x0)
                  << " numerica=" << numeric << endl;
    }

    auto p = make_shared<Power>(x, x);
    cout << "\n(x^x)' = " << p->derivative()->toString() << endl;

    return 0;
}