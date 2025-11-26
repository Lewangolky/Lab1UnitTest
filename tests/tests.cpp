#include "gtest/gtest.h"
#include "rpg_engine.h" // Включаємо код, який тестуємо (шлях правильний завдяки RPGEngine)

// Глобальний логгер для всіх тестів
Logger testLogger(Logger::Level::ERROR);

// --- 1. Тести для Item ---
TEST(ItemTest, ConstructorAndGetters) {
    Item potion("Potion", 50);
    EXPECT_EQ(potion.getName(), "Potion");
    EXPECT_EQ(potion.getValue(), 50);
    EXPECT_EQ(potion.str(), "Potion($50)");
}

// --- 2. Тести для Inventory (Template) ---
TEST(InventoryTest, AddItem) {
    Inventory<Item> inv(5);
    Item potion("Potion", 50);
    EXPECT_TRUE(inv.add(potion));
    EXPECT_EQ(inv.getSize(), 1);
}

TEST(InventoryTest, AddItemFailsWhenFull) {
    Inventory<Item> inv(1);
    inv.add(Item("Potion", 50));
    EXPECT_FALSE(inv.add(Item("Sword", 100)));
    EXPECT_EQ(inv.getSize(), 1);
}

TEST(InventoryTest, FindByName) {
    Inventory<Item> inv(5);
    inv.add(Item("Potion", 50));
    inv.add(Item("Sword", 100));

    Item* found = inv.findByName("Sword");
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->getValue(), 100);

    Item* notFound = inv.findByName("Shield");
    EXPECT_EQ(notFound, nullptr);
}

TEST(InventoryTest, RemoveItem) {
    Inventory<Item> inv(5);
    inv.add(Item("Potion", 50));
    EXPECT_TRUE(inv.removeIfName("Potion"));
    EXPECT_EQ(inv.getSize(), 0);
    EXPECT_FALSE(inv.removeIfName("Potion")); // Вже немає
}


// --- 3. Тести для Skill (поліморфізм) ---
TEST(SkillTest, BaseEffectivePower) {
    // Використовуємо 'new' і 'delete' або smart pointers для абстрактних класів у тестах
    Skill* skill = new ActiveSkill("Test Attack", 10); // Потрібна конкретна реалізація
    // basePower(10) + level(1) * 5 = 15 (для ActiveSkill)
    EXPECT_EQ(skill->effectivePower(), 15);
    delete skill;
}

TEST(SkillTest, SkillUpgrade) {
    ActiveSkill skill("Basic Attack", 10); // Використовуємо конкретний тип
    skill.upgrade(); // level -> 2, basePower -> 12
    EXPECT_EQ(skill.getLevel(), 2);
    // basePower(12) + level(2) * 5 = 22
    EXPECT_EQ(skill.effectivePower(), 22);
}

TEST(SkillTest, ActiveSkillEffectivePower) {
    ActiveSkill fireball("Fireball", 20, 10);
    // basePower(20) + level(1) * 5 = 25
    EXPECT_EQ(fireball.effectivePower(), 25);
    fireball.upgrade(); // level -> 2, basePower -> 22
    // basePower(22) + level(2) * 5 = 32
    EXPECT_EQ(fireball.effectivePower(), 32);
}

TEST(SkillTest, UltimateSkillEffectivePower) {
    UltimateSkill meteor("Meteor", 50, 50, 3);
    // basePower(50) + level(1) * 12 = 62
    EXPECT_EQ(meteor.effectivePower(), 62);
    meteor.upgrade(); // level -> 2, basePower -> 52
    // basePower(52) + level(2) * 12 = 76
    EXPECT_EQ(meteor.effectivePower(), 76);
}

// --- 4. Тести для Character (базовий клас) ---
TEST(CharacterTest, TakeDamage) {
    Character bob("Bob", testLogger);
    EXPECT_EQ(bob.getHP(), 100);
    bob.takeDamage(30);
    EXPECT_EQ(bob.getHP(), 70);
    bob.takeDamage(80); // Має впасти до 0, а не -10
    EXPECT_EQ(bob.getHP(), 0);
}

TEST(CharacterTest, LevelUp) {
    Character bob("Bob", testLogger);
    bob.levelUp();
    EXPECT_EQ(bob.getLevel(), 2);
    EXPECT_EQ(bob.getHP(), 110);
    EXPECT_EQ(bob.getMana(), 55);
    EXPECT_EQ(bob.getAttackPower(), 12);
}

TEST(CharacterTest, EquipSkillAndOverallPower) {
    Character bob("Bob", testLogger);
    // base_power = attack(10) + level(1) * 3 = 13
    EXPECT_EQ(bob.overallPower(), 13);

    // Додаємо навичку з effectivePower() = 25
    auto fireball = make_unique<ActiveSkill>("Fireball", 20, 10); // pwr = 20 + 1*5 = 25
    bob.equipSkill(move(fireball));

    // overall = 13 + (25 / 2) = 13 + 12 = 25
    EXPECT_EQ(bob.overallPower(), 25);
    EXPECT_EQ(bob.skillCount(), 1);
}

// --- 5. Тести для взаємодії класів (Mage, Warrior) ---

// Тестова "оснастка" (fixture) для тестів, де потрібні персонажі
class CharacterInteractionTest : public ::testing::Test {
protected:
    unique_ptr<Mage> mage;
    unique_ptr<Warrior> warrior;

    void SetUp() override {
        mage = make_unique<Mage>("Gandalf", testLogger);
        warrior = make_unique<Warrior>("Conan", testLogger);
    }
};

TEST_F(CharacterInteractionTest, UseSkillAppliesDamage) {
    mage->equipSkill(make_unique<ActiveSkill>("Fireball", 20, 10));

    int initialWarriorHP = warrior->getHP(); // 100
    int warriorDefense = warrior->getDefense(); // 5 (база) + 3 (бонус) = 8
    // Skill pwr = 20 + 1*5 = 25
    // Dmg = max(1, pwr(25) + rand(0-4) - def(8)) = max(1, 17 + rand(0-4))
    // Dmg range: [17, 21]

    // Щоб тест був стабільним, ми не можемо залежати від rand()
    // Але ми можемо перевірити, що шкода *була* завдана
    mage->useSkill(0, *warrior); // 0 -- індекс навички

    EXPECT_LT(warrior->getHP(), initialWarriorHP); // HP має зменшитись
    EXPECT_GE(warrior->getHP(), initialWarriorHP - (25 + 4 - warriorDefense)); // Макс шкода
    EXPECT_LE(warrior->getHP(), initialWarriorHP - (25 + 0 - warriorDefense)); // Мін шкода
}

TEST_F(CharacterInteractionTest, MageManaCheckFail) {
    mage->equipSkill(make_unique<ActiveSkill>("BigSpell", 20, 1000));

    int initialMana = mage->getMana(); // 80
    int initialWarriorHP = warrior->getHP();

    mage->useSkill(0, *warrior);

    EXPECT_EQ(mage->getMana(), initialMana); // Мана не змінилась
    EXPECT_EQ(warrior->getHP(), initialWarriorHP); // Шкоди немає
}

TEST_F(CharacterInteractionTest, MageManaCheckSuccess) {
    mage->equipSkill(make_unique<ActiveSkill>("Fireball", 20, 12));

    int initialMana = mage->getMana(); // 80
    mage->useSkill(0, *warrior);

    EXPECT_EQ(mage->getMana(), initialMana - 12); // Мана витратилась
}


// --- 6. Тести для SkillTree ---
TEST(SkillTreeTest, InsertAndFind) {
    ActiveSkill s1("RootSkill", 10);
    PassiveSkill s2("ChildSkill", 5);

    SkillTree<Skill*> tree(&s1);
    tree.insertUnder("RootSkill", &s2);

    auto* node = tree.findNodeBySkillName("ChildSkill");
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->getSkill()->getName(), "ChildSkill");
    ASSERT_NE(node->getParent(), nullptr);
    EXPECT_EQ(node->getParent()->getSkill()->getName(), "RootSkill");
}

// --- 7. Тести для Party ---
TEST(PartyTest, AddMemberAndCombinedPower) {
    Party party(testLogger);

    auto mage = make_unique<Mage>("Gandalf", testLogger); // pwr = 10(atk) + 3(lvl*1) = 13
    auto warrior = make_unique<Warrior>("Conan", testLogger); // pwr = 15(atk) + 3(lvl*1) = 18

    party.addMember(move(mage));
    party.addMember(move(warrior));

    EXPECT_EQ(party.size(), 2);
    // combined = 13 (Mage) + 18 (Warrior) = 31
    EXPECT_EQ(party.combinedPower(), 31);
}