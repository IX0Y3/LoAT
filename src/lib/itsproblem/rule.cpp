#include "rule.hpp"

ConsHash<Rule, Rule, Rule::CacheHash, Rule::CacheEqual, Bools::Expr, Subs> Rule::cache;

unsigned Rule::next_id {0};

Rule::Rule(const Bools::Expr guard, const Subs &update): guard(guard), update(update), id(next_id++) {}

size_t Rule::CacheHash::operator()(const std::tuple<Bools::Expr, Subs> &args) const noexcept {
    size_t seed {42};
    boost::hash_combine(seed, std::get<0>(args));
    boost::hash_combine(seed, std::get<1>(args));
    return seed;
}

bool Rule::CacheEqual::operator()(const std::tuple<Bools::Expr, Subs> &args1, const std::tuple<Bools::Expr, Subs> &args2) const noexcept {
    return args1 == args2;
}

Rule::~Rule() {
    cache.erase(guard, update);
}

RulePtr Rule::mk(const Bools::Expr guard, const Subs up) {
    return cache.from_cache(guard, up);
}

void Rule::collectVars(VarSet &vars) const {
    guard->collectVars(vars);
    update.collectVars(vars);
}

VarSet Rule::vars() const {
    VarSet res;
    collectVars(res);
    return res;
}

RulePtr Rule::subs(const Subs &subs) const {
    return mk(subs(guard), update.concat(subs));
}

RulePtr Rule::renameVars(const Renaming &subs) const {
    return mk(subs(guard), update.concat(subs));
}

RulePtr Rule::withGuard(const Bools::Expr guard) const {
    return mk(guard, update);
}

RulePtr Rule::withUpdate(const Subs &update) const {
    return mk(guard, update);
}

RulePtr Rule::chain(const RulePtr &that) const {
    return mk(guard && update(that->getGuard()), that->getUpdate().compose(update));
}

const Bools::Expr Rule::getGuard() const {
    return guard;
}

const Subs& Rule::getUpdate() const {
    return update;
}

unsigned Rule::getId() const {
    return id;
}

std::ostream& operator<<(std::ostream &s, const Rule &rule) {
    s << rule.getId() << ": ";
    s << rule.getGuard();
    s << " /\\ ";
    bool first = true;
    for (const auto &[x,v] : rule.getUpdate()) {
        if (first) {
            first = false;
        } else {
            s << ", ";
        }
        s << x << "'";
        s << " = " << v;
    }
    return s;
}

bool Rule::isPoly() const {
    return guard->isPoly() && update.isPoly();
}

std::ostream& operator<<(std::ostream &s, const RulePtr &idx) {
    return s << idx->getId();
}

std::ostream& operator<<(std::ostream &s, const Implicant &imp) {
    s << imp.first->getId() << ": ";
    const auto &up {imp.first->getUpdate()};
    if (imp.second != top()) {
        s << imp.second;
        if (!up.empty()) {
            s << " /\\ " << up;
        }
    } else if (!up.empty()) {
        s << up;
    } else {
        s << top();
    }
    return s;
}

bool Rule::isDeterministic() const {
    const auto vs {vars()};
    return !std::any_of(vs.begin(), vs.end(), theory::isTempVar);
}

size_t Rule::hash() const {
    size_t hash {0};
    boost::hash_combine(hash, std::hash<Bools::Expr>{}(guard));
    boost::hash_combine(hash, update.hash());
    return hash;
}

size_t hash_value(const Rule &r) {
    return r.hash();
}

RulePtr Rule::renameTmpVars() const {
    Renaming s;
    for (const auto &x: vars()) {
        if (theory::isTempVar(x)) {
            s.insert(x, theory::next(x));
        }
    }
    return renameVars(s);
}
