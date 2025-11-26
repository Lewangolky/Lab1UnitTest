/**
 * @file rpg_engine.h
 * @brief Головний файл двигуна рольової гри (RPG Engine).
 *
 * Містить визначення класів для персонажів, навичок, інвентаря,
 * логування та симуляції бою.
 *
 * @author Danilka
 * @date 2025
 */

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
 * @class Logger
 * @brief Клас для логування подій у системі.
 *
 * Дозволяє виводити повідомлення різних рівнів важливості (INFO, WARN, ERROR).
 */
class Logger {
public:
    /**
     * @enum Level
     * @brief Рівні важливості повідомлень.
     */
    enum Level { INFO, WARN, ERROR };

private:
    Level level; ///< Поточний рівень логування

public:
    /**
     * @brief Конструктор логера.
     * @param l Мінімальний рівень повідомлень для виводу.
     */
    Logger(Level l = INFO) : level(l) {}

    /**
     * @brief Записує повідомлення в лог.
     * @param msg Текст повідомлення.
     * @param l Рівень важливості повідомлення.
     */
    void log(const string& msg, Level l = INFO) {
        if (l >= level) {
            // У тестах вивід може бути небажаним, тому можна його вимкнути
            // cout << "[" << levelName(l) << "] " << msg << "\n";
        }
    }

    /**
     * @brief Перетворює enum Level у рядок.
     * @param l Рівень.
     * @return Рядок ("INFO", "WARN" тощо).
     */
    static string levelName(Level l) {
        switch (l) {
        case INFO: return "INFO";
        case WARN: return "WARN";
        default: return "ERROR";
        }
    }
};

/**
 * @class Item
 * @brief Предмет у грі.
 *
 * Має назву та цінність.
 */
class Item {
    string name; ///< Назва предмету
    int value;   ///< Вартість предмету
public:
    /**
     * @brief Конструктор предмету.
     * @param n Назва.
     * @param v Вартість.
     */
    Item(string n = "", int v = 0) : name(n), value(v) {}
    string getName() const { return name; }
    int getValue() const { return value; }
    
    /**
     * @brief Отримати строкове представлення предмету.
     * @return Рядок формату "Name($Value)".
     */
    string str() const {
        return name + "($" + to_string(value) + ")";
    }
};

/**
 * @class Inventory
 * @brief Шаблонний клас інвентаря.
 * @tparam T Тип предметів, що зберігаються (зазвичай Item).
 */
template<typename T>
class Inventory {
    vector<T> items; ///< Список предметів
    size_t capacity; ///< Максимальна місткість
public:
    /**
     * @brief Конструктор інвентаря.
     * @param cap Максимальна кількість предметів (за замовчуванням 10).
     */
    Inventory(size_t cap = 10) : capacity(cap) {}

    /**
     * @brief Додати предмет в інвентар.
     * @param it Предмет для додавання.
     * @return true, якщо додано успішно, false, якщо інвентар повний.
     */
    bool add(const T& it) {
        if (items.size() >= capacity) return false;
        items.push_back(it);
        return true;
    }

    /**
     * @brief Видалити предмет за назвою.
     * @param n Назва предмету.
     * @return true, якщо предмет знайдено і видалено.
     */
    bool removeIfName(const string& n) {
        auto it = remove_if(items.begin(), items.end(), [&](const T& it) { return it.getName() == n; });
        if (it == items.end()) return false;
        items.erase(it, items.end());
        return true;
    }

    /**
     * @brief Знайти предмет за назвою.
     * @param n Назва.
     * @return Вказівник на предмет або nullptr.
     */
    T* findByName(const string& n) {
        for (auto& it : items)
            if (it.getName() == n) return &it;
        return nullptr;
    }

    /**
     * @brief Отримати копію всіх предметів.
     * @return Вектор предметів.
     */
    vector<T> snapshot() const { return items; } 

    /**
     * @brief Строкове представлення вмісту інвентаря.
     */
    string toString() const {
        string s = "Inventory(" + to_string(items.size()) + "/" + to_string(capacity) + "): ";
        for (auto& it : items) s += it.str() + " ";
        return s;
    }
    
    size_t getSize() const { return items.size(); } // Додано для тестів
};

class Character;
class Skill;

/* --------------------- Skill hierarchy --------------------- */

/**
 * @class Skill
 * @brief Абстрактний базовий клас навички.
 */
class Skill {
protected:
    string name;    ///< Назва навички
    int level;      ///< Рівень навички
    int basePower;  ///< Базова сила
public:
    /**
     * @brief Конструктор навички.
     * @param n Назва.
     * @param p Базова сила.
     */
    Skill(const string& n, int p = 10) : name(n), level(1), basePower(p) {}
    
    virtual ~Skill() = default;

    /**
     * @brief Розрахунок ефективної сили.
     * @return Сила з урахуванням рівня.
     */
    virtual int effectivePower() const {
        return basePower + level * 3;
    }

    /**
     * @brief Опис навички.
     * @return Рядок з деталями.
     */
    virtual string description() const {
        return name + " (lvl " + to_string(level) + ", pwr " + to_string(effectivePower()) + ")";
    }

    /**
     * @brief Застосувати навичку до цілі.
     * @param target Персонаж-ціль.
     */
    virtual void apply(Character& target) = 0; 

    /**
     * @brief Покращити навичку.
     * Підвищує рівень та базову силу.
     */
    virtual void upgrade() {
        level++;
        basePower = basePower + 2;
    }
    string getName() const { return name; }
    int getLevel() const { return level; } // Додано для тестів
};

/**
 * @class ActiveSkill
 * @brief Активна навичка, що потребує мани.
 */
class ActiveSkill : public Skill {
    int manaCost; ///< Вартість використання в мані
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

/**
 * @class PassiveSkill
 * @brief Пасивна навичка, що дає модифікатори.
 */
class PassiveSkill : public Skill {
    double modifier; ///< Модифікатор (наприклад, 0.05 = 5%)
public:
    PassiveSkill(const string& n, int p = 5, double mod = 0.05) : Skill(n, p), modifier(mod) {}
    
    int effectivePower() const override {
        return basePower + static_cast<int>(level * (modifier * 100));
    }
    
    string description() const override {
        return "Passive: " + Skill::description() + " mod: " + to_string(modifier);
    }
    
    void apply(Character& target) override; 
};

/**
 * @class UltimateSkill
 * @brief Потужна навичка з кулдауном (перезарядкою).
 */
class UltimateSkill : public ActiveSkill {
    int cooldown; ///< Час перезарядки в ходах
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

/**
 * @class SkillTreeNode
 * @brief Вузол дерева навичок.
 */
class SkillTreeNode {
    Skill* skill;
    SkillTreeNode* parent;
    vector<unique_ptr<SkillTreeNode>> children;
public:
    SkillTreeNode(Skill* s = nullptr, SkillTreeNode* p = nullptr) : skill(s), parent(p) {}
    ~SkillTreeNode() { /* skill ownership is external */ }

    Skill* getSkill() const { return skill; }
    SkillTreeNode* getParent() const { return parent; }

    /**
     * @brief Додати дочірню навичку.
     * @param s Вказівник на навичку.
     * @return Вказівник на створений вузол.
     */
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

    /**
     * @brief Обхід дерева в глибину (DFS).
     * @tparam F Тип функтора.
     * @param f Функція, що викликається для кожного вузла.
     */
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
 * @class SkillTree
 * @brief Дерево навичок персонажа.
 * @tparam T Тип даних (не використовується явно в поточній реалізації, але зарезервовано).
 */
template<typename T>
class SkillTree {
    unique_ptr<SkillTreeNode> root;
public:
    SkillTree() : root(nullptr) {}
    SkillTree(Skill* s) { root = make_unique<SkillTreeNode>(s, nullptr); }

    SkillTreeNode* getRoot() const { return root.get(); }

    /**
     * @brief Вставити навичку під вказаним батьком.
     * @param parentSkillName Назва батьківської навички.
     * @param s Нова навичка.
     * @return Вказівник на створений вузол.
     */
    SkillTreeNode* insertUnder(const string& parentSkillName, Skill* s) {
        if (!root) {
            root = make_unique<SkillTreeNode>(s, nullptr);
            return root.get();
        }
        SkillTreeNode* found = findNodeBySkillName(parentSkillName);
        if (!found) return nullptr;
        return found->addChild(s);
    }

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
    
    /**
     * @brief Генерує випадкове дерево навичок.
     * @param skillFactory Функція-фабрика для створення навичок.
     * @param maxDepth Максимальна глибина дерева.
     * @param maxChildren Максимальна кількість дітей у вузла.
     */
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

/**
 * @class Character
 * @brief Базовий клас персонажа.
 */
class Character {
protected:
    string name;        ///< Ім'я
    int hp;             ///< Здоров'я
    int mana;           ///< Мана
    int attackPower;    ///< Сила атаки
    int defense;        ///< Захист
    int level;          ///< Рівень
    vector<unique_ptr<Skill>> ownedSkills; ///< Вивчені навички
    Inventory<Item> inventory; ///< Інвентар
    Logger& logger;     ///< Логер
public:
    /**
     * @brief Конструктор персонажа.
     * @param n Ім'я.
     * @param log Посилання на логер.
     */
    Character(const string& n, Logger& log)
        : name(n), hp(100), mana(50), attackPower(10), defense(5), level(1), inventory(10), logger(log) {
    }

    virtual ~Character() = default;

    /**
     * @brief Атака цілі.
     * @param target Ціль.
     * @return Завдана шкода.
     */
    virtual int attack(Character& target) {
        int raw = attackPower + level * 2;
        int variance = rand() % (level + 3);
        int dmg = max(0, raw + variance - target.getDefense());
        target.takeDamage(dmg);
        logger.log(name + " attacks " + target.getName() + " for " + to_string(dmg) + " dmg.");
        return dmg;
    }

    /**
     * @brief Використати навичку за індексом.
     * @param idx Індекс навички у списку.
     * @param target Ціль.
     */
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

    /**
     * @brief Екіпірувати (вивчити) нову навичку.
     * @param s Унікальний вказівник на навичку.
     */
    virtual void equipSkill(unique_ptr<Skill> s) {
        if (!s) return;
        logger.log(name + " equips skill " + s->getName());
        ownedSkills.push_back(move(s));
    }

    /**
     * @brief Підвищення рівня (Level Up).
     * Збільшує характеристики.
     */
    virtual void levelUp() {
        level++;
        hp += 10;
        mana += 5;
        attackPower += 2;
        defense += 1;
        logger.log(name + " leveled up to " + to_string(level));
    }

    /**
     * @brief Отримання шкоди.
     * @param d Кількість шкоди.
     */
    virtual void takeDamage(int d) {
        hp -= d;
        if (hp < 0) hp = 0;
    }

    virtual int getHP() const { return hp; }
    virtual int getMana() const { return mana; }
    virtual int getDefense() const { return defense; }
    string getName() const { return name; }
    int getLevel() const { return level; } 
    int getAttackPower() const { return attackPower; }

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

/**
 * @class Warrior
 * @brief Клас Воїна.
 * Використовує механіку люті (rage).
 */
class Warrior : public Character {
    int rage; ///< Накопичена лють
public:
    Warrior(const string& n, Logger& log) : Character(n, log), rage(0) {
        attackPower += 5;
        defense += 3;
    }
    
    /**
     * @brief Атака воїна.
     * Накопичує лють. Якщо люті > 50, завдає додаткової шкоди.
     */
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
    
    /**
     * @brief Бойовий клич.
     * Тимчасово збільшує силу атаки.
     */
    void battleShout() {
        attackPower += 2;
        logger.log(name + " shouts and increases attack!");
    }
};

/**
 * @class Mage
 * @brief Клас Мага.
 * Використовує ману для заклинань.
 */
class Mage : public Character {
    int spellPower; ///< Сила заклинань
public:
    Mage(const string& n, Logger& log) : Character(n, log), spellPower(10) {
        mana += 30;
    }
    
    /**
     * @brief Використання навички магом.
     * Перевіряє наявність мани перед використанням.
     */
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

/**
 * @class Archer
 * @brief Клас Лучника.
 * Має шанс на критичний удар та ухилення.
 */
class Archer : public Character {
    int agility; ///< Спритність
public:
    Archer(const string& n, Logger& log) : Character(n, log), agility(12) {
        attackPower += 2;
    }
    
    /**
     * @brief Атака лучника.
     * Має шанс нанести критичний удар.
     */
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
    
    /**
     * @brief Ухилення.
     * Збільшує захист.
     */
    void dodge() {
        defense += 2;
        logger.log(name + " prepares to dodge, defense increased temporarily.");
    }
};

// Реалізація методів apply для навичок
void ActiveSkill::apply(Character& target) {
    int p = effectivePower();
    int variance = rand() % 5;
    int dmg = max(1, p + variance - target.getDefense());
    target.takeDamage(dmg);
    // cout << "ActiveSkill " << name << " applied to " << target.getName() << " for " << dmg << " damage\n";
}

void PassiveSkill::apply(Character& target) {
    // В юніт-тестах пасивні навички зазвичай перевіряються через їхній
    // вплив на overallPower або через зміну статів
    // cout << "PassiveSkill " << name << " applied to " << target.getName() << " (passive buff)\n";
}

void UltimateSkill::apply(Character& target) {
    int p = effectivePower();
    int dmg = max(5, p - target.getDefense());
    target.takeDamage(dmg);
    // cout << "UltimateSkill " << name << " strikes " << target.getName() << " for " << dmg << " massive damage!\n";
}

/**
 * @class Party
 * @brief Група персонажів (паті).
 */
class Party {
    vector<unique_ptr<Character>> members;
    Logger& logger;
public:
    Party(Logger& log) : logger(log) {}
    
    /**
     * @brief Додати учасника до групи.
     */
    void addMember(unique_ptr<Character> c) {
        logger.log("Adding member " + c->getName());
        members.push_back(move(c));
    }
    
    Character* getMember(size_t idx) {
        if (idx >= members.size()) return nullptr;
        return members[idx].get();
    }
    size_t size() const { return members.size(); }

    /**
     * @brief Розрахувати загальну силу групи.
     */
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

/**
 * @class BattleSimulator
 * @brief Симулятор бою між двома групами.
 */
class BattleSimulator {
    Logger& logger;
public:
    BattleSimulator(Logger& log) : logger(log) {}
    
    /**
     * @brief Запустити симуляцію бою.
     * @param a Перша група.
     * @param b Друга група.
     */
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

/**
 * @brief Фабрика для створення випадкових навичок.
 * @return Вказівник на нову навичку.
 */
Skill* randomSkillFactory() {
    static int counter = 0;
    counter++;
    int t = rand() % 3;
    if (t == 0) return new ActiveSkill("Active_" + to_string(counter), 10 + (counter % 5));
    if (t == 1) return new PassiveSkill("Passive_" + to_string(counter), 5 + (counter % 3));
    return new UltimateSkill("Ult_" + to_string(counter), 25 + (counter % 8));
}

#endif //DANILKA_RPG_ENGINE_H