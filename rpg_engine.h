#ifndef DANILKA_RPG_ENGINE_H
#define DANILKA_RPG_ENGINE_H

#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <queue>
#include <random>
#include <chrono>
#include <cstdlib>
#include <ctime>

using namespace std;

/**
 * @brief Простіший клас для логування повідомлень.
 *
 * Дозволяє виводити інформаційні, попереджувальні та помилкові повідомлення.
 * Використовується усіма класами системи.
 * 
 */
class Logger {
public:
    enum Level { INFO, WARN, ERROR };
private:
    Level level;
public:
    Logger(Level l = INFO) : level(l) {}
    void log(const string& msg, Level l = INFO) {
        if (l >= level) {
            
        }
    }
    static string levelName(Level l) {
        switch (l) {
        case INFO: return "INFO";
        case WARN: return "WARN";
        default: return "ERROR";
        }
    }
};

class Item {
    string name;
    int value;
public:
    Item(string n = "", int v = 0) : name(n), value(v) {}
    string getName() const { return name; }
    int getValue() const { return value; }
    string str() const {
        return name + "($" + to_string(value) + ")";
    }
};

/**
 * @brief Шаблонний інвентар для зберігання предметів.
 *
 * @tparam T тип об'єкта інвентаря (наприклад Item) 
 */
template<typename T>
class Inventory {
    vector<T> items;
    size_t capacity;
public:
    Inventory(size_t cap = 10) : capacity(cap) {}
    bool add(const T& it) {
        if (items.size() >= capacity) return false;
        items.push_back(it);
        return true;
    }
    bool removeIfName(const string& n) {
        auto it = remove_if(items.begin(), items.end(), [&](const T& it) { return it.getName() == n; });
        if (it == items.end()) return false;
        items.erase(it, items.end());
        return true;
    }
    T* findByName(const string& n) {
        for (auto& it : items)
            if (it.getName() == n) return &it;
        return nullptr;
    }
    vector<T> snapshot() const { return items; } 
    string toString() const {
        string s = "Inventory(" + to_string(items.size()) + "/" + to_string(capacity) + "): ";
        for (auto& it : items) s += it.str() + " ";
        return s;
    }
    size_t getSize() const { return items.size(); } 
};

class Character;
class Skill;

/**
 * @brief Базовий клас навички персонажа.
 *
 * Містить назву, рівень та базову силу навички.
 * Від нього успадковуються активні, пасивні та ультимативні навички.
 * 
 */
class Skill {
protected:
    string name;
    int level;          
    int basePower;
public:
    Skill(const string& n, int p = 10) : name(n), level(1), basePower(p) {}
    virtual ~Skill() = default;
    /**
     * @brief Повертає ефективну силу навички.
     * @return сила з урахуванням рівня 
     */
    virtual int effectivePower() const {
        return basePower + level * 3;
    }
    /**
     * @brief @brief Повертає текстове описання навички.
     * @return рядок з описом 
     */
    virtual string description() const {
        return name + " (lvl " + to_string(level) + ", pwr " + to_string(effectivePower()) + ")";
    }
    /**
     * @brief Застосовує навичку до персонажа.
     * @param target персонаж, до якого застосовується навичка
     * @throws нічого 
     */
    virtual void apply(Character& target) = 0; // abstract action on target
    virtual void upgrade() {
        level++;
        basePower = basePower + 2;
    }
    string getName() const { return name; }
    int getLevel() const { return level; } 
};

class ActiveSkill : public Skill {
    int manaCost;
public:
    ActiveSkill(const string& n, int p = 12, int cost = 10) : Skill(n, p), manaCost(cost) {}
    int effectivePower() const override {
        return basePower + level * 5;
    }
    string description() const override {
        return "Active: " + Skill::description() + " mana:" + to_string(manaCost);
    }
    void apply(Character& target) override;
    int getManaCost() const { return manaCost; } // Додано для тестів
};

class PassiveSkill : public Skill {
    double modifier; // e.g., increases defense or attack by percentage
public:
    PassiveSkill(const string& n, int p = 5, double mod = 0.05) : Skill(n, p), modifier(mod) {}
    int effectivePower() const override {
        return basePower + static_cast<int>(level * (modifier * 100));
    }
    string description() const override {
        return "Passive: " + Skill::description() + " mod: " + to_string(modifier);
    }
    void apply(Character& target) override; // will modify stats passively
};

class UltimateSkill : public ActiveSkill {
    int cooldown;
public:
    UltimateSkill(const string& n, int p = 30, int cost = 30, int cd = 3) : ActiveSkill(n, p, cost), cooldown(cd) {}
    int effectivePower() const override {
        return basePower + level * 12;
    }
    string description() const override {
        return "Ultimate: " + Skill::description() + " cd:" + to_string(cooldown);
    }
    void apply(Character& target) override;
};

class SkillTreeNode {
    Skill* skill;
    SkillTreeNode* parent;
    vector<unique_ptr<SkillTreeNode>> children;
public:
    SkillTreeNode(Skill* s = nullptr, SkillTreeNode* p = nullptr) : skill(s), parent(p) {}
    ~SkillTreeNode() { /* skill ownership is external (managed elsewhere) */ }

    Skill* getSkill() const { return skill; }
    SkillTreeNode* getParent() const { return parent; }

    SkillTreeNode* addChild(Skill* s) {
        children.push_back(make_unique<SkillTreeNode>(s, this));
        return children.back().get();
    }
    bool removeChildWithSkillName(const string& n) {
        auto it = remove_if(children.begin(), children.end(),
            [&](const unique_ptr<SkillTreeNode>& c) { return c->skill && c->skill->getName() == n; });
        if (it == children.end()) return false;
        children.erase(it, children.end());
        return true;
    }

    template<typename F>
    void dfs(F f) {
        f(this);
        for (auto& ch : children) ch->dfs(f);
    }

    vector<SkillTreeNode*> getChildrenRaw() {
        vector<SkillTreeNode*> out;
        for (auto& c : children) out.push_back(c.get());
        return out;
    }
};

/**
 * @brief Шаблонне дерево навичок.
 *
 * Дозволяє додавати навички у вигляді дерева,
 * виконувати DFS-обхід та генерацію випадкового дерева.
 *
 * @tparam T тип даних у вузлах (зазвичай Skill*)
 */
template<typename T>
class SkillTree {
    unique_ptr<SkillTreeNode> root;
public:
    SkillTree() : root(nullptr) {}
    SkillTree(Skill* s) { root = make_unique<SkillTreeNode>(s, nullptr); }

    SkillTreeNode* getRoot() const { return root.get(); }

    SkillTreeNode* insertUnder(const string& parentSkillName, Skill* s) {
        if (!root) {
            root = make_unique<SkillTreeNode>(s, nullptr);
            return root.get();
        }
        SkillTreeNode* found = findNodeBySkillName(parentSkillName);
        if (!found) return nullptr;
        return found->addChild(s);
    }

    /**
     * @brief Пошук вузла за назвою навички.
     * @param name назва навички
     * @return вказівник на вузол або nullptr 
     */
    SkillTreeNode* findNodeBySkillName(const string& name) const {
        if (!root) return nullptr;
        SkillTreeNode* result = nullptr;
        root->dfs([&](SkillTreeNode* node) {
            if (node->getSkill() && node->getSkill()->getName() == name) result = node;
            });
        return result;
    }

    vector<string> descriptionsDFS() const {
        vector<string> out;
        if (!root) return out;
        root->dfs([&](SkillTreeNode* node) {
            if (node->getSkill()) out.push_back(node->getSkill()->description());
            });
        return out;
    }
    void generateRandom(Skill* (*skillFactory)(), int maxDepth = 3, int maxChildren = 3) {
        root = make_unique<SkillTreeNode>(skillFactory(), nullptr);
        default_random_engine rng((unsigned)chrono::high_resolution_clock::now().time_since_epoch().count());
        queue<pair<SkillTreeNode*, int>> q;
        q.push({ root.get(), 1 });
        while (!q.empty()) {
            auto [node, depth] = q.front(); q.pop();
            if (depth >= maxDepth) continue;
            uniform_int_distribution<int> distChildren(0, maxChildren);
            int nc = distChildren(rng);
            for (int i = 0;i < nc;i++) {
                Skill* s = skillFactory();
                SkillTreeNode* child = node->addChild(s);
                q.push({ child, depth + 1 });
            }
        }
    }
};

/* --------------------- Character hierarchy --------------------- */
/**
 * @brief Базовий клас для всіх персонажів гри. 
 *  
 * Містить спільні характеристики — HP, mana, attack, defense.
 * Підтримує атаки, використання навичок та інвентар.
 */
class Character {
protected:
    string name;
    int hp;
    int mana;
    int attackPower;
    int defense;
    int level;
    vector<unique_ptr<Skill>> ownedSkills; // owns some skills
    Inventory<Item> inventory; // composition of template Inventory
    Logger& logger;
public:
    Character(const string& n, Logger& log)
        : name(n), hp(100), mana(50), attackPower(10), defense(5), level(1), inventory(10), logger(log) {
    }

    virtual ~Character() = default;

    /**
     * @brief Атакує іншого персонажа.
     * @param target ціль атаки
     * @return нанесена шкода
     */
    virtual int attack(Character& target) {
        int raw = attackPower + level * 2;
        int variance = rand() % (level + 3);
        int dmg = max(0, raw + variance - target.getDefense());
        target.takeDamage(dmg);
        logger.log(name + " attacks " + target.getName() + " for " + to_string(dmg) + " dmg.");
        return dmg;
    }

    virtual void useSkill(size_t idx, Character& target) {
        if (idx >= ownedSkills.size()) {
            logger.log(name + " tried to use invalid skill index.", Logger::WARN);
            return;
        }
        Skill* sk = ownedSkills[idx].get();
        if (!sk) return;
        logger.log(name + " uses " + sk->getName() + " on " + target.getName());
        sk->apply(target); // dynamic dispatch
    }

    virtual void equipSkill(unique_ptr<Skill> s) {
        if (!s) return;
        logger.log(name + " equips skill " + s->getName());
        ownedSkills.push_back(move(s));
    }

    virtual void levelUp() {
        level++;
        hp += 10;
        mana += 5;
        attackPower += 2;
        defense += 1;
        logger.log(name + " leveled up to " + to_string(level));
    }

    virtual void takeDamage(int d) {
        hp -= d;
        if (hp < 0) hp = 0;
    }

    virtual int getHP() const { return hp; }
    virtual int getMana() const { return mana; }
    virtual int getDefense() const { return defense; }
    string getName() const { return name; }
    int getLevel() const { return level; } // Додано для тестів
    int getAttackPower() const { return attackPower; } // Додано для тестів

    virtual string status() const {
        return name + " (lvl " + to_string(level) + ") HP:" + to_string(hp) + " MP:" + to_string(mana);
    }

    virtual int overallPower() const {
        int p = attackPower + level * 3;
        for (auto& s : ownedSkills) p += s->effectivePower() / 2;
        return p;
    }

    Inventory<Item>& getInventory() { return inventory; }
    size_t skillCount() const { return ownedSkills.size(); }
};

/* Derived classes: Warrior, Mage, Archer */
class Warrior : public Character {
    int rage;
public:
    Warrior(const string& n, Logger& log) : Character(n, log), rage(0) {
        attackPower += 5;
        defense += 3;
    }
    int attack(Character& target) override {
        rage = min(100, rage + 10);
        int base = Character::attack(target);
        if (rage >= 50) {
            int bonus = 5 + level;
            target.takeDamage(bonus);
            logger.log(name + " uses RAGE bonus for " + to_string(bonus) + " extra dmg!");
            rage = 0;
            return base + bonus;
        }
        return base;
    }
    void battleShout() {
        attackPower += 2;
        logger.log(name + " shouts and increases attack!");
    }
};

class Mage : public Character {
    int spellPower;
public:
    Mage(const string& n, Logger& log) : Character(n, log), spellPower(10) {
        mana += 30;
    }
    void useSkill(size_t idx, Character& target) override {
        if (idx >= skillCount()) { logger.log("Invalid skill idx", Logger::WARN); return; }
        Skill* sk = ownedSkills[idx].get();
        if (!sk) return;

        int cost = 0;
        if (auto active_sk = dynamic_cast<ActiveSkill*>(sk)) {
             cost = active_sk->getManaCost();
        } else {
             cost = max(5, sk->effectivePower() / 3); 
        }

        if (mana < cost) {
            logger.log(name + " doesn't have enough mana (" + to_string(mana) + ") to cast " + sk->getName(), Logger::WARN);
            return;
        }
        mana -= cost;
        logger.log(name + " casts " + sk->getName() + " costing " + to_string(cost) + " mana.");
        sk->apply(target);
    }
    int getSpellPower() const { return spellPower; }
};

class Archer : public Character {
    int agility;
public:
    Archer(const string& n, Logger& log) : Character(n, log), agility(12) {
        attackPower += 2;
    }
    int attack(Character& target) override {
        int chance = min(50, agility + level);
        int r = rand() % 100;
        if (r < chance) {
            int dmg = Character::attack(target) + 7;
            logger.log(name + " lands a CRITICAL hit!");
            return dmg;
        }
        else {
            return Character::attack(target);
        }
    }
    void dodge() {
        defense += 2;
        logger.log(name + " prepares to dodge, defense increased temporarily.");
    }
};

void ActiveSkill::apply(Character& target) {
    int p = effectivePower();
    int variance = rand() % 5;
    int dmg = max(1, p + variance - target.getDefense());
    target.takeDamage(dmg);
    // cout << "ActiveSkill " << name << " applied to " << target.getName() << " for " << dmg << " damage\n";
}

void PassiveSkill::apply(Character& target) {
    // В юніт-тестах пасивні навички зазвичай перевіряються через їхній
    // вплив на overallPower або через зміну статів (якщо б вони могли їх міняти)
    // cout << "PassiveSkill " << name << " applied to " << target.getName() << " (passive buff)\n";
}

void UltimateSkill::apply(Character& target) {
    int p = effectivePower();
    int dmg = max(5, p - target.getDefense());
    target.takeDamage(dmg);
    // cout << "UltimateSkill " << name << " strikes " << target.getName() << " for " << dmg << " massive damage!\n";
}

class Party {
    vector<unique_ptr<Character>> members;
    Logger& logger;
public:
    Party(Logger& log) : logger(log) {}
    void addMember(unique_ptr<Character> c) {
        logger.log("Adding member " + c->getName());
        members.push_back(move(c));
    }
    Character* getMember(size_t idx) {
        if (idx >= members.size()) return nullptr;
        return members[idx].get();
    }
    size_t size() const { return members.size(); }

    int combinedPower() const {
        int sum = 0;
        for (auto& m : members) sum += m->overallPower();
        return sum;
    }

    void showStatus() const {
        logger.log("Party status:");
        for (auto& m : members) cout << "  " << m->status() << "\n";
    }
};

class BattleSimulator {
    Logger& logger;
public:
    BattleSimulator(Logger& log) : logger(log) {}
    void simulate(Party& a, Party& b) {
        logger.log("Battle starts between two parties!");
        size_t turn = 0;
        while (turn < 50) {
            if (allDead(a)) { logger.log("Party A defeated!"); return; }
            if (allDead(b)) { logger.log("Party B defeated!"); return; }

            Character* ca = randomAlive(a);
            Character* cb = randomAlive(b);
            if (!ca || !cb) break;

            if (turn % 2 == 0) {
                ca->attack(*cb);
            }
            else {
                cb->attack(*ca);
            }
            turn++;
        }
        logger.log("Battle ended after max turns.");
    }

private:
    bool allDead(Party& p) {
        for (size_t i = 0;i < p.size();++i) {
            Character* c = p.getMember(i);
            if (c && c->getHP() > 0) return false;
        }
        return true;
    }
    Character* randomAlive(Party& p) {
        vector<Character*> alive;
        for (size_t i = 0;i < p.size();++i) {
            Character* c = p.getMember(i);
            if (c && c->getHP() > 0) alive.push_back(c);
        }
        if (alive.empty()) return nullptr;
        return alive[rand() % alive.size()];
    }
};

Skill* randomSkillFactory() {
    static int counter = 0;
    counter++;
    int t = rand() % 3;
    if (t == 0) return new ActiveSkill("Active_" + to_string(counter), 10 + (counter % 5));
    if (t == 1) return new PassiveSkill("Passive_" + to_string(counter), 5 + (counter % 3));
    return new UltimateSkill("Ult_" + to_string(counter), 25 + (counter % 8));
}

#endif //DANILKA_RPG_ENGINE_H